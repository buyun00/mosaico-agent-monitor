import {spawn} from 'node:child_process';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import readline from 'node:readline';
import {codexQuota} from '../model.mjs';
export function findCodex() {
  const names=process.platform==='win32'?['codex.exe']:['codex'];
  const directories=[...(process.env.PATH||'').split(path.delimiter),
    path.join(process.env.LOCALAPPDATA||os.homedir(),'OpenAI','Codex','bin')];
  for(const dir of directories) for(const name of names) if(dir && fs.existsSync(path.join(dir,name))) return path.join(dir,name);
  const prefix=process.env.APPDATA?path.join(process.env.APPDATA,'npm'):path.join(os.homedir(),'.npm-global','lib');
  const pkg=path.join(prefix,'node_modules','@openai','codex');
  const find=(dir,depth=0)=>{
    if(depth>7 || !fs.existsSync(dir)) return null;
    for(const item of fs.readdirSync(dir,{withFileTypes:true})) {
      const p=path.join(dir,item.name);
      if(item.isFile() && item.name==='codex.exe') return p;
      if(item.isDirectory()) {const result=find(p,depth+1);if(result)return result;}
    }
    return null;
  };
  return find(pkg);
}
export function readCodexQuota(executable=findCodex(), timeoutMs=12000) {
  return new Promise((resolve,reject)=>{
    if(!executable) return reject(new Error('Codex executable unavailable'));
    const child=spawn(executable,['app-server','--listen','stdio://'],{windowsHide:true,stdio:['pipe','pipe','pipe']});
    let finished=false;
    const done=(error,result)=>{if(finished)return;finished=true;clearTimeout(timer);lines.close();child.kill();child.stdin.destroy();child.stdout.destroy();child.stderr.destroy();error?reject(error):resolve(codexQuota(result));};
    const lines=readline.createInterface({input:child.stdout});
    const timer=setTimeout(()=>done(new Error('Codex quota query timed out')),timeoutMs);
    child.stderr.on('data',()=>{});child.stdin.on('error',()=>{});
    child.on('error',()=>done(new Error('Codex app-server could not start')));
    child.on('exit',()=>{if(!finished)done(new Error('Codex app-server exited'));});
    lines.on('line',line=>{
      let message;try{message=JSON.parse(line);}catch{return;}
      if(message.id===1) {
        if(message.error)return done(new Error('Codex initialization failed'));
        child.stdin.write(JSON.stringify({method:'initialized',params:{}})+'\n');
        child.stdin.write(JSON.stringify({id:2,method:'account/rateLimits/read',params:{}})+'\n');
      }
      if(message.id===2) message.error?done(new Error('Codex quota unavailable')):done(null,message.result);
    });
    child.stdin.write(JSON.stringify({id:1,method:'initialize',params:{clientInfo:{name:'mosaico-agent-monitor',title:'Mosaico Agent Monitor',version:'0.15.0'}}})+'\n');
  });
}
