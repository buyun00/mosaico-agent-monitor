// A community adapter sends only normalized display data, never credentials or raw transcripts.
import http from 'node:http';
import {loadConfig} from '../config.mjs';
export function publish(platform, input) {
  const config=loadConfig();
  return new Promise((resolve,reject)=>{
    const request=http.request({host:'127.0.0.1',port:config.controlPort,path:'/event',method:'POST',timeout:1000,
      headers:{'Content-Type':'application/json','Authorization':'Bearer '+config.controlToken}},response=>{
        response.resume();response.on('end',()=>response.statusCode===204?resolve():reject(new Error('Adapter rejected')));
      });
    request.on('error',reject);request.on('timeout',()=>request.destroy(new Error('Monitor unavailable')));
    request.end(JSON.stringify({platform,kind:'external',input}));
  });
}
// Add your platform name to ~/.mosaico-agent-monitor/config.json platforms first, then:
// await publish('myagent', {task:{id:'session-1',name:'Build project',state:'running',phase:'coding'},
//   quota:{five_hour:{remaining_percent:72,resets_at:REAL_RESET_EPOCH},seven_day:null}});
// Omit unsupported windows or set them to null. Do not invent values for missing limits.
