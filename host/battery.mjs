import fs from 'node:fs';
const unknown=()=>({mic_bars:[-1,-1],mic_max:7,remote_percent:-1});
export function batteryView(raw,now=Date.now()/1000){
 const result=unknown();
 if(raw?.state!=='ready'||!Number.isFinite(raw.checkedAt)||now-raw.checkedAt>90||raw.checkedAt-now>5)return result;
 for(const receiver of raw.microphones??[]){if(receiver.connected!==true)continue;
  for(const tx of receiver.transmitters??[]){if(tx.connected===true&&[1,2].includes(tx.slot)&&Number.isInteger(tx.batteryBars)&&tx.batteryBars>=0&&tx.batteryBars<=7&&tx.batteryBarsMax===7)result.mic_bars[tx.slot-1]=tx.batteryBars;}}
 const remote=raw.remote;
 if(remote?.connected===true&&Number.isFinite(remote.checkedAt)&&now-remote.checkedAt<=90&&remote.checkedAt-now<=5&&Number.isInteger(remote.batteryPercent)&&remote.batteryPercent>=0&&remote.batteryPercent<=100)result.remote_percent=remote.batteryPercent;
 return result;
}
export function readBattery(file){try{return batteryView(JSON.parse(fs.readFileSync(file,'utf8')));}catch{return unknown();}}
