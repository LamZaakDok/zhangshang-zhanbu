#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""生成 M5Burner 封面图 m5burner/cover.png。"""
import os
from PIL import Image, ImageDraw, ImageFont

W, H = 640, 400
TITLE_FONT = r"C:\Windows\Fonts\STXINGKA.TTF"
SUB_FONT = r"C:\Windows\Fonts\msyh.ttc"

canvas = Image.new("RGB", (W, H), (10, 12, 26))
d = ImageDraw.Draw(canvas)
# 顶部/底部渐深
for y in range(H):
    t = y / (H - 1)
    r = int(18 * (1 - t) + 8 * t); g = int(22 * (1 - t) + 10 * t); b = int(46 * (1 - t) + 22 * t)
    d.line([(0, y), (W, y)], fill=(r, g, b))

# 左侧: 开机签筒图
splash = Image.open("data/splash.jpg").convert("RGB")
sh = 330
sw = int(splash.width * sh / splash.height)
canvas.paste(splash.resize((sw, sh), Image.LANCZOS), (34, 40))

# 右侧: 多屏示意图
shot = Image.open("docs/screens.png").convert("RGB")
tw = W - (34 + sw + 24) - 24
th = int(shot.height * tw / shot.width)
canvas.paste(shot.resize((tw, th), Image.LANCZOS), (34 + sw + 24, (H - th) // 2))

# 标题与副标题(叠在右上部) —— 这里改为右上角单独一块
title = ImageFont.truetype(TITLE_FONT, 46)
sub = ImageFont.truetype(SUB_FONT, 20)
tx = 34 + sw + 24
d.text((tx, 26), "掌上占卜", font=title, fill=(255, 214, 0))
d.text((tx, 84), "StickS3 · 抽签/塔罗/六爻/掷杯筊", font=sub, fill=(210, 216, 226))
# 再压一层(示意图下移一点, 避免压住标题)
canvas.paste(shot.resize((tw, int(shot.height * (tw * 0.94) / shot.width)), Image.LANCZOS),
             (tx, 120))

os.makedirs("m5burner", exist_ok=True)
canvas.save("m5burner/cover.png")
print("wrote m5burner/cover.png")
