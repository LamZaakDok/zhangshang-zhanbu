#pragma once
#include <M5Unified.h>
#include <math.h>

// 摇动检测。静置时加速度模长约 1g, 明显超过阈值即视为摇动。
// 供六爻起卦与抽签共用。
static inline bool shakeDetected(float threshold = 2.0f) {
    float ax, ay, az;
    if (!M5.Imu.getAccelData(&ax, &ay, &az)) return false;
    return sqrtf(ax * ax + ay * ay + az * az) > threshold;
}
