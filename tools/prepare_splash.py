#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
启动图处理: 把 tools/splash_src.png 适配成 135x240 的 data/splash.jpg。
- 按宽度缩放到 135, 垂直居中;
- 透明背景合成到白底;
- 上下留白处由固件叠加"掌上占卜/祝君好运"两行字。

用法: python tools/prepare_splash.py
"""
import os
from PIL import Image

W, H = 135, 240
BG = (0, 0, 0)          # 与主界面一致的底色(黑)

src = Image.open("tools/splash_src.png").convert("RGBA")
sw, sh = src.size
nw = W
nh = int(round(sh * (W / sw)))
im = src.resize((nw, nh), Image.LANCZOS)

base = Image.new("RGB", (nw, nh), BG)
base.paste(im, (0, 0), im)          # 用 alpha 合成到白底

canvas = Image.new("RGB", (W, H), BG)
canvas.paste(base, (0, (H - nh) // 2))

os.makedirs("data", exist_ok=True)
canvas.save("data/splash.jpg", "JPEG", quality=92, optimize=True)
canvas.save("tools/splash_preview.png")
print("wrote data/splash.jpg (%dx%d, 图高 %d)" % (W, H, nh))
