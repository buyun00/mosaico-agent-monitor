import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import {randomBytes} from 'node:crypto';
export const configDir = () => process.env.MOSAICO_MONITOR_HOME || path.join(os.homedir(), '.mosaico-agent-monitor');
export function loadConfig() {
  try { return JSON.parse(fs.readFileSync(path.join(configDir(), 'config.json'), 'utf8')); }
  catch { return {platforms:['codex','claude'], port:8769, controlPort:8770, displayPlatform:'auto'}; }
}
export function saveConfig(value) {
  fs.mkdirSync(configDir(), {recursive:true, mode:0o700});
  const config = {...loadConfig(), ...value};
  config.controlToken ||= randomBytes(24).toString('hex');
  const target = path.join(configDir(), 'config.json');
  const tmp = target + '.' + process.pid + '.tmp';
  fs.writeFileSync(tmp, JSON.stringify(config, null, 2), {mode:0o600});
  fs.renameSync(tmp, target);
  return config;
}
export function lanAddresses() {
  return Object.values(os.networkInterfaces()).flat().filter(x => x && x.family==='IPv4' && !x.internal &&
    /^(10\.|192\.168\.|172\.(1[6-9]|2\d|3[01])\.)/.test(x.address)).map(x=>x.address);
}
