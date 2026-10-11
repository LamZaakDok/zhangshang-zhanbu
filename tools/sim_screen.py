#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""用生成的 cnfont_generated.h 复刻固件的文字渲染, 输出 PNG 供核对。"""
import re, sys
from PIL import Image
sys.stdout.reconfigure(encoding="utf-8")

h = open("src/cnfont_generated.h", encoding="utf-8").read()
font = {}
for m in re.finditer(r"\{0x([0-9A-Fa-f]{4}),\{([^}]*)\}\}", h):
    cp = int(m.group(1), 16)
    bs = [int(b, 16) for b in m.group(2).split(",")]
    font[cp] = bs

W, H = 135, 240
img = Image.new("RGB", (W, H), (0, 0, 0))
px = img.load()

def blit(x, y, cp, color):
    bs = font.get(cp)
    if not bs:
        return
    for r in range(16):
        bits = (bs[r * 2] << 8) | bs[r * 2 + 1]
        for c in range(16):
            if bits & (0x8000 >> c):
                xx, yy = x + c, y + r
                if 0 <= xx < W and 0 <= yy < H:
                    px[xx, yy] = color

def draw_text(x, y, s, color=(255, 255, 255)):
    cx = x
    for ch in s:
        cp = ord(ch)
        blit(cx, y, cp, color)
        cx += 8 if cp < 0x80 else 16
    return cx

def draw_wrapped(x, y, maxw, s, color=(255, 255, 255), lh=18):
    cx, cy, lw = x, y, 0
    for ch in s:
        cp = ord(ch)
        if ch == "\n":
            cx, cy, lw = x, cy + lh, 0
            continue
        cw = 8 if cp < 0x80 else 16
        if lw + cw > maxw and lw > 0:
            cx, cy, lw = x, cy + lh, 0
        blit(cx, cy, cp, color)
        cx += cw
        lw += cw

about = ("掌上占卜 版本7\n抽签/塔罗/六爻\nESP32-S3 / StickS3\n"
         "操作 蓝键=确认/继续, 右键=下移\n长按蓝键=返回\n仅供娱乐")
draw_text(4, 2, "关于", (0, 255, 255))
draw_wrapped(6, 26, 123, about)
img.resize((W * 3, H * 3), Image.NEAREST).save("tools/sim_about.png")
print("wrote tools/sim_about.png; font glyphs:", len(font))
