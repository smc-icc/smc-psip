# smc-psip

Windows 批量 SIP 软电话 / 呼叫模拟器。基于 [PJSIP](https://www.pjsip.org/) 与 [MicroSIP](https://www.microsip.org/) 改造，用于多账号注册、批量外呼、IVR DTMF、通话录音与监听。

当前版本 **1.1.0**。站点：[www.seami.cloud](http://www.seami.cloud/)

> 这是带界面的批量模拟客户端，不是专用压测引擎。并发上限受 PJSIP 编译宏约束（默认最多 1024 路通话）。

## 功能

- **批量账号**：添加外呼账号或 SIP 坐席，支持号码递增、CSV 导入
- **批量外呼**：队列调度，约 20 CPS，避免瞬时打满对端
- **媒体路径**：默认不把所有通话混进本机声卡；放音文件接到各路通话
- **DTMF / IVR**：接通后发送 `123#`，或按时间轴 `1@3|2@8`（第 3 秒发 1、第 8 秒发 2），每步只发一次
- **录音**：双向 / 本方 / 对方；范围可选全部、仅选中或最多 N 路；默认目录为 exe 旁的 `Recordings`
- **监听与麦广播**：只把当前监听的一路接到喇叭；可选把麦克风广播到全部通话
- **编解码**：默认 `PCMA/8000`、`PCMU/8000`
- **界面语言**：将 `langpack_simpchinese.txt` 放到 exe 同目录即可显示简体中文

## 环境要求

| 项 | 说明 |
| --- | --- |
| 系统 | Windows 10/11，建议 x64 |
| IDE | Visual Studio 2022（含使用 C++ 的桌面开发、MFC） |
| 解决方案 | `pjproject-vs14.sln` |
| 推荐配置 | **Release \| x64** |
| 其他 | [Git LFS](https://git-lfs.com/)、[7-Zip](https://www.7-zip.org/) |

x64 可执行文件使用动态 CRT，运行机器需要安装 [VC++ 2015–2022 x64 运行库](https://learn.microsoft.com/zh-cn/cpp/windows/latest-supported-vc-redist)。

## 获取源码与依赖

`third_party/` 源码树不进 Git，以 Git LFS 分卷归档。仓库里只跟踪 `third_party/build` 工程文件。

```bash
git lfs install
git clone <仓库地址>
cd smc-psip

# 从分卷解压到仓库根目录，与已有 third_party/build 叠在一起
7z x third_party.7z.001
```

工作区里如果已经有完整的 `third_party/`，不要删。解压只给干净克隆使用。

未安装 Git LFS 时，`third_party.7z.*` 只会是指针文件，解压会失败。

## 编译

1. 用 Visual Studio 打开 `pjproject-vs14.sln`
2. 配置选 **Release**，平台选 **x64**
3. 生成解决方案，或只生成项目 `smc-psip`
4. 输出：`x64\Release\smc-psip.exe`

### 平台差异

| 平台 | 说明 |
| --- | --- |
| **x64** | 推荐。视频走 DirectShow。仓库内 FFmpeg/SDL 预编译库只有 Win32，x64 已关闭 FFmpeg/SDL |
| **Win32** | 可开 FFmpeg/SDL 视频。应用侧使用静态 MFC |

不要混用 Win32 的 FFmpeg 库去链 x64。

## 运行

1. 把 `langpack_simpchinese.txt` 拷到 exe 同目录（需要中文界面时）
2. 同目录不要放多个 `langpack_*.txt`，程序只加载找到的第一个
3. 启动 `smc-psip.exe`
4. 配置账号 / 添加外呼任务后点开始

录音文件名格式：`{号码}_{yyyyMMddHHmmss}_{both|local|remote}.wav`。

## 使用要点

### 添加外呼

每条任务包含：号码、SIP 用户名、密码、域名、端口、被叫、通话时长、注册超时、DTMF。

- 用户名、号码需为数字
- **Start** 只对外呼任务入队，不会把坐席当外呼打出去
- 挂断后约 1 秒冷却再重呼

### DTMF

| 写法 | 含义 |
| --- | --- |
| `123#` | 接通后等待 `dtmfduration` 秒，整串发出 |
| `1@3\|2@8` | 从接通起算，第 3 秒发 `1`，第 8 秒发 `2` |

### 监听与录音

- 第一路接通会自动监听；右键可改听另一路
- 挂断后自动切到下一场仍接通的通话
- 监听和录音互相独立
- 同时开「监听 + 麦广播 + 外放」容易回声，建议耳机并打开回声消除

### 容量

编译期硬顶（`pjlib/include/pj/config_site.h`）：

- 通话 `PJSUA_MAX_CALLS`：1024
- 会议桥端口：1024
- Dialog / 事务 / IOQueue 已按批量场景放大

实际能跑多少路，还取决于机器、编解码和网络，而不是只看这些数字。

## 目录结构

```
smc-psip/                 应用（MFC）
pjlib/ pjlib-util/        PJSIP 基础库
pjmedia/ pjnath/ pjsip/   媒体、NAT、SIP
pjsip-apps/               libpjproject 聚合库
third_party/build/        第三方工程文件（进 Git）
third_party.7z.001–013    其余第三方源码与预编译库（Git LFS）
pjproject-vs14.sln        Visual Studio 解决方案
langpack_simpchinese.txt  简体中文语言包
```

## 许可

本项目按 **GNU GPL v2** 或更高版本发布，见 [COPYING](COPYING)。

PJSIP、MicroSIP 以及 `third_party` 中的编解码、SRTP 等组件各有其许可证，分发时请一并遵守。

## 致谢

- [PJSIP / PJPROJECT](https://www.pjsip.org/)
- [MicroSIP](https://www.microsip.org/)
