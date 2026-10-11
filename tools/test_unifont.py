import sys
from PIL import Image, ImageDraw, ImageFont

sys.stdout.reconfigure(encoding="utf-8")

FONT = "tools/raw/unifont.otf"

def render(font, ch, size, thr):
    img = Image.new("L", (size, size), 0)
    ImageDraw.Draw(img).text((0, 0), ch, fill=255, font=font)
    px = img.load()
    rows = []
    for y in range(size):
        line = "".join("#" if px[x, y] > thr else "." for x in range(size))
        rows.append(line)
    return rows

for size in (16,):
    f = ImageFont.truetype(FONT, size)
    for ch in ["藏", "齑", "灵", "隐", "龙", "龟", "A", "3"]:
        print("=== %r @%d ===" % (ch, size))
        for r in render(f, ch, size, 128):
            print(r)

# 覆盖率: 统计素材与源码里每个字是否有笔画
import glob, os
chars = set()
for pat in ["data/**/*.csv", "src/**/*.cpp", "src/**/*.h"]:
    for fn in glob.glob(pat, recursive=True):
        if os.path.basename(fn) == "cnfont_generated.h":
            continue
        for c in open(fn, encoding="utf-8").read():
            if ord(c) >= 0x80:
                chars.add(c)
f = ImageFont.truetype(FONT, 16)
missing = []
for c in sorted(chars, key=ord):
    img = Image.new("L", (16, 16), 0)
    ImageDraw.Draw(img).text((0, 0), c, fill=255, font=f)
    if not any(px > 128 for px in img.getdata()):
        missing.append(c)
print("语料非ASCII字数:", len(chars), " 缺字:", len(missing), "".join(missing))
