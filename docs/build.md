# 构建固件和电脑端

目标为 ESP-Mosaico V1.2 / ESP32-S31 / 16 MiB，不适用于 ESP32-S3 或 V1.0。验证环境是 ESP-IDF 6.2 预览 SDK，riscv32-esp-elf GCC 16.1.0（esp-16.1.0_20260609）。使用支持 `esp32s31` 的配套 SDK 和工具链，依赖组件由 `firmware/main/idf_component.yml` 与 `dependencies.lock` 固定。

在已经启用该 SDK 的终端中：

```sh
idf.py --preview -C firmware build
node --test tests/*.test.mjs
```

Windows 建议将项目放在没有空格的短路径；也可使用 `subst` 建立无空格入口。初音资源已放在 `assets/miku-animation.bin`，构建会将其登记为同伴镜像，不需要下载生成工具或原始大图集。

构建产物位于 `firmware/build`。以 `flasher_args.json` 为镜像位置的权威来源；本版是：

| 镜像 | 地址 |
|---|---|
| bootloader | `0x2000` |
| 分区表 | `0x8000` |
| 应用 | `0x10000` |
| 动画资源 | `0x310000` |

应用分区为 3 MiB，动画资源分区为 8 MiB。本次公开目录完整构建生成的应用约 2.9 MiB，仍有约 4% 空间；以后新增功能须继续检查容量。

电脑端没有 npm 运行依赖。Windows 即用包包含 Node.js 24.15.0 和 esptool 5.4.0；后者用 PyInstaller 6.16.0 打包，入口为 `scripts/esptool-entry.py`。macOS/Linux 可以安装 Node.js 22+ 和支持 ESP32-S31 的 esptool，再运行 `scripts/flash.py --port PORT`。不把本机 SDK、账号配置或 Flash 备份加入公开仓库。
