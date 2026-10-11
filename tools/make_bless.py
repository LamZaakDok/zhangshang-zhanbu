#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
生成启动第二页: 黄色"祝君好运"(行楷), 黑底, 存 data/bless.jpg (135x240)。
换字体改 FONT; 换颜色改 COLOR。
用法: python tools/make_bless.py
"""
import os
from PIL import Image, ImageDraw, ImageFont

W, H, S = 135, 240, 4
FONT = r"C:\Windows\Fonts\STXINGKA.TTF"     # 华文行楷; 可换 simkai.ttf / STLITI.TTF
COLOR = (255, 214, 0)                        # 金黄
TEXT = "祝君好运"

canvas = Image.new("RGB", (W * S, H * S), (0, 0, 0))
d = ImageDraw.Draw(canvas)

size = 30 * S
while size > 8:
    font = ImageFont.truetype(FONT, size)
    bbox = d.textbbox((0, 0), TEXT, font=font)
    tw, th = bbox[2] - bbox[0], bbox[3] - bbox[1]
    if tw <= W * S - 20 and th <= H * S - 40:
        break
    size -= 2

x = (W * S - tw) / 2 - bbox[0]
y = (H * S - th) / 2 - bbox[1]
d.text((x, y), TEXT, font=font, fill=COLOR)

img = canvas.resize((W, H), Image.LANCZOS)
os.makedirs("data", exist_ok=True)
img.save("data/bless.jpg", "JPEG", quality=92, optimize=True)
img.save("tools/bless_preview.png")
print("wrote data/bless.jpg (字体 %s, 字号 %d)" % (FONT.split("\\")[-1], size // S))
