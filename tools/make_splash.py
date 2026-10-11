#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
启动图生成: 黄色签筒(签筒内插着数支签), 无文字。
输出 data/splash.jpg (135x240)。固件启动时全屏显示, 并在其上叠加"掌上占卜/祝君好运"。
用法: python tools/make_splash.py
"""
import os
from PIL import Image, ImageDraw

W, H = 135, 240
S = 4                      # 超采样倍数
CW, CH = W * S, H * S

img = Image.new("RGB", (CW, CH), (0, 0, 0))
d = ImageDraw.Draw(img)

# ---- 背景: 深蓝夜色竖向渐变 ----
for y in range(CH):
    t = y / (CH - 1)
    r = int(26 * (1 - t) + 8 * t)
    g = int(28 * (1 - t) + 10 * t)
    b = int(58 * (1 - t) + 24 * t)
    d.line([(0, y), (CW, y)], fill=(r, g, b))

cx = CW // 2
tube_x0 = cx - 33 * S
tube_x1 = cx + 33 * S
tube_top = 100 * S
tube_bot = 196 * S

# ---- 签(竹签): 先画, 让筒身盖住下端 ----
sticks = [(-30, 60), (-20, 52), (-8, 56), (4, 50), (16, 58), (27, 62)]
for i, (ox, top) in enumerate(sticks):
    x = cx + ox * S
    w = 9 * S
    ytop = top * S
    ybot = tube_top + 34 * S
    body = (226, 208, 150)
    edge = (176, 156, 96)
    # 轻微交替色, 增加层次
    if i % 2:
        body, edge = (214, 194, 132), (162, 142, 84)
    d.rounded_rectangle([x - w // 2, ytop, x + w // 2, ybot],
                        radius=w // 2, fill=body, outline=edge, width=max(1, S // 2))
    # 签顶红头
    d.rounded_rectangle([x - w // 2, ytop, x + w // 2, ytop + 12 * S],
                        radius=w // 2, fill=(196, 60, 44))

# ---- 筒身: 水平渐变(中间亮, 两侧暗)的黄色圆柱 ----
for x in range(tube_x0, tube_x1):
    u = (x - tube_x0) / (tube_x1 - tube_x0 - 1)
    k = 1.0 - abs(u - 0.5) * 1.35          # 0.32 .. 1.0
    r = int(150 + 105 * k)
    g = int(122 + 92 * k)
    b = int(28 + 46 * k)
    d.line([(x, tube_top), (x, tube_bot)], fill=(r, g, b))

# ---- 上下圆角(椭圆收口) ----
d.ellipse([tube_x0, tube_bot - 9 * S, tube_x1, tube_bot + 9 * S],
          fill=(198, 162, 44))                 # 筒底
d.ellipse([tube_x0, tube_top - 9 * S, tube_x1, tube_top + 9 * S],
          fill=(255, 224, 120))                # 筒口(亮)
# 筒口内沿阴影, 增强立体
d.ellipse([tube_x0 + 4 * S, tube_top - 5 * S, tube_x1 - 4 * S, tube_top + 5 * S],
          fill=(150, 118, 30))
# 筒口外圈高光带
d.arc([tube_x0 - 2 * S, tube_top - 11 * S, tube_x1 + 2 * S, tube_top + 11 * S],
      200, 340, fill=(255, 240, 170), width=2 * S)

# ---- 筒身竖向高光条 ----
d.rounded_rectangle([cx - 22 * S, tube_top + 6 * S, cx - 14 * S, tube_bot - 6 * S],
                    radius=4 * S, fill=(255, 236, 150))

# ---- 底部投影 ----
d.ellipse([tube_x0 + 6 * S, tube_bot + 4 * S, tube_x1 + 6 * S, tube_bot + 14 * S],
          fill=(10, 12, 22))

img = img.resize((W, H), Image.LANCZOS)

os.makedirs("data", exist_ok=True)
img.save("data/splash.jpg", "JPEG", quality=90, optimize=True)
img.save("tools/splash_preview.png")     # 便于查看
print("wrote data/splash.jpg and tools/splash_preview.png")
