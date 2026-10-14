# 统一显示协议 v1

电脑数据服务默认在 TCP 8769 上提供只读 `GET /quota`（`/state` 为别名）。设备每三秒读取一次，响应限制为 6 KiB。成功连接与额度是否存在分别处理；额度不可用时仍返回有效任务数据。

```json
{
  "schema_version": 1,
  "timestamp": "2026-10-14T00:00:00Z",
  "platform": "claude",
  "weekly_percent": 41,
  "reset_seconds": 100000,
  "providers": [
    {
      "id": "claude", "label": "CC",
      "weekly_percent": 41, "reset_seconds": 100000,
      "five_hour_available": true,
      "five_hour_percent": 80,
      "five_hour_reset_seconds": 8100
    }
  ],
  "tasks_available": true,
  "tasks": [
    {"id": "example-session", "platform": "claude", "name": "示例任务",
     "state": "running", "phase": "coding", "summary": "编写代码", "updated_at": 1791936000}
  ],
  "battery": {"mic_bars": [-1, -1], "remote_percent": -1}
}
```

以上全部数字和任务都是协议示例，不是实时数据。百分比均为**剩余**，范围整数 0–100，`-1` 表示未知；剩余 0 不等于未知。`five_hour_available=false` 隐藏五小时行，不能用 100% 代替不受该窗口约束的情况。重置倒计时单位为秒，`-1` 表示未知。设备顶部按 `小时:分钟` 显示五小时窗口的倒计时；外圈始终表示七天窗口。

最多四个平台和四个任务，任务使用 `platform + id` 区分；切换任务时，设备选择同一 `platform` 的额度。上层平台可缺少任一窗口，两个窗口独立存在。电脑以最近 90 秒收到的额度样本为有效，窗口重置时间已经过去时降为未知。服务不会根据日期或套餐名制造新额度。

Codex 只从 `rateLimitsByLimitId.codex` 或对应的旧单桶读取数据，以 `windowDurationMins=300` 和 `10080` 明确识别窗口，不把 primary/secondary 的位置固定解释成五小时/七天，也不读取 reserve 桶。CC 从 Status Line 的 `rate_limits.five_hour` 和 `seven_day` 读取。来源字段是已用百分比，桥接层转换为剩余百分比。账号凭据由各平台自己管理，不上传到设备。

任务状态：`running`、`attention`、`done`、`waiting`、`paused`、`failed`、`offline`。运行阶段：`thinking`、`coding`、`checking`、`working`。顶层 `tasks_available` 不依赖额度。用户消息进入时工作，工具编辑时编码，权限请求时待回复，Stop 时本轮完成，Interrupt 时暂停；子 Agent 的结束不能替代主任务完成。任务缓存保留最长两小时，SessionEnd 删除对应任务。

## 社区接入

在用户私有配置 `~/.mosaico-agent-monitor/config.json` 的 `platforms` 中加入自己的平台标识（小写字母、数字、下划线或连字符，最长 20 字符，最多四个平台），使用 `host/adapters/example.mjs` 的 `publish()` 上报。上报对象形状为：

```js
await publish('myagent', {
  task: {id: 'session-1', name: 'Build project', state: 'running', phase: 'coding'},
  quota: {
    five_hour: {remaining_percent: actualRemaining, resets_at: actualResetEpoch},
    seven_day: null
  }
});
```

未知百分比使用 `null`；不支持的窗口使用 `null` 或省略。重置时间为 Unix 秒数。事件入口仅监听 `127.0.0.1:8770`，要求私有配置中的控制令牌，LAN 接口不接受写入。接口只保留展示字段，拒绝任意原始字段外传。

### 可选电量输入

电量采集文件使用原协议 `state=ready`、`checkedAt`（Unix 秒）、`microphones[].transmitters[]` 的 `slot/connected/batteryBars/batteryBarsMax`，以及 `remote.connected/checkedAt/batteryPercent`。只有 90 秒内且范围正确的记录会被转成 `mic_bars[2]` 和 `remote_percent`，源文件中其他设备信息不会出现在 LAN 响应中。

官方接口参考：[Codex App Server](https://learn.chatgpt.com/docs/app-server)、[Codex Hooks](https://learn.chatgpt.com/docs/hooks)、[CC Hooks](https://code.claude.com/docs/en/hooks)、[CC Status Line](https://code.claude.com/docs/en/statusline)。
