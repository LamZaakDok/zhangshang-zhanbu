#pragma once
#include <M5Unified.h>

// 按键模型:
//   蓝色键 短按 = 确认 / 继续   (BtnA)
//   右侧按钮   = 下移          (BtnB)
//   蓝色键 长按 = 返回
// 设备上左右相反则交换 okA / downB。

enum class In : uint8_t { NONE, OK, DOWN, BACK };

static inline In inputPoll() {
    static bool     held = false;
    static uint32_t t0 = 0;
    static bool     suppress = false;   // 长按后释放不再触发确认

    M5.update();
    bool okDown = M5.BtnA.isPressed();
    bool downEdge = M5.BtnB.wasPressed();
    const uint32_t HOLD_MS = 600;

    if (okDown) {
        if (!held) { held = true; t0 = millis(); }
        else if (!suppress && (millis() - t0) >= HOLD_MS) {
            suppress = true;
            return In::BACK;                 // 长按 -> 返回
        }
    } else if (held) {
        held = false;
        uint32_t dur = millis() - t0;
        if (suppress) suppress = false;       // 长按后的释放, 忽略
        else if (dur < HOLD_MS) return In::OK; // 短按 -> 确认
    }

    if (downEdge) return In::DOWN;
    return In::NONE;
}

// 阻塞等待一次有效输入
static inline In inputWait() {
    while (true) {
        In e = inputPoll();
        if (e != In::NONE) return e;
        delay(15);
    }
}
