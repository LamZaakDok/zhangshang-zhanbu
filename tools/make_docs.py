#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""生成 README 用的界面示意图(1x 逻辑分辨率放大 4 倍)。"""
import os
from PIL import Image, ImageDraw, ImageFont

W, H, S = 135, 240, 4
FONT = os.path.join("tools", "raw", "unifont.otf")
if not os.path.exists(FONT):
    raise SystemExit("先运行 tools/gen_font.py 下载 Unifont")

f16 = ImageFont.truetype(FONT, 16)
img = Image.new("RGB", (W, H), (0, 0, 0))
d = ImageDraw.Draw(img)

CYAN = (0, 255, 255)
GRAY = (180, 190, 200)
BLACK = (0, 0, 0)
SEL = (8, 12, 40)

def tw(s):
    return int(d.textlength(s, font=f16))

def center(y, s, col):
    d.text(((W - tw(s)) // 2, y), s, font=f16, fill=col)

def left(x, y, s, col):
    d.text((x, y), s, font=f16, fill=col)

# 状态栏(居中标题)
d.rectangle([0, 0, W, 18], fill=BLACK)
center(2, "掌上占卜", CYAN)
d.line([0, 18, W, 18], fill=(40, 40, 48))

# 菜单
items = ["抽签", "塔罗", "六爻", "掷杯筊", "关于"]
for i, it in enumerate(items):
    y = 28 + i * 38
    if i == 0:
        d.rectangle([0, y, W, y + 34], fill=SEL)
        d.rectangle([0, y, 3, y + 34], fill=CYAN)
    left(14, y + 9, it, CYAN if i == 0 else GRAY)

# 底部: 提示 + 电量
d.line([0, H - 18, W, H - 18], fill=(40, 40, 48))
left(2, H - 15, "确认 蓝色按键", (120, 130, 140))
right = "88%"
d.text((W - 4 - tw(right), H - 15), right, font=f16, fill=(230, 230, 230))

os.makedirs("docs", exist_ok=True)
img.resize((W * S, H * S), Image.NEAREST).save("docs/preview.png")
print("wrote docs/preview.png")
