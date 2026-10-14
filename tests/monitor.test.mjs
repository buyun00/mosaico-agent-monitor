import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import {spawn,spawnSync} from 'node:child_process';
import {Monitor,codexQuota,claudeQuota,hookTask} from '../host/model.mjs';
import {startMonitor} from '../host/server.mjs';
const now=1800000000;
const short={usedPercent:25,windowDurationMins:300,resetsAt:now+7200};
const week={usedPercent:61,windowDurationMins:10080,resetsAt:now+500000};
test('Codex Plus preserves both windows and selects codex rather than the reserve bucket',()=>{
 const q=codexQuota({rateLimitsByLimitId:{reserve:{primary:week},codex:{primary:short,secondary:week}}},now);
 assert.equal(q.five_hour.remaining_percent,75);assert.equal(q.seven_day.remaining_percent,39);
});
test('Codex Pro with no five-hour window never fabricates one',()=>{
 const monitor=new Monitor(['codex']);monitor.quota('codex',codexQuota({rateLimits:{limitId:'codex',primary:null,secondary:week}},now));
 const s=monitor.snapshot(now);assert.equal(s.providers[0].five_hour_available,false);assert.equal(s.providers[0].five_hour_percent,-1);assert.equal(s.weekly_percent,39);
});
test('Only exact 300-minute and 10080-minute windows are displayed',()=>{
 const q=codexQuota({rateLimits:{primary:{...short,windowDurationMins:120},secondary:week}},now);
 assert.equal(q.five_hour,null);assert.equal(q.seven_day.remaining_percent,39);
 assert.equal(codexQuota({rateLimitsByLimitId:{reserve:{primary:week}}},now).seven_day,null);
});
test('Claude Code reads real subscription fields and accepts independent missing windows',()=>{
 const q=claudeQuota({rate_limits:{five_hour:{used_percentage:12.5,resets_at:now+3000}}},now);
 assert.equal(q.five_hour.remaining_percent,88);assert.equal(q.seven_day,null);
 assert.equal(claudeQuota({},now).five_hour,null);
});
test('Missing, invalid, expired and zero remaining quota remain distinct',()=>{
 for(const used of [null,'25',-1,101,NaN])assert.equal(codexQuota({rateLimits:{primary:{...short,usedPercent:used}}},now).five_hour.remaining_percent,null);
 assert.equal(codexQuota({rateLimits:{primary:{...short,usedPercent:100}}},now).five_hour.remaining_percent,0);
 assert.equal(codexQuota({rateLimits:{primary:{...short,resetsAt:now-1}}},now).five_hour.remaining_percent,null);
});
test('Quota expiry does not interrupt task state or report false zero',()=>{
 const monitor=new Monitor(['codex']);monitor.quota('codex',codexQuota({rateLimits:{primary:short,secondary:week}},now));
 monitor.event('codex',{session_id:'one',hook_event_name:'UserPromptSubmit',prompt:'Write a program'},now);
 const s=monitor.snapshot(now+91);assert.equal(s.weekly_percent,-1);assert.equal(s.tasks_available,true);assert.equal(s.tasks[0].state,'running');
});
test('Switching CC and Codex keeps separate account windows and session identities',()=>{
 const m=new Monitor();m.quota('codex',codexQuota({rateLimits:{primary:null,secondary:week}},now));
 m.quota('claude',claudeQuota({rate_limits:{five_hour:{used_percentage:90,resets_at:now+1200}}},now));
 m.event('codex',{session_id:'same',hook_event_name:'UserPromptSubmit',prompt:'Codex task'},now);
 m.event('claude',{session_id:'same',hook_event_name:'UserPromptSubmit',prompt:'Claude task'},now+1);
 const s=m.snapshot(now+1);assert.equal(s.platform,'claude');assert.equal(s.tasks.length,2);assert.equal(s.providers[1].five_hour_percent,10);
 assert.equal(s.providers[0].five_hour_available,false);
});
test('Permission waits, coding, completed turns and interruptions map from actual events',()=>{
 const e={session_id:'x'};assert.equal(hookTask('codex',{...e,hook_event_name:'PermissionRequest'}, {},now).state,'attention');
 assert.equal(hookTask('claude',{...e,hook_event_name:'PreToolUse',tool_name:'Edit'}, {},now).phase,'coding');
 assert.equal(hookTask('codex',{...e,hook_event_name:'Stop'}, {},now).state,'done');
 assert.equal(hookTask('codex',{...e,hook_event_name:'Interrupt'}, {},now).state,'paused');
 assert.equal(hookTask('codex',{...e,hook_event_name:'SubagentStop'}, {},now),undefined);
});
test('HTTP returns usable task data without quota, rejects LAN writes and filters private fields',async()=>{
 const service=await startMonitor({port:0,controlPort:0,bind:'127.0.0.1',controlToken:'test-only',noPoll:true,platforms:['claude']});
 try{
  const base=`http://127.0.0.1:${service.readServer.address().port}`,control=`http://127.0.0.1:${service.control.address().port}/event`;
  let r=await fetch(control,{method:'POST',headers:{Authorization:'Bearer wrong'},body:'{}'});assert.equal(r.status,401);
  const input={session_id:'x',hook_event_name:'UserPromptSubmit',prompt:'Make an example',access_token:'PRIVATE-TEST-SENTINEL',cwd:'private-path'};
  r=await fetch(control,{method:'POST',headers:{Authorization:'Bearer test-only'},body:JSON.stringify({platform:'claude',kind:'event',input})});assert.equal(r.status,204);
  r=await fetch(base+'/quota');assert.equal(r.status,200);const raw=await r.text();const state=JSON.parse(raw);
  assert.equal(state.weekly_percent,-1);assert.equal(state.tasks[0].state,'running');assert(!raw.includes('PRIVATE-TEST-SENTINEL'));assert(!raw.includes('private-path'));
  assert.equal((await fetch(base+'/quota',{method:'POST',body:'{}'})).status,405);
 }finally{service.close();}
});
test('Installer is idempotent and preserves existing hooks and status line on paths containing spaces',()=>{
 const root=fs.mkdtempSync(path.join(os.tmpdir(),'mosaico installer '));const home=path.join(root,'user home');const config=path.join(root,'monitor config');
 fs.mkdirSync(path.join(home,'.claude'),{recursive:true});fs.mkdirSync(path.join(home,'.codex'),{recursive:true});
 const original={hooks:{Stop:[{hooks:[{type:'command',command:'existing-command'}]}]},statusLine:{type:'command',command:'old-status-line'},other:{keep:true}};
 fs.writeFileSync(path.join(home,'.claude/settings.json'),JSON.stringify(original));
 fs.writeFileSync(path.join(home,'.codex/hooks.json'),JSON.stringify({hooks:{Stop:[{hooks:[{type:'command',command:'existing-codex'}]}]}}));
 const run=()=>spawnSync(process.execPath,['host/setup.mjs','--user-home',home],{cwd:new URL('..',import.meta.url),env:{...process.env,MOSAICO_MONITOR_HOME:config},encoding:'utf8'});
 assert.equal(run().status,0);assert.equal(run().status,0);
 const settings=JSON.parse(fs.readFileSync(path.join(home,'.claude/settings.json')));assert.deepEqual(settings.other,original.other);assert.equal(settings.hooks.Stop.length,2);
 assert.equal(settings.hooks.Stop[0].hooks[0].command,'existing-command');
 assert.equal(JSON.parse(fs.readFileSync(path.join(config,'config.json'))).previousClaudeStatusLine.command,'old-status-line');
 assert(path.resolve(root).startsWith(path.resolve(os.tmpdir())+path.sep));
 fs.rmSync(root,{recursive:true,force:true});
});
test('Installed CC shell commands deliver events and both quota windows, and fail open without a bridge',async()=>{
 const root=fs.mkdtempSync(path.join(os.tmpdir(),'mosaico hook qa ')),home=path.join(root,'user home'),dir=path.join(root,'config');
 const env={...process.env,MOSAICO_MONITOR_HOME:dir};
 const setup=spawnSync(process.execPath,['host/setup.mjs','--user-home',home,'--platforms','claude'],{cwd:new URL('..',import.meta.url),env,encoding:'utf8'});assert.equal(setup.status,0);
 const service=await startMonitor({port:0,controlPort:0,bind:'127.0.0.1',controlToken:'qa-only',noPoll:true,platforms:['claude']});
 const target=path.join(dir,'config.json'),config=JSON.parse(fs.readFileSync(target));config.controlPort=service.control.address().port;config.controlToken='qa-only';fs.writeFileSync(target,JSON.stringify(config));
 const settings=JSON.parse(fs.readFileSync(path.join(home,'.claude/settings.json')));
 const run=(command,input)=>new Promise((resolve,reject)=>{
  const child=spawn(command,{shell:true,windowsHide:true,env,stdio:['pipe','pipe','pipe']});let output='',error='';
  child.stdout.on('data',chunk=>output+=chunk);child.stderr.on('data',chunk=>error+=chunk);child.stdin.end(JSON.stringify(input));
  child.on('error',reject);child.on('close',code=>resolve({code,output,error}));
 });
 try{
  const epoch=Math.floor(Date.now()/1000),input={session_id:'shell-test',rate_limits:{five_hour:{used_percentage:20,resets_at:epoch+7200},seven_day:{used_percentage:40,resets_at:epoch+500000}}};
  let result=await run(settings.statusLine.command,input);assert.equal(result.code,0,result.error);assert(result.output.includes('5h 80%'));
  result=await run(settings.hooks.PermissionRequest[0].hooks[0].command,{session_id:'shell-test',hook_event_name:'PermissionRequest'});assert.equal(result.code,0,result.error);assert.equal(result.output,'');
  const state=await(await fetch(`http://127.0.0.1:${service.readServer.address().port}/quota`)).json();
  assert.equal(state.providers[0].five_hour_percent,80);assert.equal(state.weekly_percent,60);assert.equal(state.tasks[0].state,'attention');
  service.close();const started=Date.now();result=await run(settings.hooks.Stop[0].hooks[0].command,{session_id:'shell-test',hook_event_name:'Stop'});assert.equal(result.code,0);assert.equal(result.output,'');assert(Date.now()-started<2000);
 }finally{service.close();assert(path.resolve(root).startsWith(path.resolve(os.tmpdir())+path.sep));fs.rmSync(root,{recursive:true,force:true});}
});
