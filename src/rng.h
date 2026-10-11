#pragma once
#include <stdint.h>
#include <esp_random.h>

// ESP32 硬件真随机源。起卦/抽签/洗牌都走这里,
// 按键与摇一摇的时序不参与熵,保证结果不可预测。
static inline uint32_t rnd_u32() { return esp_random(); }

// [0, n) 的均匀随机数
static inline int rnd_range(int n) {
    if (n <= 1) return 0;
    return (int)(esp_random() % (uint32_t)n);
}
