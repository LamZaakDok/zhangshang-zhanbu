# 发布到 M5Burner

本目录是把「掌上占卜」发布到 [M5Burner](https://docs.m5stack.com/zh_CN/uiflow/m5burner/intro) 所需的材料。

## 需要的文件

| 文件 | 说明 | 从哪来 |
|---|---|---|
| `zhangshang-zhanbu-full.bin` | **整片烧录镜像**（bootloader+分区+固件+素材，从 `0x0` 烧录） | 从本仓库 [Releases](../../releases) 下载同名附件 |
| `cover.png` | 封面图 | 本目录 |
| `payload.json` | 发布参数（CLI 用） | 本目录 |

> 说明：M5Burner 的「USER CUSTOM」固件是**单个 bin**，所以用 `zhangshang-zhanbu-full.bin`（把 2 个分区合并成了一片，烧到 `0x0`）。

## 方式一：客户端发布（推荐，最简单）

1. 下载并打开 M5Burner 桌面客户端，右上角**登录 M5Stack 社区账号**。
2. 左下角点 **USER CUSTOM → Publish**。
3. 按下面填写：

   | 字段 | 值 |
   |---|---|
   | Name | `掌上占卜` |
   | Version | `1.0.0` |
   | Description | 见 `payload.json` 的 `description` |
   | Device Type | **StickS3** |
   | Github | `https://github.com/LamZaakDok/zhangshang-zhanbu` |
   | FirmWare | 上传 `zhangshang-zhanbu-full.bin` |
   | Cover | 上传 `cover.png` |

4. 点 **Upload**。成功后可在 USER CUSTOM 看到；
   - **Publish** 开关控制公开/私有；
   - **Share** 生成 `Share Code`，可用 **Share Burn** 让别人直接烧。

## 方式二：命令行发布（m5-burner-js）

```bash
# 需要 Node.js；用你的 M5Stack 账号登录拿 token
npx -y m5-burner login <你的邮箱> <密码>

# 把 firmware 放到本目录(从 Releases 下载)，然后：
export M5_AUTH_TOKEN="<上一步返回的 token>"
npx -y m5-burner publish-firmware payload.json
```

## 方式三：进入官方固件目录（长期）

把固件按官方仓库 [m5stack/M5Stack-Firmware](https://github.com/m5stack/M5Stack-Firmware) 的规则整理
（根目录 `m5burner.json` + `firmware/` 目录，文件名需为 `名称_0x地址.bin`），
再提 PR，审核通过后固件会出现在 M5Burner 官方列表里。此后更新无需再提 PR。

## 备注

- 首次烧录建议先在设备上 **Erase** 再 Burn（会清空已保存数据）。
- 若只想自己/朋友用，客户端方式一 + Share Code 就够。
