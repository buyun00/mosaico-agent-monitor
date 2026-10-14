# Mosaico Ideas 投稿材料

标题：**Mosaico Agent Monitor：CC / Codex 动画任务与双窗口额度看板**

简介：让 ESP-Mosaico V1.2 显示电脑上的 Claude Code 和 Codex 任务，随着工作、思考、待回复、空闲、暂停和完成切换初音动画。七天剩余额度通过外圈和大数字显示；支持五小时窗口的账号在顶部显示剩余比例和重置倒计时。保留热点配网，新增 Wi-Fi 扫描，联网后从局域网电脑脚本获取数据。

硬件：ESP-Mosaico V1.2，ESP32-S31，16 MiB Flash。

电脑端：Windows x64 即用包；Node.js 源码供其他系统使用。需电脑保持服务运行。设备不需要 AI 账号或密钥。当前提供 CC/Codex 适配，其他平台有统一协议与示例。

源码：https://github.com/buyun00/mosaico-agent-monitor

下载：https://github.com/buyun00/mosaico-agent-monitor/releases/tag/v0.15.0

封面建议：`docs/images/plus-both-windows.png`。动画：Release 的生产渲染 MP4。两者均为主机渲染示例，应注明“生产渲染预览，非真机拍摄”。

安装说明：使用 Windows 包里的四镜像刷机脚本保留 NVS/PHY；平台刷机工具若使用合并的 full.bin，应从 0x0 写入，并告知用户它会清空已有配网。表单登录后按实际允许的格式选择完整 BIN 或镜像 ZIP。版本验证边界见 verification.md。

活动要求：先公开分享至少一个固件项目，再填写联系邮箱报名。不要把项目发布和活动报名当作同一个步骤。活动页：https://mosaico-ideas.espressif.com/activities/mosaico-power-board
