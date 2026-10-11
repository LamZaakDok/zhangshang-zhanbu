#include <SPIFFS.h>   // 早于 M5Unified.h: 生成 fs::SPIFFSFS 的 DataWrapper 特化
#include "tarot.h"
#include "ui.h"
#include "rng.h"
#include "store.h"
#include "input.h"
#include <vector>

// 塔罗。牌阵: 单张 / 三张 / 凯尔特十字。逆位 180° 旋转。
// 按键: 蓝键=确认/继续, 右键=下移, 长按蓝键=返回。

struct Card { int id; String name; String arcana; String upright; String reversed; };

static std::vector<Card> s_deck;
static M5Canvas* s_canvas = nullptr;

static const int BODY_X = 6;
static const int BODY_W = ui::SCR_W - 12;
static const int M_LINES = 9;

static bool loadDeck() {
    if (!s_deck.empty()) return true;
    String data;
    if (!storeRead("/tarot.csv", data)) return false;
    bool first = true;
    forEachLine(data, [&](const String& line) {
        if (first && line.startsWith("id,")) { first = false; return; }
        first = false;
        auto f = splitLine(line);
        if (f.size() < 5) return;
        Card c;
        c.id = f[0].toInt(); c.name = f[1]; c.arcana = f[2];
        c.upright = f[3]; c.reversed = f[4];
        s_deck.push_back(c);
    });
    return !s_deck.empty();
}

static void shuffle(std::vector<int>& v) {
    for (int i = (int)v.size() - 1; i > 0; i--) {
        int j = rnd_range(i + 1);
        int t = v[i]; v[i] = v[j]; v[j] = t;
    }
}

static M5Canvas* canvas() {
    if (!s_canvas) {
        s_canvas = new M5Canvas(&M5.Display);
        s_canvas->setColorDepth(16);
        s_canvas->createSprite(116, 200);
    }
    return s_canvas;
}

static void decode(int imgId) {
    M5Canvas* c = canvas();
    c->fillSprite(TFT_BLACK);
    char path[32];
    snprintf(path, sizeof(path), "/tarot/%02d.jpg", imgId);
    fs::FS& fs = SPIFFS;
    c->drawJpgFile(fs, path, 0, 0, 116, 200);
}

static void showCard(int imgId, bool reversed, int x, int y, int w, int h) {
    decode(imgId);
    float zx = (float)w / 116.0f, zy = (float)h / 200.0f;
    float zoom = (zx < zy) ? zx : zy;
    float cx = x + w / 2.0f, cy = y + h / 2.0f;
    canvas()->pushRotateZoom(&M5.Display, (int)cx, (int)cy,
                             reversed ? 180.0f : 0.0f, zoom, zoom);
    float aw = 116 * zoom, ah = 200 * zoom;
    M5.Display.drawRect((int)(cx - aw / 2) - 1, (int)(cy - ah / 2) - 1,
                        (int)aw + 2, (int)ah + 2, 0x7BEF);
}

// 等待继续(确认/下移); 返回 false 表示用户返回
static bool waitGo() {
    while (true) {
        In e = inputWait();
        if (e == In::BACK) return false;
        if (e == In::OK || e == In::DOWN) return true;
    }
}

// 逐张查看牌面与牌义。返回 false = 用户返回。
static bool viewCards(const int* picks, const bool* revs, const char* const* pos, int count) {
    int ci = 0, sub = 0;
    while (ci < count) {
        const Card& c = s_deck[picks[ci]];
        bool r = revs[ci];
        const char* meaning = r ? c.reversed.c_str() : c.upright.c_str();
        int mPages = (ui::pagedCount(BODY_W, meaning) + M_LINES - 1) / M_LINES;
        if (mPages < 1) mPages = 1;

        ui::clear();
        char title[40];
        snprintf(title, sizeof(title), "塔罗 %s", pos ? pos[ci] : "");
        ui::statusBar(title);
        if (sub == 0) {
            showCard(c.id, r, 17, 22, 101, 174);
            char b[40];
            snprintf(b, sizeof(b), "%s %s", c.name.c_str(), r ? "逆位" : "正位");
            ui::center(202, b, r ? TFT_RED : TFT_GREEN);
        } else {
            ui::center(20, c.name.c_str(), TFT_MAGENTA);
            ui::drawPaged(BODY_X, 42, BODY_W, M_LINES, meaning, sub - 1, TFT_WHITE);
        }
        ui::prompt(sub == 0 ? "蓝 牌义" : (sub < mPages ? "蓝 继续" : "蓝 下一张"),
                   "长按返回");
        if (!waitGo()) return false;
        if (sub == 0) sub = 1;
        else if (sub < mPages) sub++;
        else { ci++; sub = 0; }
    }
    return true;
}

static void overview3(const int* picks, const bool* revs) {
    ui::clear();
    ui::statusBar("塔罗 三张");
    const char* POS[3] = { "过去", "现在", "未来" };
    for (int i = 0; i < 3; i++) {
        int cx = 30 + i * 38;
        showCard(picks[i], revs[i], cx - 17, 52, 34, 60);
        int w = ui::textWidth(POS[i]);
        ui::drawText(cx - w / 2, 118, POS[i], TFT_CYAN);
        const char* o = revs[i] ? "逆" : "正";
        int wo = ui::textWidth(o);
        ui::drawText(cx - wo / 2, 138, o, revs[i] ? TFT_RED : TFT_GREEN);
    }
    ui::prompt("蓝 详情", "长按返回");
}

static void celticBox(int x, int y, int w, int h, int n, bool rev) {
    uint16_t col = rev ? TFT_RED : TFT_GREEN;
    M5.Display.fillRect(x, y, w, h, 0x0841);
    M5.Display.drawRect(x, y, w, h, col);
    char b[8];
    snprintf(b, sizeof(b), "%d %s", n, rev ? "逆" : "正");
    ui::drawText(x + (w - ui::textWidth(b)) / 2, y + (h - 16) / 2, b,
                 rev ? TFT_RED : TFT_GREEN);
}

static void overviewCeltic(const bool* revs) {
    ui::clear();
    ui::statusBar("塔罗 凯尔特十字");
    const int bw = 58, bh = 30;
    for (int i = 0; i < 10; i++) {
        int col = i % 2, row = i / 2;
        celticBox(6 + col * 65, 28 + row * 34, bw, bh, i + 1, revs[i]);
    }
    ui::prompt("蓝 详情", "长按返回");
}

static void runSpread(int kind) {
    std::vector<int> order(s_deck.size());
    for (size_t i = 0; i < order.size(); i++) order[i] = (int)i;

    const char* POS1[1] = { "单张" };
    const char* POS3[3] = { "过去", "现在", "未来" };
    const char* POS10[10] = { "现况", "阻碍", "根基", "过去", "目标",
                              "未来", "自我", "环境", "心绪", "结果" };
    int count = (kind == 0) ? 1 : (kind == 1) ? 3 : 10;

    while (true) {
        ui::clear();
        ui::statusBar("塔罗 抽牌");
        ui::center(56, kind == 0 ? "单张" : kind == 1 ? "过去 现在 未来" : "凯尔特十字 十张",
                   TFT_YELLOW);
        ui::center(88, "静心冥想你的问题", 0xE7E0);
        ui::center(112, "按蓝键抽牌", TFT_WHITE);
        ui::prompt("蓝 抽牌", "长按返回");
        if (!waitGo()) return;

        int picks[10]; bool revs[10];
        shuffle(order);
        for (int i = 0; i < count; i++) { picks[i] = order[i]; revs[i] = rnd_range(2) == 1; }

        const char* const* pos;
        if (kind == 0) pos = POS1;
        else if (kind == 1) { pos = POS3; overview3(picks, revs); if (!waitGo()) return; }
        else { pos = POS10; overviewCeltic(revs); if (!waitGo()) return; }

        if (!viewCards(picks, revs, pos, count)) return;
    }
}

void tarotRun() {
    if (!loadDeck()) {
        ui::clear();
        ui::statusBar("塔罗");
        ui::drawWrapped(6, 40, ui::SCR_W - 12,
                        "未找到 /tarot.csv\n请先执行\npio run -t uploadfs", TFT_RED);
        ui::prompt("", "长按返回");
        while (inputWait() != In::BACK) {}
        return;
    }

    int sel = 0;
    const int N = 4;
    const char* items[4] = { "单张", "三张 过去 现在 未来", "凯尔特十字 十张", "返回" };
    while (true) {
        ui::clear();
        ui::statusBar("塔罗 选牌阵");
        for (int i = 0; i < N; i++)
            ui::menuRow(44 + i * 44, 34, items[i], i == sel);
        ui::prompt("蓝 确认", "右 下移");

        In e = In::NONE;
        uint32_t last = millis();
        while (true) {
            e = inputPoll();
            if (e != In::NONE) break;
            if (millis() - last > 40) {          // 长标签滚动需要重绘
                for (int i = 0; i < N; i++)
                    ui::menuRow(44 + i * 44, 34, items[i], i == sel);
                last = millis();
            }
            delay(5);
        }
        if (e == In::DOWN) { sel = (sel + 1) % N; continue; }
        if (e == In::BACK) return;
        if (sel < 3) runSpread(sel);
        else return;
    }
}
