# StickS3 占卜固件

M5Stack **StickS3**（ESP32-S3）上的占卜固件：**抽签 / 六爻 / 塔罗**。
抽签支持三个庙系，各自一百签：**灵隐寺·观音灵签 / 黄大仙祠·灵签 / 浅草寺·观音签**。
平台：PlatformIO + Arduino + M5Unified。

```
fortune-stick/
├─ platformio.ini          编译配置
├─ partitions.csv          3MB 应用 + ~4.9MB LittleFS
├─ src/
│  ├─ main.cpp             主菜单状态机
│  ├─ ui.h / ui.cpp        中英混排显示、分页、卦象绘制
│  ├─ rng.h                硬件真随机
│  ├─ store.h              CSV 读取工具
│  ├─ draw_lots.cpp        抽签(多庙系 + 按真实吉凶权重抽取)
│  ├─ tarot.cpp            塔罗(插画牌面)
│  ├─ liuyao.cpp           六爻(三枚铜钱法)
│  └─ cnfont_generated.h   由脚本生成的中文字库(不提交)
├─ data/                   会上传到 LittleFS 的素材
│  ├─ gua64.csv            64 卦全表
│  ├─ tarot.csv            78 张牌义
│  ├─ lots_lingyin.csv     灵隐寺 观音灵签(100)
│  ├─ lots_wongtaisin.csv  黄大仙祠 灵签(100)
│  ├─ lots_sensoji.csv     浅草寺 观音签(100)
│  └─ tarot/00.jpg..77.jpg 牌面图(脚本生成)
└─ tools/
   ├─ gen_font.py          中文字库子集生成(扫 data/ 与 src/, 含缺字检测)
   ├─ prepare_tarot.py     塔罗图片预处理(自有素材)
   ├─ fetch_tarot.py       自动抓取 RWS 全套牌面
   ├─ import_lots.py       签文 JSON -> CSV 导入
   └─ verify_gua.py        六爻装卦校验(核对纳甲/六亲/世应)
```

## 环境准备

1. VSCode + **PlatformIO** 插件（首次会自动装 ESP32 平台与 M5Unified）。
2. Python 3 + Pillow：`pip install pillow`。

## 签文数据（已内置）

三家一百签已由公开数据集导入 `data/`，无需手工录入：

| 庙系 | 文件 | 数据来源 |
|---|---|---|
| 灵隐寺·观音灵签 | `lots_lingyin.csv` | `yanxinyu777-beep/guanyin-lingqian-100` |
| 黄大仙祠·灵签 | `lots_wongtaisin.csv` | `wickes1/wong-tai-sin` |
| 浅草寺·观音签 | `lots_sensoji.csv` | `Tamshen/senso-ji-stick-data` |

要重新生成（例如换数据源或更新）：

```bash
python tools/import_lots.py     # 读 tools/raw/ 下的 JSON -> data/*.csv
```

`tools/raw/` 里是下载的原始 JSON（已在 .gitignore 中，可重新下载）。
每套 CSV 的列统一为 `no,level,title,verse,explain`，多行用 `|` 分隔。


## 三步走

### 1. 准备素材

**中文字库（必做，否则中文不显示）** —— 只嵌入 data 里真正用到的字：

```bash
python tools/gen_font.py --ttf C:/Windows/Fonts/msyh.ttc
# 生成 src/cnfont_generated.h
```

脚本会报告字数与体积，并**检查缺字**（某字体缺某个字会提示，换覆盖更全的字体即可）。

**塔罗牌面** —— 两种方式二选一：

- **自动抓取（推荐）**：直接下整套公有领域 1909 Rider–White–Smith 并预处理：

  ```bash
  python tools/fetch_tarot.py
  # 生成 data/tarot/00.jpg .. 77.jpg (自动断点续传, 可重复执行)
  ```

- **自带素材**：把 78 张源图放进 `tools/tarot_src/`，然后：

  ```bash
  python tools/prepare_tarot.py
  # 生成 data/tarot/00.jpg .. 77.jpg (116x200 JPEG)
  ```

牌面来自公有领域的 1909 Rider–White–Smith 牌组
（图源 `mixvlad/TarotCards`，MIT）。

### 2. 编译并上传固件

```bash
pio run -t upload
```

### 3. 上传素材到文件系统

```bash
pio run -t uploadfs
```

串口屏 115200 可看日志。

## 操作

- **主菜单**：`A` 切换项，`B` 进入。三个模块内 `B` 返回主菜单。
- **抽签**：先选庙系（灵隐寺 / 黄大仙祠 / 浅草寺）→ **摇一摇或按 A** 起签 →
  `A` 翻页（签诗 / 解曰）→ 末页 `A` 重抽。各庙签文本身即按真实吉凶比例收录，等概率抽取。
  菜单里的 **历史签** 记录最近 30 次抽取（存 NVS，掉电不丢）。
- **塔罗**：选牌阵——**单张** / **三张（过去·现在·未来）** / **凯尔特十字（10 张）**。
  三张与凯尔特十字先看概览（凯尔特十字为十字+权杖的图解布局，绿=正 红=逆），
  再逐张看牌面与牌义；逆位以 180° 旋转呈现。
- **六爻**：先**择今日天干**（离线手选，定六神）→ 掷币成卦 → **择问何事**
  （求财/功名/婚姻/疾病/出行/子女，定用神）→ 依次查看：
  本卦 → **本卦装卦** → 变卦 → **变卦装卦** → 动爻 → **用神断语**。
  装卦含**八宫、世应、纳甲干支、五行、六亲**与**六神**；
  断语依用神是否上卦/持世/发动、以及用神与世爻、变爻的生克给简明提示。
  动爻左侧红标、世爻橙、应爻青、六神各具配色。

## 扩展指南

- **加签**：编辑对应庙系的 CSV（列 `no,level,title,verse,explain`），
  升 `no` 到 1..100 补全。重新 `uploadfs`，并再跑一次 `gen_font.py`（有新汉字时）
  后重新编译。若要增删庙系，改 `draw_lots.cpp` 里的 `SOURCES[]` 与吉凶权重表。
- **改卦辞**：`data/gua64.csv` 的 `note` 列可换成完整卦辞/断语。
- **加牌阵**：在 `tarot.cpp` 里把单张抽牌扩展为三张（过去/现在/未来），
  屏幕宽度不足时分屏显示。
- **约定**：所有 CSV 一律用**半角逗号**作分隔符，文案内部只用**全角标点**。

## 已知限制

- 屏幕仅 135x240，详细卦辞/牌义需分页；`drawWrapped` 已按宽度自动折行。
- 逆位用贴图旋转实现；若某台设备 `pushRotateZoom` 表现异常，可改为解码后手动翻转。
- 六爻装卦已含八宫 / 世应 / 纳甲干支 / 五行 / 六亲 / 六神 与动爻。**变卦按变卦自身的宫装六亲**，
  与主卦各自独立；若你所用门派以主卦之宫统装变爻，可在 `liuyao.cpp` 的 `zhuangGua()` 调整。
- **六神依日干起**，设备无 RTC/日历，故起卦前手动选择今日天干（记忆上次选择）。
  如日后接入 WiFi NTP，可改为自动推算日干。
- 六爻**用神断语为简明版**：只据用神持世/发动、及用神与世爻、变爻的生克关系提示；
  完整断卦还需月建日辰、旬空、伏神、进退等，未纳入。
- 抽签历史存于 NVS（命名空间 `fortune`，键 `lots`），最多保留最近 30 条。
- 字库仅 16x16 点阵，`gen_font.py --size` 目前只支持 16。
