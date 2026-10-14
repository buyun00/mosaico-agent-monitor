import http from 'node:http';
import {spawn} from 'node:child_process';
import {loadConfig} from './config.mjs';
const args=process.argv.slice(2), option=(name,fallback)=>args.includes(name)?args[args.indexOf(name)+1]:fallback;
const platform=option('--platform','codex'), kind=option('--kind','event');
const config=loadConfig();let body='';
for await(const chunk of process.stdin) {body+=chunk;if(Buffer.byteLength(body)>65536)process.exit(0);}
let input;try{input=JSON.parse(body);}catch{process.exit(0);}
// Hooks never print control decisions or raw prompts. A stopped bridge must not block an Agent.
await new Promise(resolve=>{
  const request=http.request({host:'127.0.0.1',port:config.controlPort||8770,path:'/event',method:'POST',timeout:350,
    headers:{'Content-Type':'application/json','Authorization':'Bearer '+(config.controlToken||'')}},response=>{response.resume();response.on('end',resolve);});
  request.on('error',resolve);request.on('timeout',()=>{request.destroy();resolve();});
  request.end(JSON.stringify({platform,kind,input}));
});
if(kind==='statusline') {
  if(config.previousClaudeStatusLine?.command) {
    await new Promise(resolve=>{
      const child=spawn(config.previousClaudeStatusLine.command,{shell:true,windowsHide:true,stdio:['pipe','pipe','ignore']});
      const timer=setTimeout(()=>{child.kill();resolve();},1200);
      child.stdin.on('error',()=>{});child.stdin.end(body);child.stdout.pipe(process.stdout);
      child.on('error',()=>{clearTimeout(timer);resolve();});child.on('exit',()=>{clearTimeout(timer);resolve();});
    });
  } else {
    const limits=input.rate_limits||{}, parts=['Mosaico'];
    for(const [key,label] of [['five_hour','5h'],['seven_day','7d']]) {
      const value=limits[key]?.used_percentage;
      if(typeof value==='number' && value>=0 && value<=100) parts.push(`${label} ${Math.round(100-value)}% remaining`);
    }
    process.stdout.write(parts.join(' · ')+'\n');
  }
}
