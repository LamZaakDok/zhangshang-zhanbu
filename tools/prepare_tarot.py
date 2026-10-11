#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
塔罗牌面图预处理
==================
把源图统一裁成 116x200 的牌面, 存为 JPEG 到 data/tarot/。

牌面建议使用公有领域素材: 1909 年 Rider–White–Smith 牌组
(Wikimedia Commons 有整套高清扫描, 可直接商用)。

命名: 放 78 张源图到 tools/tarot_src/, 按文件名排序即对应
  00..21 大阿卡纳(愚者..世界)
  22..35 权杖 首牌,二..十,侍从,骑士,王后,国王
  36..49 圣杯  50..63 宝剑  64..77 星币
也可以用 00.jpg ~ 77.jpg 精确指定序号。

依赖: pip install pillow
用法: python tools/prepare_tarot.py
"""
import glob
import os
import sys

try:
    from PIL import Image
except ImportError:
    sys.exit("需要 Pillow: pip install pillow")

SRC = "tools/tarot_src"
OUT = "data/tarot"
CW, CH = 116, 200          # 与固件 drawCardFace 的贴图尺寸一致
QUALITY = 78


def main():
    os.makedirs(OUT, exist_ok=True)
    srcs = []
    for ext in ("*.jpg", "*.jpeg", "*.png", "*.webp", "*.bmp"):
        srcs += glob.glob(os.path.join(SRC, ext))
    srcs.sort()

    if not srcs:
        sys.exit("未在 %s 找到源图。请先放入 78 张塔罗牌面。" % SRC)
    if len(srcs) != 78:
        print("警告: 找到 %d 张, 期望 78 张" % len(srcs))

    total = 0
    for i, path in enumerate(srcs):
        im = Image.open(path).convert("RGB")
        im.thumbnail((CW, CH), Image.LANCZOS)
        canvas = Image.new("RGB", (CW, CH), (0, 0, 0))
        canvas.paste(im, ((CW - im.width) // 2, (CH - im.height) // 2))
        dst = os.path.join(OUT, "%02d.jpg" % i)
        canvas.save(dst, "JPEG", quality=QUALITY, optimize=True)
        total += os.path.getsize(dst)

    print("完成: %d 张, 共 %.2f MB" % (len(srcs), total / 1024 / 1024))
    print("如需更省空间可把 QUALITY 调到 60~70。")


if __name__ == "__main__":
    main()
