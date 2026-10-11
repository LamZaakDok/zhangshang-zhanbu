#include <SPIFFS.h>   // 早于 M5Unified.h: 生成 fs::SPIFFSFS 的 DataWrapper 特化
#include <M5Unified.h>
#include "ui.h"
#include "input.h"
#include "draw_lots.h"
#include "tarot.h"
#include "liuyao.h"
#include "jiaobei.h"

// 主菜单 + 状态机。按键: 蓝键=确认/继续, 右键=下移, 长按蓝键=返回。

static const char* MENU[] = { "抽签", "塔罗", "六爻", "掷杯筊", "关于" };
static const int MENU_N = sizeof(MENU) / sizeof(MENU[0]);

static void drawMenu(int sel) {
    ui::clear();
    ui::statusBarCenter("掌上占卜");
    for (int i = 0; i < MENU_N; i++)
        ui::menuRow(28 + i * 38, 34, MENU[i], i == sel);
}

// 底部: 滚动提示 + 右下角电量(纯文字)
static void drawBottom() {
    const int y = ui::SCR_H - 18;
    M5.Display.fillRect(0, y, ui::SCR_W, 18, TFT_BLACK);
    M5.Display.drawFastHLine(0, y, ui::SCR_W, 0x4208);
    M5.Display.setClipRect(0, y, ui::SCR_W - 70, 18);
    ui::marquee(y + 3, "确认 蓝色按键    下移 右侧键", 0x7BEF, 30);
    M5.Display.clearClipRect();
    ui::battery(ui::SCR_W - 4, y + 3);
}

static void about() {
    ui::clear();
    ui::statusBar("关于");
    ui::center(24, "掌上占卜", TFT_WHITE);
    ui::drawText(12, 58, "操作提示", 0x9CD3);
    ui::drawText(12, 92, "蓝键 确认、继续", TFT_WHITE);
    ui::drawText(12, 126, "右键 下移", TFT_WHITE);
    ui::drawText(12, 160, "长按蓝键 返回", TFT_WHITE);
    ui::drawText(12, 194, "仅供娱乐", 0x9CD3);
    ui::prompt("", "长按返回");
    while (inputWait() != In::BACK) {}
}

// 启动: 第一页只图片, 第二页只有"祝君好运"(黄色行楷图); 均黑底
static void splash() {
    bool have = SPIFFS.exists("/splash.jpg");
    fs::FS& fs = SPIFFS;

    ui::clear();
    if (have) M5.Display.drawJpgFile(fs, "/splash.jpg", 0, 0, ui::SCR_W, ui::SCR_H);
    uint32_t t = millis();
    while (millis() - t < 1700) {
        if (inputPoll() != In::NONE) return;
        delay(30);
    }

    ui::clear();
    if (SPIFFS.exists("/bless.jpg"))
        M5.Display.drawJpgFile(fs, "/bless.jpg", 0, 0, ui::SCR_W, ui::SCR_H);
    t = millis();
    while (millis() - t < 1700) {
        if (inputPoll() != In::NONE) return;
        delay(30);
    }
}

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    Serial.begin(115200);
    if (!SPIFFS.begin(true)) {
        Serial.println("[fs] SPIFFS mount failed");
    }
    ui::begin();
    splash();
    Serial.printf("[fs] total=%u used=%u\n",
                  SPIFFS.totalBytes(), SPIFFS.usedBytes());
}

void loop() {
    static int sel = 0;
    drawMenu(sel);
    drawBottom();
    uint32_t last = millis();
    while (true) {
        In e = inputPoll();
        if (e == In::DOWN) {
            sel = (sel + 1) % MENU_N;
            drawMenu(sel);
            drawBottom();
        } else if (e == In::OK) {
            switch (sel) {
                case 0: drawLotsRun();  break;
                case 1: tarotRun();     break;
                case 2: liuyaoRun();    break;
                case 3: jiaobeiRun();   break;
                case 4: about();        break;
            }
            drawMenu(sel);
            drawBottom();
        }
        if (millis() - last > 40) { drawBottom(); last = millis(); }
        delay(5);
    }
}
