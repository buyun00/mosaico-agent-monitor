const STATES = new Set(['running','attention','done','waiting','paused','failed','offline']);
export const seconds = () => Math.floor(Date.now()/1000);
export const clean = (value, max=56) => Array.from(String(value??'').replace(/[\x00-\x1f\x7f]/g,' ')).slice(0,max).join('');
const validPercent = v => typeof v==='number' && Number.isFinite(v) && v>=0 && v<=100;
function window(value, usedKey, resetKey, now) {
  if (!value || typeof value!=='object') return null;
  const used=value[usedKey], reset=value[resetKey];
  const resetsAt=Number.isSafeInteger(reset) && reset>0 ? reset : null;
  return {remaining_percent:validPercent(used) && (!resetsAt || resetsAt>now) ? Math.round(100-used) : null,
    resets_at:resetsAt, updated_at:now};
}
export function codexQuota(result, now=seconds()) {
  const bucket=result?.rateLimitsByLimitId?.codex ??
    ((!result?.rateLimits?.limitId || result.rateLimits.limitId==='codex') ? result?.rateLimits : null);
  const windows=[bucket?.primary,bucket?.secondary];
  return {five_hour:window(windows.find(x=>x?.windowDurationMins===300),'usedPercent','resetsAt',now),
    seven_day:window(windows.find(x=>x?.windowDurationMins===10080),'usedPercent','resetsAt',now), updated_at:now};
}
export function claudeQuota(input, now=seconds()) {
  return {five_hour:window(input?.rate_limits?.five_hour,'used_percentage','resets_at',now),
    seven_day:window(input?.rate_limits?.seven_day,'used_percentage','resets_at',now), updated_at:now};
}
export function hookTask(platform, event, previous={}, now=seconds()) {
  const name=event.hook_event_name;
  if (name==='SessionEnd') return null;
  const state=({SessionStart:'waiting',UserPromptSubmit:'running',PreToolUse:'running',PostToolUse:'running',
    PermissionRequest:'attention',Stop:'done',Interrupt:'paused',PostToolUseFailure:'failed',Notification:'attention'})[name];
  if (!state) return undefined;
  if (name==='Notification' && !['permission_prompt','idle_prompt'].includes(event.notification_type)) return undefined;
  const tool=String(event.tool_name??'');
  let phase='thinking';
  if (name==='PreToolUse') phase=/write|edit|patch/i.test(tool)?'coding':/test|build|check/i.test(tool)?'checking':'working';
  return {id:clean(event.session_id,100), platform,
    name:clean(event.session_title || event.session_name || (name==='UserPromptSubmit'?event.prompt:previous.name) || `${platform==='claude'?'Claude Code':'Codex'} task`),
    state, phase, updated_at:now};
}
export class Monitor {
  constructor(platforms=['codex','claude'], displayPlatform='auto') {
    this.platforms=platforms; this.displayPlatform=displayPlatform; this.quotas=new Map(); this.tasks=new Map();
  }
  quota(platform, value) {if(this.platforms.includes(platform)) this.quotas.set(platform,value);}
  event(platform, event, now=seconds()) {
    if(!this.platforms.includes(platform) || !event?.session_id) return;
    const key=platform+':'+clean(event.session_id,100);
    const task=hookTask(platform,event,this.tasks.get(key),now);
    if(task===null) this.tasks.delete(key); else if(task) this.tasks.set(key,task);
  }
  external(platform, value, now=seconds()) {
    if(!this.platforms.includes(platform)) return;
    if(value.quota) {
      const mapped={updated_at:now};
      for(const key of ['five_hour','seven_day']) {
        const v=value.quota[key];
        mapped[key]=v?{remaining_percent:validPercent(v.remaining_percent)?Math.round(v.remaining_percent):null,
          resets_at:Number.isSafeInteger(v.resets_at)&&v.resets_at>0?v.resets_at:null,updated_at:now}:null;
      }
      this.quota(platform,mapped);
    }
    if(value.task && STATES.has(value.task.state) && value.task.id) {
      this.tasks.set(platform+':'+clean(value.task.id,100), {id:clean(value.task.id,100),platform,
        name:clean(value.task.name),state:value.task.state,phase:clean(value.task.phase,20),updated_at:now});
    }
  }
  snapshot(now=seconds()) {
    const tasks=[...this.tasks.values()].filter(t=>now-t.updated_at<7200).sort((a,b)=>b.updated_at-a.updated_at).slice(0,4);
    const providers=this.platforms.map(id=>{
      const q=this.quotas.get(id); const fresh=q && now-q.updated_at<=90;
      const remaining=w=>fresh && w && (!w.resets_at || w.resets_at>now) && w.remaining_percent!==null?w.remaining_percent:-1;
      const reset=w=>fresh && w?.resets_at>now?w.resets_at-now:-1;
      return {id:clean(id,20),label:id==='claude'?'CC':id==='codex'?'CODEX':clean(id,12).toUpperCase(),
        weekly_percent:remaining(q?.seven_day),reset_seconds:reset(q?.seven_day),
        five_hour_available:!!(fresh && q?.five_hour),five_hour_percent:remaining(q?.five_hour),
        five_hour_reset_seconds:reset(q?.five_hour)};
    });
    const selected=providers.find(p=>p.id===(this.displayPlatform==='auto'?tasks[0]?.platform:this.displayPlatform)) || providers[0];
    const summaries={thinking:'思考与分析',coding:'编写代码',checking:'检查与验证',working:'处理任务'};
    return {schema_version:1,timestamp:new Date(now*1000).toISOString(),platform:selected?.id||'',
      weekly_percent:selected?.weekly_percent??-1,reset_seconds:selected?.reset_seconds??-1,providers,
      tasks_available:tasks.length>0,tasks:tasks.map(t=>({...t,summary:t.state==='running'?(summaries[t.phase]||'处理任务'):''})),
      running:tasks.filter(t=>t.state==='running').length,attention:tasks.filter(t=>t.state==='attention').length,
      failed:tasks.filter(t=>t.state==='failed').length,battery:{mic_bars:[-1,-1],remote_percent:-1}};
  }
}
