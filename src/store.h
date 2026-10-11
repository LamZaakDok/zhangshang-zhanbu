#pragma once
#include <Arduino.h>
#include <SPIFFS.h>
#include <vector>

// 极简数据读取工具。CSV 一律以「半角逗号」分隔,
// 文案内部请使用全角标点(，。！),避免与分隔符冲突。

static inline bool storeRead(const char* path, String& out) {
    File f = SPIFFS.open(path, "r");
    if (!f) return false;
    out = f.readString();
    f.close();
    return true;
}

static inline std::vector<String> splitLine(const String& line) {
    std::vector<String> v;
    int start = 0;
    while (true) {
        int c = line.indexOf(',', start);
        if (c < 0) { v.push_back(line.substring(start)); break; }
        v.push_back(line.substring(start, c));
        start = c + 1;
    }
    return v;
}

// 逐行调用 fn(line)。自动跳过空行和以 # 开头的注释行。
template <typename F>
static void forEachLine(const String& data, F fn) {
    int start = 0, n = data.length();
    while (start < n) {
        int nl = data.indexOf('\n', start);
        String line = (nl < 0) ? data.substring(start) : data.substring(start, nl);
        start = (nl < 0) ? n : nl + 1;
        line.trim();
        if (line.isEmpty() || line[0] == '#') continue;
        fn(line);
    }
}
