#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""生成 README 用的界面示意图(逻辑分辨率 135x240, 放大输出)。"""
import os
from PIL import Image, ImageDraw, ImageFont

FONT = os.path.join("tools", "raw", "unifont.otf")
if not os.path.exists(FONT):
    raise SystemExit("先运行 tools/gen_font.py 下载 Unifont")
f = ImageFont.truetype(FONT, 16)

BLACK, CYAN, GRAY, SEL = (0, 0, 0), (0, 255, 255), (180, 190, 200), (8, 12, 40)
WHITE, YEL, RED, GRN, BAMBOO = (235, 235, 235), (255, 220, 60), (240, 70, 70), (90, 230, 90), (231, 224, 160)
LINE = (40, 40, 48)


def screen():
    return Image.new("RGB", (135, 240), BLACK)


def W(d, s):
    return int(d.textlength(s, font=f))


def txt(d, x, y, s, c):
    d.text((x, y), s, font=f, fill=c)


def ctr(d, y, s, c):
    d.text(((135 - W(d, s)) // 2, y), s, font=f, fill=c)


def bar(d, title):
    d.rectangle([0, 0, 135, 18], fill=BLACK)
    ctr(d, 2, title, CYAN)
    d.line([0, 18, 135, 18], fill=LINE)


def prompt(d, left, right):
    d.line([0, 222, 135, 222], fill=LINE)
    if left:
        txt(d, 4, 225, left, (130, 130, 140))
    if right:
        txt(d, 135 - 4 - W(d, right), 225, right, (130, 130, 140))


def battery(d, s="88%"):
    txt(d, 135 - 4 - W(d, s), 225, s, WHITE)


def crescent(d, cx, cy, r, yang):
    d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=BAMBOO)
    off = -int(r * 0.72) if yang else int(r * 0.72)
    d.ellipse([cx - r, cy + off - r, cx + r, cy + off + r], fill=BLACK)


# ---- 各屏 ----
def sc_menu():
    im = screen(); d = ImageDraw.Draw(im)
    bar(d, "掌上占卜")
    for i, it in enumerate(["抽签", "塔罗", "六爻", "掷杯筊", "关于"]):
        y = 28 + i * 38
        if i == 0:
            d.rectangle([0, y, 135, y + 34], fill=SEL)
            d.rectangle([0, y, 3, y + 34], fill=CYAN)
        txt(d, 14, y + 9, it, CYAN if i == 0 else GRAY)
    prompt(d, "确认 蓝键", ""); battery(d)
    return im


def sc_zhuang():
    im = screen(); d = ImageDraw.Draw(im)
    bar(d, "六爻 本卦装卦")
    ctr(d, 22, "甲日 乾宫金", CYAN)
    rows = [("青龙", "父 壬戌土 世", (0, 200, 0)), ("朱雀", "兄 壬申金", WHITE),
            ("勾陈", "官 壬午火", WHITE), ("螣蛇", "父 甲辰土 应", CYAN),
            ("白虎", "财 甲寅木", WHITE), ("玄武", "子 甲子水", WHITE)]
    for i, (sp, m, mc) in enumerate(rows):
        y = 46 + i * 18
        txt(d, 6, y, sp, (200, 200, 200)); txt(d, 40, y, m, mc)
    prompt(d, "蓝 继续", "长按返回")
    return im


def sc_celtic():
    im = screen(); d = ImageDraw.Draw(im)
    bar(d, "塔罗 凯尔特十字")
    for i in range(10):
        col, row = i % 2, i // 2
        x, y = 6 + col * 65, 28 + row * 34
        rev = (i % 3 == 0)
        c = RED if rev else GRN
        d.rectangle([x, y, x + 58, y + 30], fill=SEL, outline=c)
        s = "%d %s" % (i + 1, "逆" if rev else "正")
        txt(d, x + (58 - W(d, s)) // 2, y + 7, s, c)
    prompt(d, "蓝 详情", "长按返回")
    return im


def sc_jiaobei():
    im = screen(); d = ImageDraw.Draw(im)
    bar(d, "掷杯筊")
    res = [("聖筊", GRN, True, False), ("聖筊", GRN, False, True), ("笑筊", YEL, True, True)]
    for k, (nm, c, ya, yb) in enumerate(res):
        y = 30 + k * 46
        txt(d, 4, y + 6, str(k + 1), (150, 150, 150))
        crescent(d, 32, y + 14, 12, ya); crescent(d, 62, y + 14, 12, yb)
        txt(d, 88, y + 6, nm, c)
    ctr(d, 168, "聖2 笑1 陰0", WHITE)
    ctr(d, 190, "未得三聖筊 可再掷", (231, 224, 160))
    prompt(d, "蓝 再掷", "长按返回")
    return im


PANELS = [("主界面", sc_menu()), ("六爻 装卦", sc_zhuang()),
          ("塔罗 凯尔特十字", sc_celtic()), ("掷杯筊", sc_jiaobei())]

S = 2
pad, cap = 16, 24
pw, ph = 135 * S, 240 * S
total_w = pad + len(PANELS) * (pw + pad)
total_h = cap + ph + pad
sheet = Image.new("RGB", (total_w, total_h), (12, 12, 16))
sd = ImageDraw.Draw(sheet)
for i, (cap_txt, im) in enumerate(PANELS):
    x = pad + i * (pw + pad)
    big = im.resize((pw, ph), Image.NEAREST)
    sheet.paste(big, (x, cap))
    sd.text((x, 4), cap_txt, font=f, fill=(200, 200, 200))

os.makedirs("docs", exist_ok=True)
sheet.save("docs/screens.png")
PANELS[0][1].resize((135 * 4, 240 * 4), Image.NEAREST).save("docs/preview.png")
print("wrote docs/screens.png, docs/preview.png")
