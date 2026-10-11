#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
塔罗牌面自动抓取
================
从 mixvlad/TarotCards (MIT, 公有领域 1909 Rider–White–Smith) 的 720px
目录下载整套 78 张, 缩放为 116x200 存到 data/tarot/00.jpg .. 77.jpg。

- 原图缓存在 tools/raw/rw/ , 重跑只补缺失的(断点续传)。
- 按大阿卡纳 00..21 / 权杖22..35 / 圣杯36..49 / 宝剑50..63 / 星币64..77 编号。
- 已存在且非空的 data/tarot/NN.jpg 会跳过, 可反复运行。

用法: python tools/fetch_tarot.py
"""
import os
import ssl
import sys
import time
import urllib.request
from concurrent.futures import ThreadPoolExecutor, as_completed

from PIL import Image

BASE = "https://cdn.jsdelivr.net/gh/mixvlad/TarotCards@main/tarot/rider-waite/720px/"
CACHE = os.path.join("tools", "raw", "rw")
OUT = os.path.join("data", "tarot")
CW, CH, QUALITY = 116, 200, 78
WORKERS = 4

sys.stdout.reconfigure(encoding="utf-8")

CTX = ssl.create_default_context()
CTX.check_hostname = False
CTX.verify_mode = ssl.CERT_NONE   # 规避部分环境的证书吊销检查问题

MAJORS = ["00_Fool", "01_Magician", "02_High_Priestess", "03_Empress", "04_Emperor",
          "05_Hierophant", "06_Lovers", "07_Chariot", "08_Strength", "09_Hermit",
          "10_Wheel_of_Fortune", "11_Justice", "12_Hanged_Man", "13_Death",
          "14_Temperance", "15_Devil", "16_Tower", "17_Star", "18_Moon", "19_Sun",
          "20_Judgement", "21_World"]
SUITS = [("Wands", 22), ("Cups", 36), ("Swords", 50), ("Pents", 64)]


def build_map():
    m = {}
    for i, name in enumerate(MAJORS):
        m[i] = name
    for suit, base in SUITS:
        for n in range(1, 15):
            m[base + n - 1] = "%s%02d" % (suit, n)
    return m


def fetch(name):
    """下载原图到缓存, 返回本地路径; 失败返回 None。"""
    local = os.path.join(CACHE, name + ".jpg")
    if os.path.exists(local) and os.path.getsize(local) > 2000:
        return local
    url = BASE + name + ".jpg"
    for attempt in range(4):
        try:
            req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
            with urllib.request.urlopen(req, timeout=90, context=CTX) as r:
                data = r.read()
            if len(data) < 2000:
                raise IOError("too small: %d" % len(data))
            with open(local, "wb") as f:
                f.write(data)
            return local
        except Exception as e:
            if attempt == 3:
                print("  [fail] %s: %s" % (name, e))
                return None
            time.sleep(2 + attempt * 2)


def process(idx, name):
    dst = os.path.join(OUT, "%02d.jpg" % idx)
    if os.path.exists(dst) and os.path.getsize(dst) > 1000:
        return idx, True, "cached"
    src = fetch(name)
    if not src:
        return idx, False, name
    im = Image.open(src).convert("RGB")
    im.thumbnail((CW, CH), Image.LANCZOS)
    canvas = Image.new("RGB", (CW, CH), (0, 0, 0))
    canvas.paste(im, ((CW - im.width) // 2, (CH - im.height) // 2))
    canvas.save(dst, "JPEG", quality=QUALITY, optimize=True)
    return idx, True, name


def main():
    os.makedirs(CACHE, exist_ok=True)
    os.makedirs(OUT, exist_ok=True)
    m = build_map()
    items = sorted(m.items())
    ok, fail = 0, []
    with ThreadPoolExecutor(max_workers=WORKERS) as ex:
        futs = {ex.submit(process, i, n): i for i, n in items}
        for fut in as_completed(futs):
            idx, good, info = fut.result()
            if good:
                ok += 1
                if info != "cached":
                    print("  [%2d] %s" % (idx, info))
            else:
                fail.append((idx, info))
    print("完成: %d/%d" % (ok, len(items)))
    if fail:
        print("失败(可重跑本脚本补):", fail)
        sys.exit(1)


if __name__ == "__main__":
    main()
