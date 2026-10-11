#include "jiaobei.h"
#include "ui.h"
#include "rng.h"
#include "motion.h"
#include "input.h"

// 掷杯筊 · 三掷。一次问事连掷三个筊, 逐个揭晓。
//   每个筊: 正(平面朝上) / 反(凸面朝上)
//   一正一反 -> 聖筊(允) ; 两正 -> 笑筊 ; 两反 -> 陰筊(不允)
// 三掷皆聖 -> 神明明确允准。

static void drawJiao(int cx, int cy, int r, bool yang) {
    M5.Display.fillCircle(cx, cy, r, 0xE7E0);
    int off = yang ? -(int)(r * 0.72f) : (int)(r * 0.72f);
    M5.Display.fillCircle(cx, cy + off, r, TFT_BLACK);   // 挖月牙, 表示朝向
}

static const char* resName(int r) { return r == 0 ? "聖筊" : r == 1 ? "笑筊" : "陰筊"; }
static uint16_t  resColor(int r) { return r == 0 ? TFT_GREEN : r == 1 ? TFT_YELLOW : TFT_RED; }

// 揭晓 count 个结果(count 1..3)
static void drawResult(const int* a, const int* b, const int* res, int count) {
    ui::clear();
    ui::statusBar("掷杯筊");
    for (int k = 0; k < count; k++) {
        int y = 30 + k * 46;
        char n[4];
        snprintf(n, sizeof(n), "%d", k + 1);
        ui::drawText(4, y + 6, n, 0x9CD3);
        drawJiao(32, y + 14, 12, a[k] == 0);
        drawJiao(62, y + 14, 12, b[k] == 0);
        ui::drawText(88, y + 6, resName(res[k]), resColor(res[k]));
    }
    if (count == 3) {
        int c0 = 0, c1 = 0, c2 = 0;
        for (int k = 0; k < 3; k++) { if (res[k] == 0) c0++; else if (res[k] == 1) c1++; else c2++; }
        char s[32];
        snprintf(s, sizeof(s), "聖%d 笑%d 陰%d", c0, c1, c2);
        ui::center(168, s, TFT_WHITE);
        const char* v = (c0 == 3) ? "三聖筊 神明明确允准" : "未得三聖筊 神意未明 可再掷";
        ui::drawWrapped(8, 190, ui::SCR_W - 16, v, c0 == 3 ? TFT_GREEN : 0xE7E0);
    }
    ui::prompt(count == 3 ? "蓝 再掷" : "", "长按返回");
}

void jiaobeiRun() {
    while (true) {
        bool back = false;
        ui::clear();
        ui::statusBar("掷杯筊");
        ui::centerScroll(58, "默念所求之事", TFT_YELLOW);
        ui::prompt("蓝 掷筊", "长按返回");
        {
            const char* line = "摇一摇 或 蓝键 投掷三次";
            auto drawLine = [line]() {
                M5.Display.fillRect(0, 90, ui::SCR_W, 16, TFT_BLACK);   // 只清这一条
                ui::centerScroll(90, line, TFT_WHITE);
            };
            drawLine();
            uint32_t last = millis();
            while (true) {
                In e = inputPoll();
                if (e == In::BACK) { back = true; break; }
                if (e == In::OK || e == In::DOWN) break;
                if (shakeDetected()) break;
                if (millis() - last > 40) { drawLine(); last = millis(); }
                delay(5);
            }
        }
        if (back) return;

        int a[3], b[3], res[3];
        for (int k = 0; k < 3; k++) {
            for (int i = 0; i < 6; i++) {          // 掷动动画
                ui::clear();
                ui::statusBar("掷杯筊");
                drawJiao(45, 96, 18, (i % 2) == 0);
                drawJiao(90, 96, 18, ((i / 2) % 2) == 0);
                delay(50);
            }
            a[k] = rnd_range(2); b[k] = rnd_range(2);        // 0=正 1=反
            res[k] = (a[k] != b[k]) ? 0 : (a[k] == 0 ? 1 : 2); // 0聖 1笑 2陰
            drawResult(a, b, res, k + 1);
            delay(700);
        }
        drawResult(a, b, res, 3);

        In e = inputWait();
        if (e == In::BACK) return;
        // OK/DOWN -> 再问一次
    }
}
