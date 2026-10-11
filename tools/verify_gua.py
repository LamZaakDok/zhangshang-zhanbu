#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
六爻装卦校验
============
用与固件相同的表, 打印若干卦的纳甲装卦结果, 供人工核对。
"""
import sys
sys.stdout.reconfigure(encoding="utf-8")

# 八宫: 乾 坤 震 巽 坎 离 艮 兑
PALACE_NAME = ["乾", "坤", "震", "巽", "坎", "离", "艮", "兑"]
PALACE_WX = [0, 4, 2, 2, 1, 3, 4, 0]           # 金水土? -> 金 土 木 木 水 火 土 金
WX = ["金", "水", "木", "火", "土"]
TRIG = {"乾": 7, "兑": 3, "离": 5, "震": 1, "巽": 6, "坎": 2, "艮": 4, "坤": 0}

PALACE_GUA = [
    ["乾", "姤", "遯", "否", "观", "剥", "晋", "大有"],
    ["坤", "复", "临", "泰", "大壮", "夬", "需", "比"],
    ["震", "豫", "解", "恒", "升", "井", "大过", "随"],
    ["巽", "小畜", "家人", "益", "无妄", "噬嗑", "颐", "蛊"],
    ["坎", "节", "屯", "既济", "革", "丰", "明夷", "师"],
    ["离", "旅", "鼎", "未济", "蒙", "涣", "讼", "同人"],
    ["艮", "贲", "大畜", "损", "睽", "履", "中孚", "渐"],
    ["兑", "困", "萃", "咸", "蹇", "谦", "小过", "归妹"],
]
SHI_POS = [6, 1, 2, 3, 4, 5, 4, 3]

ZHI_NAME = ["子", "丑", "寅", "卯", "辰", "巳", "午", "未", "申", "酉", "戌", "亥"]
ZHI_WX = [1, 4, 2, 2, 4, 3, 3, 4, 0, 0, 4, 1]
GAN_IN = ["乙", "庚", "戊", "丁", "丙", "己", "辛", "甲"]
GAN_OUT = ["癸", "庚", "戊", "丁", "丙", "己", "辛", "壬"]
TRIG_ZHI = [
    [7, 5, 3, 1, 11, 9],
    [0, 2, 4, 6, 8, 10],
    [2, 4, 6, 8, 10, 0],
    [5, 3, 1, 11, 9, 7],
    [4, 6, 8, 10, 0, 2],
    [3, 1, 11, 9, 7, 5],
    [1, 11, 9, 7, 5, 3],
    [0, 2, 4, 6, 8, 10],
]
LIUQIN = ["父", "兄", "子", "财", "官"]


def sheng(i): return (i + 1) % 5
def ke(i): return (i + 2) % 5


def qin(gong, yao):
    if yao == gong: return 1
    if sheng(yao) == gong: return 0
    if sheng(gong) == yao: return 2
    if ke(yao) == gong: return 4
    if ke(gong) == yao: return 3
    return 1


def key_of(upper, lower):
    return TRIG[lower] | (TRIG[upper] << 3)


def palace_of(name, gua64_name2key):
    for p in range(8):
        for k in range(8):
            if PALACE_GUA[p][k] == name:
                return p, SHI_POS[k]
    return -1, 0


def show(name, upper, lower, gua64):
    key = key_of(upper, lower)
    p, shi = palace_of(name, gua64)
    gong = PALACE_WX[p]
    lt, ut = key & 7, (key >> 3) & 7
    print("%s  (上%s下%s, key=%d)  %s宫%s  世%d 应%d"
          % (name, upper, lower, key, PALACE_NAME[p], WX[gong], shi, (shi + 2) % 6 + 1))
    for i in range(5, -1, -1):
        tri = lt if i < 3 else ut
        gan = GAN_IN[tri] if i < 3 else GAN_OUT[tri]
        z = TRIG_ZHI[tri][i]
        wx = ZHI_WX[z]
        sy = "世" if i == shi - 1 else ("应" if i == (shi + 2) % 6 else "")
        print("   %d爻 %s %s%s%s %s" % (i + 1, LIUQIN[qin(gong, wx)], gan, ZHI_NAME[z], WX[wx], sy))
    print()


if __name__ == "__main__":
    # 已知正确结果见代码注释, 运行比对
    show("乾", "乾", "乾", None)   # 乾宫金 世6应3
    show("坤", "坤", "坤", None)   # 坤宫土 世6应3
    show("坎", "坎", "坎", None)   # 坎宫水 世6应3
    show("晋", "离", "坤", None)   # 乾宫游魂 世4应1
    show("既济", "坎", "离", None) # 坎宫三世 世3应6
