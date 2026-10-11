#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
签文导入器
==========
把 tools/raw/ 下的公开数据集 JSON 转成固件用的 CSV。

数据来源(均为公开仓库):
  灵隐寺·观音灵签  yanxinyu777-beep/guanyin-lingqian-100  (guanyin-lots.json)
  黄大仙祠·灵签    wickes1/wong-tai-sin                  (sticks.json)
  浅草寺·观音签    Tamshen/senso-ji-stick-data           (data.zh.json)

用法: python tools/import_lots.py
输出: data/lots_lingyin.csv / lots_wongtaisin.csv / lots_sensoji.csv
列: no,level,title,verse,explain  (多行用 | 分隔)
"""
import json
import os
import sys
from collections import Counter

sys.stdout.reconfigure(encoding="utf-8")

RAW = os.path.join("tools", "raw")


def clean(s):
    if s is None:
        return ""
    s = str(s).replace("\r", "").replace("\n", "|").replace(",", "，")
    while "||" in s:
        s = s.replace("||", "|")
    return s.strip("| ").strip()


def write_csv(path, comment, rows):
    with open(path, "w", encoding="utf-8") as f:
        f.write(comment)
        f.write("no,level,title,verse,explain\n")
        for r in rows:
            f.write(",".join(clean(x) for x in r) + "\n")
    print("wrote %-26s %3d rows" % (path, len(rows)))


def load(name):
    with open(os.path.join(RAW, name), encoding="utf-8") as f:
        return json.load(f)


def do_lingyin():
    g = load("guanyin.json")
    rows = [[x["number"], x.get("level", ""), x.get("story", ""),
             "|".join(x.get("poem", [])), x.get("interpretation", "")] for x in g]
    write_csv("data/lots_lingyin.csv",
              "# 灵隐寺 · 观音灵签(一百签)\n"
              "# 来源: github.com/yanxinyu777-beep/guanyin-lingqian-100\n",
              rows)
    print("  levels:", dict(Counter(x.get("level", "") for x in g)))


def do_wongtaisin():
    w = load("wongtaisin.json")
    rows = []
    for x in w:
        poem = "|".join(str(x.get("poem", "")).split())
        explain = "%s|%s" % (x.get("interpretation", ""), x.get("allusion", ""))
        rows.append([x["number"], x.get("fortune", ""), x.get("title", ""), poem, explain])
    write_csv("data/lots_wongtaisin.csv",
              "# 黄大仙祠 · 灵签(一百签)\n"
              "# 来源: github.com/wickes1/wong-tai-sin\n",
              rows)
    print("  levels:", dict(Counter(x.get("fortune", "") for x in w)))


def do_sensoji():
    s = load("sensoji.zh.json")["qcs"]
    rows = []
    for i, item in enumerate(s[1:], start=1):
        level = item[0] if len(item) > 0 else ""
        poem = item[1] if len(item) > 1 else ""
        parts = [item[k] for k in (2, 3, 4) if len(item) > k and item[k]]
        rows.append([i, level, "", poem, "|".join(parts)])
    write_csv("data/lots_sensoji.csv",
              "# 浅草寺 · 观音签(一百签)\n"
              "# 来源: github.com/Tamshen/senso-ji-stick-data\n",
              rows)
    print("  levels:", dict(Counter(r[1] for r in rows)))


if __name__ == "__main__":
    for fn in (do_lingyin, do_wongtaisin, do_sensoji):
        fn()
    print("完成。记得重跑 tools/gen_font.py 并 pio run -t uploadfs")
