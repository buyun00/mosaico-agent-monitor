import http from 'node:http';
import fs from 'node:fs';
import path from 'node:path';
import {fileURLToPath} from 'node:url';
import {Monitor,claudeQuota} from './model.mjs';
import {loadConfig,saveConfig,lanAddresses,configDir} from './config.mjs';
import {readBattery} from './battery.mjs';
import {readCodexQuota} from './adapters/codex.mjs';
export async function startMonitor(overrides={}) {
  const config={...loadConfig(),...overrides};
  if(!Array.isArray(config.platforms)||!config.platforms.length||config.platforms.length>4||config.platforms.some(p=>!/^[a-z0-9_-]{1,20}$/.test(p))||new Set(config.platforms).size!==config.platforms.length)throw new Error('Configure 1-4 distinct platform names');
  if(!config.controlToken)Object.assign(config,saveConfig(config));
  const monitor=new Monitor(config.platforms,config.displayPlatform);let polling=false;
  const poll=async()=>{if(polling||!config.platforms.includes('codex'))return;polling=true;
    try{monitor.quota('codex',await readCodexQuota(config.codexPath));}catch{}finally{polling=false;}};
  const readServer=http.createServer((req,res)=>{
    res.setHeader('Cache-Control','no-store');
    if(req.method!=='GET'){res.writeHead(405);return res.end();}
    if(req.url==='/') {res.setHeader('Content-Type','text/html; charset=utf-8');return res.end(fs.readFileSync(fileURLToPath(new URL('./dashboard.html',import.meta.url))));}
    res.setHeader('Content-Type','application/json; charset=utf-8');
    if(req.url==='/health')return res.end(JSON.stringify({service:'mosaico-agent-monitor',version:'0.15.0'}));
    if(!['/quota','/state'].includes(req.url)){res.writeHead(404);return res.end('{}');}
    const state=monitor.snapshot();state.battery=readBattery(config.batteryPath||path.join(configDir(),'device-battery.json'));
    res.end(JSON.stringify(state));
  });
  const control=http.createServer(async(req,res)=>{
    if(req.method!=='POST' || req.url!=='/event'){res.writeHead(404);return res.end();}
    if(req.headers.authorization!=='Bearer '+config.controlToken){res.writeHead(401);return res.end();}
    let body='';
    try{
      for await(const chunk of req){body+=chunk;if(Buffer.byteLength(body)>65536){res.writeHead(413);return res.end();}}
      const value=JSON.parse(body),platform=value.platform;
      if(!config.platforms.includes(platform)){res.writeHead(400);return res.end();}
      if(value.kind==='statusline' && platform==='claude') monitor.quota(platform,claudeQuota(value.input));
      else if(value.kind==='external') monitor.external(platform,value.input);
      else monitor.event(platform,value.input);
      res.writeHead(204);res.end();
    }catch{res.writeHead(400);res.end();}
  });
  const listen=(server,port,host)=>new Promise((resolve,reject)=>{server.once('error',reject);server.listen(port,host,resolve);});
  try{await listen(control,config.controlPort,'127.0.0.1');await listen(readServer,config.port,config.bind||'0.0.0.0');}
  catch(error){control.close();readServer.close();throw error;}
  if(!config.noPoll)poll();const timer=config.noPoll?null:setInterval(poll,30000);
  const close=()=>{clearInterval(timer);readServer.close();control.close();};
  return {monitor,config,close,readServer,control};
}
if(process.argv[1]===fileURLToPath(import.meta.url)) {
  const args=process.argv.slice(2), option=(key,fallback)=>args.includes(key)?args[args.indexOf(key)+1]:fallback;
  const overrides={};if(args.includes('--port'))overrides.port=Number(option('--port',8769));
  if(args.includes('--control-port'))overrides.controlPort=Number(option('--control-port',8770));
  if(args.includes('--no-poll'))overrides.noPoll=true;
  if(args.includes('--platforms'))overrides.platforms=option('--platforms','codex,claude').split(',');
  if(args.includes('--battery'))overrides.batteryPath=option('--battery','');
  try{
    const service=await startMonitor(overrides);
    console.log('Mosaico Agent Monitor 0.15.0');
    for(const ip of lanAddresses())console.log(`Device URL: http://${ip}:${service.config.port}/quota`);
    console.log(`Computer dashboard: http://127.0.0.1:${service.config.port}/`);
    for(const signal of ['SIGINT','SIGTERM'])process.on(signal,()=>{service.close();process.exit(0);});
  }catch(error){console.error('Monitor could not start: '+error.message);process.exitCode=1;}
}
