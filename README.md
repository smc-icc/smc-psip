# smc-psip

SIP 批量模拟器（基于 MicroSIP / PJSIP）。

## 获取依赖

克隆后先把仓库根目录的分卷 `third_party.7z.001` 解压到仓库根目录，得到 `third_party/`（与现有 `third_party/build` 工程文件叠在一起），再打开 `pjproject-vs14.sln` 编译 `smc-psip`。

```
7z x third_party.7z.001
```

`third_party/` 本体不进 Git，归档走 Git LFS。工作区里已有的 `third_party` 目录不要删，解压是给干净克隆用的。
