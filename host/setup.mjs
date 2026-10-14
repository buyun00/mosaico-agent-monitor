import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import {fileURLToPath} from 'node:url';
import {saveConfig,lanAddresses} from './config.mjs';
import {findCodex} from './adapters/codex.mjs';
const args=process.argv.slice(2),option=(key,fallback)=>args.includes(key)?args[args.indexOf(key)+1]:fallback;
const home=option('--user-home',os.homedir());
const codexHome=option('--codex-home',args.includes('--user-home')?path.join(home,'.codex'):(process.env.CODEX_HOME||path.join(home,'.codex')));
const claudeHome=option('--claude-home',args.includes('--user-home')?path.join(home,'.claude'):(process.env.CLAUDE_CONFIG_DIR||path.join(home,'.claude')));
const platforms=option('--platforms','codex,claude').split(',');
if(!platforms.length || platforms.some(p=>!['codex','claude'].includes(p)))throw new Error('Use --platforms codex,claude, codex, or claude');
const port=Number(option('--port',8769)),controlPort=Number(option('--control-port',8770));
if(![port,controlPort].every(p=>Number.isInteger(p)&&p>=1024&&p<=65535)||port===controlPort)throw new Error('Invalid ports');
const hookPath=fileURLToPath(new URL('./hook.mjs',import.meta.url));
const quote=p=>process.platform==='win32'?`"${p.replaceAll('\\','/')}"`:"'"+p.replaceAll("'","'\\''")+"'";
const write=(target,value)=>{
  fs.mkdirSync(path.dirname(target),{recursive:true});
  if(fs.existsSync(target))fs.copyFileSync(target,target+'.mosaico-backup-'+Date.now());
  const temporary=target+'.mosaico.tmp';fs.writeFileSync(temporary,JSON.stringify(value,null,2)+'\n',{mode:0o600});fs.renameSync(temporary,target);
};
const read=target=>fs.existsSync(target)?JSON.parse(fs.readFileSync(target,'utf8').replace(/^\uFEFF/,'')):{};
const commands={};
const config=saveConfig({platforms,port,controlPort,codexPath:findCodex(),displayPlatform:option('--display','auto'),
  ...(args.includes('--battery')?{batteryPath:path.resolve(option('--battery',''))}:{})});
for(const platform of platforms) {
  const target=path.join(platform==='codex'?codexHome:claudeHome,platform==='codex'?'hooks.json':'settings.json');
  const settings=read(target);settings.hooks ||= {};
  const command=`${quote(process.execPath)} ${quote(hookPath)} --platform ${platform}`;commands[platform]=command;
  const previousCommand=config.installedHookCommands?.[platform];
  const ownsCommand=value=>[command,previousCommand].filter(Boolean).some(c=>value===c||value===c+' --kind statusline')||String(value||'').includes(hookPath);
  const events=['SessionStart','UserPromptSubmit','PreToolUse','PostToolUse','PermissionRequest','Stop','SessionEnd',
    ...(platform==='codex'?['Interrupt']:['PostToolUseFailure','Notification'])];
  for(const event of events) {
    // Remove only this monitor's handlers, preserving all unrelated groups and fields.
    const groups=(settings.hooks[event]||[]).map(group=>({...group,hooks:(group.hooks||[]).filter(h=>!ownsCommand(h.command))})).filter(group=>group.hooks.length);
    groups.push({hooks:[{type:'command',command,timeout:2}]});settings.hooks[event]=groups;
  }
  if(platform==='claude') {
    const statusCommand=command+' --kind statusline';
    if(settings.statusLine?.command && !ownsCommand(settings.statusLine.command))config.previousClaudeStatusLine=settings.statusLine;
    settings.statusLine={type:'command',command:statusCommand};
  }
  write(target,settings);
  console.log(`Installed ${platform} hooks in ${target}`);
}
config.installedHookCommands={...config.installedHookCommands,...commands};
saveConfig(config);
console.log('Codex: open /hooks and trust the Mosaico handlers, then start a new session.');
console.log('Claude Code: restart the session to load hooks/status line.');
for(const ip of lanAddresses())console.log(`Device URL: http://${ip}:${port}/quota`);
console.log('Start the computer service with Start.cmd or node host/server.mjs.');
