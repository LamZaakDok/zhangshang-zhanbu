import json, sys
from collections import Counter
sys.stdout.reconfigure(encoding="utf-8")

s = json.load(open("tools/raw/sensoji.zh.json", encoding="utf-8"))["qcs"]
print("sensoji qcs len:", len(s))
for i in range(0, 4):
    print(i, json.dumps(s[i], ensure_ascii=False)[:400])

g = json.load(open("tools/raw/guanyin.json", encoding="utf-8"))
print("\nguanyin n=", len(g))
print("levels:", Counter(x.get("level") for x in g))
print("keys:", list(g[0].keys()))
for i in [0, 1, 2]:
    print(i, json.dumps(g[i], ensure_ascii=False)[:500])

w = json.load(open("tools/raw/wongtaisin.json", encoding="utf-8"))
print("\nwts n=", len(w))
print("fortune:", Counter(x.get("fortune") for x in w))
print("sample poem split:", w[0]["poem"].split())
