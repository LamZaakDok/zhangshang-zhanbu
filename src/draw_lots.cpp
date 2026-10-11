#include "draw_lots.h"
#include "ui.h"
#include "rng.h"
#include "store.h"
#include "motion.h"
#include "input.h"
#include <Preferences.h>
#include <vector>

// 多签系抽签 + 历史记录(NVS)。
// 按键: 蓝键=确认/继续, 右键=下移, 长按蓝键=返回。

struct Lot {
    int no;
    String level;
    String title;
    String verse;    // 多行以 | 分隔
    String explain;  // 可长, 自动分页
};

struct LotSource { const char* name; const char* note; const char* file; };

static const LotSource SOURCES[] = {
    { "灵隐寺",   "观音灵签 一百签", "/lots_lingyin.csv" },
    { "黄大仙祠", "灵签 一百签",     "/lots_wongtaisin.csv" },
    { "浅草寺",   "观音签 一百签",   "/lots_sensoji.csv" },
};
static const int SOURCE_N = sizeof(SOURCES) / sizeof(SOURCES[0]);

static const int BODY_X = 6;
static const int BODY_W = ui::SCR_W - 12;
static const int EX_LINES = 9;

static Preferences s_prefs;

// ---- 历史记录 ----
struct Rec { int src; int no; String level; };

static std::vector<Rec> loadHist() {
    std::vector<Rec> v;
    s_prefs.begin("fortune", true);
    String s = s_prefs.getString("lots", "");
    s_prefs.end();
    int start = 0;
    while (start < (int)s.length()) {
        int nl = s.indexOf('\n', start);
        String line = (nl < 0) ? s.substring(start) : s.substring(start, nl);
        start = (nl < 0) ? s.length() : nl + 1;
        line.trim();
        if (line.isEmpty()) continue;
        auto f = splitLine(line);
        if (f.size() < 3) continue;
        Rec r; r.src = f[0].toInt(); r.no = f[1].toInt(); r.level = f[2];
        v.push_back(r);
    }
    return v;
}

static void pushHist(int src, int no, const String& level) {
    auto v = loadHist();
    v.insert(v.begin(), Rec{ src, no, level });
    const int MAX = 30;
    if ((int)v.size() > MAX) v.resize(MAX);
    String s;
    for (size_t i = 0; i < v.size(); i++) {
        if (i) s += '\n';
        s += String(v[i].src); s += ','; s += String(v[i].no); s += ','; s += v[i].level;
    }
    s_prefs.begin("fortune", false);
    s_prefs.putString("lots", s);
    s_prefs.end();
}

// ---- 数据 ----
static int loadInto(const char* path, std::vector<Lot>& out) {
    File f = SPIFFS.open(path, "r");
    if (!f) return -1;
    bool first = true;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.isEmpty() || line[0] == '#') continue;
        if (first && line.startsWith("no,")) { first = false; continue; }
        first = false;
        auto fl = splitLine(line);
        if (fl.size() < 5) continue;
        Lot l;
        l.no = fl[0].toInt(); l.level = fl[1]; l.title = fl[2];
        l.verse = fl[3]; l.explain = fl[4];
        out.push_back(l);
    }
    f.close();
    return (int)out.size();
}

static uint16_t levelColor(const String& l) {
    if (l.indexOf("凶") >= 0 || l.indexOf("下") >= 0) return 0x52AA;
    if (l.indexOf("大吉") >= 0 || l.indexOf("上") >= 0) return 0xFD20;
    if (l.indexOf("中") >= 0) return TFT_YELLOW;
    return TFT_GREEN;
}

static void showMissing(const char* path, int rows) {
    ui::clear();
    ui::statusBar("抽签");
    String msg;
    if (SPIFFS.exists(path)) {
        File f = SPIFFS.open(path, "r");
        long sz = f ? (long)f.size() : -1;
        if (f) f.close();
        msg = path; msg += "\n存在 字节 "; msg += String(sz);
        msg += " 解析行 "; msg += String(rows);
    } else {
        msg = "未找到 "; msg += path;
    }
    msg += "\n文件系统 ";
    msg += (SPIFFS.totalBytes() > 0 ? "已挂载" : "未挂载");
    ui::drawWrapped(BODY_X, 30, BODY_W, msg.c_str(), TFT_RED);
    ui::prompt("", "长按返回");
    while (inputWait() != In::BACK) {}
}

static void runSource(int idx) {
    const LotSource& src = SOURCES[idx];
    std::vector<Lot> lots;
    int nrows = loadInto(src.file, lots);
    if (nrows <= 0) { showMissing(src.file, nrows); return; }

    while (true) {
        ui::clear();
        ui::statusBar(src.name);
        ui::center(58, src.note, TFT_CYAN);
        ui::center(92, "摇动签筒 心诚则灵", TFT_YELLOW);
        ui::center(118, "摇一摇 蓝键抽签", TFT_WHITE);
        ui::prompt("蓝 抽签", "长按返回");
        bool back = false;
        while (true) {
            In e = inputPoll();
            if (e == In::BACK) { back = true; break; }
            if (e == In::OK || e == In::DOWN) break;
            if (shakeDetected()) break;
            delay(10);
        }
        if (back) return;

        for (int i = 0; i < 12; i++) {
            ui::clear();
            ui::statusBar(src.name);
            ui::center(90 + ((i % 2) ? 4 : -4), "签筒摇动中…", TFT_ORANGE);
            delay(60);
        }

        const Lot& lot = lots[rnd_range((int)lots.size())];
        pushHist(idx, lot.no, lot.level);

        int exPages = (ui::pagedCount(BODY_W, lot.explain.c_str()) + EX_LINES - 1) / EX_LINES;
        if (exPages < 1) exPages = 1;
        const int total = 1 + exPages + 1;

        int page = 0;
        bool again = false;
        while (!again) {
            ui::clear();
            ui::statusBar(src.name);
            char buf[48];
            snprintf(buf, sizeof(buf), "第 %d 签 %s", lot.no, lot.level.c_str());
            ui::center(22, buf, levelColor(lot.level));

            if (page == 0) {
                if (lot.title.length()) ui::center(48, lot.title.c_str(), TFT_WHITE);
                ui::drawWrapped(BODY_X, 76, BODY_W, lot.verse.c_str(), 0xE7E0);
            } else if (page <= exPages) {
                ui::drawPaged(BODY_X, 46, BODY_W, EX_LINES, lot.explain.c_str(), page - 1);
            } else {
                ui::drawWrapped(BODY_X, 60, BODY_W,
                    "签已抽毕。\n按蓝键重新抽签,\n长按蓝键返回。", TFT_WHITE);
            }
            ui::prompt(page == 0 ? "蓝 解曰" : (page < total - 1 ? "蓝 继续" : "蓝 重抽"),
                       "长按返回");

            In e = inputWait();
            if (e == In::BACK) return;
            if (page < total - 1) page++;
            else again = true;
        }
    }
}

static void showHistory() {
    auto v = loadHist();
    ui::clear();
    ui::statusBar("抽签 历史");
    if (v.empty()) {
        ui::center(100, "暂无抽签记录", 0x9CD3);
        ui::prompt("", "长按返回");
        while (inputWait() != In::BACK) {}
        return;
    }
    const int LINES = 10;
    int pages = ((int)v.size() + LINES - 1) / LINES;
    int page = 0;
    while (true) {
        ui::clear();
        ui::statusBar("抽签 历史");
        int y = 26;
        for (int i = page * LINES; i < (int)v.size() && i < (page + 1) * LINES; i++) {
            char b[40];
            snprintf(b, sizeof(b), "%s 第%d签", SOURCES[v[i].src].name, v[i].no);
            ui::drawText(6, y, b, TFT_WHITE);
            int lw = ui::textWidth(v[i].level.c_str());
            if (6 + ui::textWidth(b) + 6 + lw <= ui::SCR_W - 4)   // 放不下就不画, 避免重叠
                ui::drawText(ui::SCR_W - lw - 6, y, v[i].level.c_str(), levelColor(v[i].level));
            y += 18;
        }
        char pg[16];
        snprintf(pg, sizeof(pg), "%d/%d", page + 1, pages);
        ui::drawText(ui::SCR_W - ui::textWidth(pg) - 6, 2, pg, TFT_CYAN);
        ui::prompt("蓝 翻页", "长按返回");

        In e = inputWait();
        if (e == In::BACK) return;
        page = (page + 1) % pages;
    }
}

static void drawSourceMenu(int sel) {
    ui::clear();
    ui::statusBar("抽签 选择庙系");
    const int N = SOURCE_N + 2;
    const char* labels[5];
    for (int i = 0; i < SOURCE_N; i++) labels[i] = SOURCES[i].name;
    labels[SOURCE_N] = "历史签";
    labels[SOURCE_N + 1] = "返回";
    for (int i = 0; i < N; i++)
        ui::menuRow(28 + i * 38, 32, labels[i], i == sel);
    ui::prompt("蓝 确认", "右 下移");
}

void drawLotsRun() {
    const int N = SOURCE_N + 2;
    int sel = 0;
    while (true) {
        drawSourceMenu(sel);
        In e = inputWait();
        if (e == In::DOWN) { sel = (sel + 1) % N; continue; }
        if (e == In::BACK) return;
        // OK
        if (sel < SOURCE_N) runSource(sel);
        else if (sel == SOURCE_N) showHistory();
        else return;
    }
}
