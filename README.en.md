# smc-psip

Windows batch SIP softphone / call simulator. Forked from [PJSIP](https://www.pjsip.org/) and [MicroSIP](https://www.microsip.org/) for multi-account registration, outbound campaigns, IVR DTMF, recording, and listen-in.

Current version **1.1.0**. [中文](README.md) · Download: [GitHub Releases](https://github.com/smc-icc/smc-psip/releases/latest)

> This is a GUI batch simulator, not a dedicated load tester. The call ceiling is set by PJSIP compile macros (1024 calls by default).

Screenshots below show the **Simplified Chinese** UI. Each figure has a label map (on-screen text → English).

## Features

- **Batch accounts**: outbound tasks or SIP agents, number increment, CSV import
- **Outbound queue**: about 20 CPS so the far end is not flooded
- **Media path**: calls are not mixed onto the PC sound card by default; playback files attach per call
- **DTMF / IVR**: send `123#` after answer, or a timeline `1@3|2@8` (digit 1 at 3 s, digit 2 at 8 s), each step once
- **Recording**: right-click Both / Local / Remote; nothing is recorded unless a mode is chosen. Default folder: `records` next to the exe
- **Listen-in and mic broadcast**: only the listened call goes to the speaker; optional mic to all calls
- **Codecs**: default `PCMA/8000`, `PCMU/8000`
- **Language**: Settings → Language → Simplified Chinese, with `langpack_zh.txt` beside the exe

## Requirements

| Item | Notes |
| --- | --- |
| OS | Windows 10/11, x64 recommended |
| IDE | Visual Studio 2022 (Desktop C++, MFC) |
| Solution | `pjproject-vs14.sln` |
| Config | **Release \| x64** |
| Other | [Git LFS](https://git-lfs.com/), [7-Zip](https://www.7-zip.org/) |

The x64 build uses the dynamic CRT. Install the [VC++ 2015–2022 x64 redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist).

## Source and dependencies

The `third_party/` tree is not in Git as source. It is a Git LFS split archive. Only `third_party/build` project files are tracked.

```bash
git lfs install
git clone <repo-url>
cd smc-psip
7z x third_party.7z.001
```

Do not delete an existing full `third_party/`. Extract only on a clean clone. Without Git LFS the `third_party.7z.*` files are pointers and extract will fail.

## Build

1. Open `pjproject-vs14.sln` in Visual Studio
2. Configuration **Release**, platform **x64**
3. Build the solution or the `smc-psip` project
4. Output: `x64\Release\smc-psip.exe` (sometimes `smc-psip\x64\Release\`)

| Platform | Notes |
| --- | --- |
| **x64** | Recommended. Video via DirectShow. FFmpeg/SDL prebuilds in-tree are Win32 only; disabled on x64 |
| **Win32** | FFmpeg/SDL video possible. App uses static MFC |

Do not link Win32 FFmpeg into an x64 build.

## Run

1. Copy `langpack_zh.txt` next to the exe
2. Settings → Language → Simplified Chinese, save, restart
3. Start `smc-psip.exe`
4. Add agents or outbound tasks, then click Start

Recording name: `{number}_{yyyyMMddHHmmss}_{both|local|remote}.wav`.

---

# User guide

## 1. Main window

![Figure 1 Main window](docs/manual/fig1-main.png)

| On screen | English |
| --- | --- |
| 模拟呼叫 | Simulated call (tab) |
| 麦克风广播 | Microphone broadcast |
| 监听: 无 | Listen: none |
| 开始 | Start |
| 结束 | Stop |
| 帐号 | Account / number |
| 用户 | Name / auth user |
| 密码 | Password |
| 域名 | Domain / SIP server |
| 端口 | Port |
| 呼叫号码 | Called number |
| 按键 | DTMF |
| 按键间隔 | Seconds after answer before DTMF |
| 间隔时间 | Call duration (seconds) |
| 注册周期 | Registration interval (seconds) |
| 状态 | Status |
| 录音 | Recording |
| 质量监控 | Quality / RTCP |
| 已注册 / 未注册 | Registered / Unregistered |
| 拨号中 | Calling |
| 电话呼入 | Incoming |
| Early | Early |
| 连接中 / 已连接 | Connecting / Connected |

**Start** queues **outbound tasks** only. SIP agents register and answer; they are not dialed as outbound. **Stop** ends the outbound queue and related calls. The search box at the bottom-left filters the list.

## 2. Batch SIP agents

### Figure 2: context menu

Right-click **empty space** in the list, choose **增加 SIP 代理** (Add SIP agent).

![Figure 2 Add SIP agent](docs/manual/fig2-add-agent-menu.png)

| On screen | English |
| --- | --- |
| 增加呼叫任务 | Add call task |
| 增加SIP代理 | Add SIP agent |
| 编辑 / 复制 / 删除 | Edit / Copy / Delete |
| 导入 | Import |

### Figure 3: agent dialog

![Figure 3 SIP agent dialog](docs/manual/fig3-sip-agent-dlg.png)

| On screen | English | Meaning |
| --- | --- | --- |
| 帐号 | Number | Starting SIP account, digits only, e.g. `990001` |
| 用户 | Name | Auth username, digits; increments with Number |
| 密码 | Password | Register password |
| 服务地址 | Server / URL | SIP host or IP |
| 端口 | Port | Usually `5060` |
| 注册周期 | Reg timeout | Refresh interval in seconds, e.g. `3600` |
| 帐号数 | Count | How many rows to add (e.g. `10` → 990001–990010) |
| 自动应答 | Auto answer | Incoming calls answered automatically |
| 确认 / 取消 | OK / Cancel | Save or close |

Existing numbers are skipped.

### Figure 4: after add

![Figure 4 Agents in the list](docs/manual/fig4-agents-added.png)

| On screen | English |
| --- | --- |
| 未注册 | Unregistered |
| 关 | Recording off |

Called number is empty for agents. Click **Start** to register.

## 3. Row context menu

Select one or more rows, then right-click. With no selection, Call / Edit / Copy / Delete / Listen / Record stay disabled.

![Figure 5 Row context menu](docs/manual/fig5-agent-context.png)

| On screen | English | Action |
| --- | --- | --- |
| 呼叫 | Call | Audio call for the selected account |
| 视频呼叫 | Video call | Video call |
| 信息 | Message | Message window |
| 增加呼叫任务 | Add call task | Outbound batch dialog (section 4) |
| 增加SIP代理 | Add SIP agent | Agent batch dialog (section 2) |
| 编辑 | Edit | Edit the current row |
| 复制 | Copy | Copy selected rows as CSV (header included) |
| 删除 | Delete | Delete all selected rows |
| 监听此通话 | Listen this call | Far-end audio to the PC speaker (one call at a time) |
| 双向录音 | Record both | Local + remote; click again to stop |
| 本方录音 | Record local | Local only (playback / mic) |
| 对方录音 | Record remote | Remote only |
| 导入 | Import | Submenu → CSV (`import_calls_template.csv`) |

Recording starts the file only after the call is confirmed. Changing mode closes the old file and opens a new one. Default folder: `records` beside the exe.

## 4. Batch outbound

Right-click empty space, **增加呼叫任务** (Add call task).

![Figure 6 Add call task](docs/manual/fig6-add-call-task.png)

The **Call Task** dialog fields:

| Field | Meaning |
| --- | --- |
| Number / Name | Starting account and username (digits) |
| Passwd | Password |
| Url | SIP server |
| Port | Usually `5060` |
| Called | Destination; optional increment with Number |
| Increase (Number / Called) | Add 1 per created row |
| DTMF | Digits after answer, optional |
| DTMF duration | Seconds after answer before DTMF (`0` = immediately) |
| CallsCount | How many outbound rows to create |
| CallPeriod | Talk time in seconds |
| RegTimeout | Registration refresh |

**DTMF**

| Pattern | Meaning |
| --- | --- |
| `123#` | After answer and DTMF delay, send the whole string |
| `1@3\|2@8` | From answer: send `1` at 3 s, `2` at 8 s |

**Start** dials registered outbound tasks from a queue (~20 CPS). About 1 s cooldown after hangup before redial.

CSV template `import_calls_template.csv`: `Type=0` outbound, `Type=1` agent. See `import_calls_template.txt`.

## 5. Settings

### Figure 7: menu

Click the menu button at the far right of the toolbar (next to Start / Stop).

![Figure 7 Main menu](docs/manual/fig7-main-menu.png)

Choose **设置** (Settings).

### Figure 8: all settings

![Figure 8 Settings](docs/manual/fig8-settings.png)

| On screen | English | Meaning |
| --- | --- | --- |
| 语言 | Language | English = no pack; 简体中文 = `langpack_zh.txt`. Restart required |
| 单呼叫模式 | Single call mode | Phone tab instead of the batch list |
| Echo To Remote | Echo to remote | Send local playback to the far end |
| 响铃声音 | Ringing sound | Custom ring wav |
| 响铃设备 / 扬声器 / 麦克风 | Ring / speaker / mic | Audio devices (Default is fine) |
| 录音目录 | Record directory | Wav output, default `exe\records` |
| 放大麦克风音量 | Mic amplification | Software mic gain |
| 软件水平调整 | Software level adjustment | Software volume |
| 可用编码 / 启用编码 | Available / enabled codecs | Space enables, Delete disables; arrows set priority. Default G.711 |
| VAD | VAD | Voice activity detection |
| EC | EC | Echo canceler |
| 强制呼入的编码 | Force codec for incoming | Force local codec on inbound |
| 摄像头 / 视频编码 / 码速率 | Camera / video codec / bitrate | Video capture and encode |
| H.264 / H.263+ / VP8 | Video codecs | Enable extra video codecs |
| 源端口 | Source port | Local SIP port, `0` = auto |
| rport | rport | Discover public port via Via |
| RTP端口 | RTP ports | RTP range, `0` = auto |
| Use DNS SRV | Use DNS SRV | Resolve SIP via SRV |
| STUN 服务器 | STUN server | NAT traversal |
| DTMF 传递方法 | DTMF method | Auto / in-band / RFC2833 / SIP INFO |
| 自动应答 | Auto answer | No / button / SIP header / all |
| 拒绝来电 | Deny incoming | No / button / user-or-domain filter / all |
| 用户目录 | Directory of users | Optional user-directory URL |
| 多媒体控制按钮 | Handle media buttons | Keyboard media keys |
| 声音事件 | Sound events | Ring / hangup sounds |
| 启用日志文件 | Enable log file | Log beside the exe |
| 来电时显示在最前面 | Bring to front on incoming | Raise the window |
| 开启本地账号 | Enable local account | Local SIP account without a server |
| 随机位置 | Random popup position | Random incoming-dialog position |
| 检查更新 | Check for updates | Daily / weekly / monthly / quarterly / never. Queries [GitHub Releases](https://github.com/smc-icc/smc-psip/releases/latest) |
| 保存 / 取消 | Save / Cancel | Apply or close |

Language change needs a restart.

## 6. Multi-select

- **Ctrl+A**: select all when the list has focus
- **Shift+click**: range from the anchor to the clicked row
- **Drag with the left button**: select a contiguous block

Then batch delete, copy, or set recording mode.

---

## Tips

- The first confirmed call is listened automatically; right-click to change; after hangup the next live call is used
- Listen and record are independent
- Listen + mic broadcast + speakers can echo; use a headset and enable EC

### Capacity

Compile-time caps (`pjlib/include/pj/config_site.h`):

- `PJSUA_MAX_CALLS`: 1024
- Conference ports: 1024
- Dialog / transaction / IOQueue scaled for batch use

Real capacity still depends on CPU, codecs, and the network.

## Layout

```
smc-psip/                 MFC app
pjlib/ pjlib-util/        PJSIP core
pjmedia/ pjnath/ pjsip/   media, NAT, SIP
pjsip-apps/               libpjproject
third_party/build/        third-party project files (in Git)
third_party.7z.001–013    remaining third-party sources (Git LFS)
pjproject-vs14.sln        Visual Studio solution
langpack_zh.txt           Simplified Chinese pack
import_calls_template.csv import template
docs/manual/              screenshots
```

## License

GNU GPL v2 or later. See [COPYING](COPYING).

PJSIP, MicroSIP, and codecs/SRTP under `third_party` have their own licenses.

## Credits

- [PJSIP / PJPROJECT](https://www.pjsip.org/)
- [MicroSIP](https://www.microsip.org/)
