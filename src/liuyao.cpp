#include "liuyao.h"
#include "ui.h"
#include "rng.h"
#include "store.h"
#include "motion.h"
#include "input.h"

// 六爻 三枚铜钱法 + 纳甲装卦 + 用神断语。
// 按键: 蓝键=确认/继续, 右键=下移, 长按蓝键=返回。

static String s_name[64];
static String s_note[64];
static bool   s_valid[64];
static int    s_palace[64];
static int    s_shi[64];

static const char* WX_NAME[5] = { "金", "水", "木", "火", "土" };
static inline int shengIdx(int i) { return (i + 1) % 5; }
static inline int keIdx(int i)    { return (i + 2) % 5; }

static int trigramBits(const String& t) {
    if (t == "乾") return 7; if (t == "兑") return 3; if (t == "离") return 5;
    if (t == "震") return 1; if (t == "巽") return 6; if (t == "坎") return 2;
    if (t == "艮") return 4; if (t == "坤") return 0;
    return -1;
}

static const char* PALACE_NAME[8] = { "乾", "坤", "震", "巽", "坎", "离", "艮", "兑" };
static const int   PALACE_WX[8]   = { 0, 4, 2, 2, 1, 3, 4, 0 };
static const char* PALACE_GUA[8][8] = {
    { "乾",  "姤", "遯",   "否", "观",   "剥", "晋",   "大有" },
    { "坤",  "复", "临",   "泰", "大壮", "夬", "需",   "比"   },
    { "震",  "豫", "解",   "恒", "升",   "井", "大过", "随"   },
    { "巽",  "小畜", "家人", "益", "无妄", "噬嗑", "颐", "蛊" },
    { "坎",  "节", "屯",   "既济", "革", "丰",   "明夷", "师" },
    { "离",  "旅", "鼎",   "未济", "蒙", "涣",   "讼",   "同人" },
    { "艮",  "贲", "大畜", "损", "睽",   "履", "中孚", "渐"   },
    { "兑",  "困", "萃",   "咸", "蹇",   "谦", "小过", "归妹" },
};
static const int SHI_POS[8] = { 6, 1, 2, 3, 4, 5, 4, 3 };

static const char* ZHI_NAME[12] = { "子", "丑", "寅", "卯", "辰", "巳", "午", "未", "申", "酉", "戌", "亥" };
static const int   ZHI_WX[12]   = { 1, 4, 2, 2, 4, 3, 3, 4, 0, 0, 4, 1 };
static const char* TRIG_GAN_IN[8]  = { "乙", "庚", "戊", "丁", "丙", "己", "辛", "甲" };
static const char* TRIG_GAN_OUT[8] = { "癸", "庚", "戊", "丁", "丙", "己", "辛", "壬" };
static const int   TRIG_ZHI[8][6] = {
    { 7, 5, 3, 1, 11, 9 }, { 0, 2, 4, 6, 8, 10 }, { 2, 4, 6, 8, 10, 0 },
    { 5, 3, 1, 11, 9, 7 }, { 4, 6, 8, 10, 0, 2 },  { 3, 1, 11, 9, 7, 5 },
    { 1, 11, 9, 7, 5, 3 }, { 0, 2, 4, 6, 8, 10 },
};

static const char* LIUQIN[5] = { "父", "兄", "子", "财", "官" };
static const char* FULL_QIN[5] = { "父母", "兄弟", "子孙", "妻财", "官鬼" };

static const char* GAN[10] = { "甲", "乙", "丙", "丁", "戊", "己", "庚", "辛", "壬", "癸" };
static const char* SPIRITS[6] = { "青龙", "朱雀", "勾陈", "螣蛇", "白虎", "玄武" };
static const int SPIRIT_START[10] = { 0, 0, 1, 1, 2, 3, 4, 4, 5, 5 };
static const uint16_t SPIRIT_COLOR[6] = { 0x07E0, 0xF800, 0xFD20, 0xF81F, TFT_WHITE, 0x05BF };

static const char* Q_TYPE[6] = { "求财", "功名", "婚姻", "疾病", "出行", "子女" };
static const int   Q_YONG[6] = { 3, 4, 3, 4, 0, 2 };
static const char* Q_TIP[6]  = {
    "妻财为用", "官鬼为用", "男占妻财 女占官鬼", "官鬼为病 子孙为药",
    "父母为用", "子孙为用",
};

static int qinIndex(int gongWx, int yaoWx) {
    if (yaoWx == gongWx) return 1;
    if (shengIdx(yaoWx) == gongWx) return 0;
    if (shengIdx(gongWx) == yaoWx) return 2;
    if (keIdx(yaoWx) == gongWx) return 4;
    if (keIdx(gongWx) == yaoWx) return 3;
    return 1;
}

static bool loadGua() {
    static bool loaded = false;
    if (loaded) return true;
    for (int i = 0; i < 64; i++) { s_palace[i] = -1; s_shi[i] = 0; }
    String data;
    if (!storeRead("/gua64.csv", data)) return false;
    bool first = true;
    forEachLine(data, [&](const String& line) {
        if (first && line.startsWith("id,")) { first = false; return; }
        first = false;
        auto f = splitLine(line);
        if (f.size() < 5) return;
        int up = trigramBits(f[2]), lo = trigramBits(f[3]);
        if (up < 0 || lo < 0) return;
        int key = lo | (up << 3);
        if (key < 0 || key > 63) return;
        s_name[key] = f[1]; s_note[key] = f[4]; s_valid[key] = true;
    });
    auto nameToKey = [](const char* nm) -> int {
        for (int k = 0; k < 64; k++) if (s_valid[k] && s_name[k] == nm) return k;
        return -1;
    };
    for (int p = 0; p < 8; p++)
        for (int k = 0; k < 8; k++) {
            int key = nameToKey(PALACE_GUA[p][k]);
            if (key >= 0) { s_palace[key] = p; s_shi[key] = SHI_POS[k]; }
        }
    loaded = true;
    return true;
}

static const char* guaName(int key) { return s_valid[key & 63] ? s_name[key & 63].c_str() : "?"; }
static const char* guaNote(int key) { return s_valid[key & 63] ? s_note[key & 63].c_str() : ""; }

struct Yao { int qin; int wx; String gz; int sy; };

static void zhuangGua(int key, Yao out[6]) {
    int p = (s_palace[key] >= 0) ? s_palace[key] : 0;
    int gongWx = PALACE_WX[p];
    int lt = key & 7, ut = (key >> 3) & 7;
    for (int i = 0; i < 6; i++) {
        int tri = (i < 3) ? lt : ut;
        const char* gan = (i < 3) ? TRIG_GAN_IN[tri] : TRIG_GAN_OUT[tri];
        int z = TRIG_ZHI[tri][i];
        out[i].wx = ZHI_WX[z];
        out[i].qin = qinIndex(gongWx, out[i].wx);
        out[i].gz = String(gan) + ZHI_NAME[z];
        out[i].sy = 0;
    }
    int shi = s_shi[key] ? s_shi[key] : 1;
    int ying = ((shi + 2) % 6) + 1;
    out[shi - 1].sy = 1;
    out[ying - 1].sy = 2;
}

// 等待继续(确认/下移); false = 用户返回
static bool waitGo() {
    while (true) {
        In e = inputWait();
        if (e == In::BACK) return false;
        if (e == In::OK || e == In::DOWN) return true;
    }
}

static void drawZhuangPage(int key, const char* title, int gan, const int* moving) {
    ui::clear();
    ui::statusBar(title);
    int p = (s_palace[key] >= 0) ? s_palace[key] : 0;
    char hdr[48];
    snprintf(hdr, sizeof(hdr), "%s日 %s宫%s", GAN[gan], PALACE_NAME[p], WX_NAME[PALACE_WX[p]]);
    ui::center(22, hdr, TFT_CYAN);
    Yao y[6];
    zhuangGua(key, y);
    int start = SPIRIT_START[gan];
    for (int row = 0; row < 6; row++) {
        int i = 5 - row;
        int ly = 46 + row * 18;
        bool mv = (moving && moving[i]);
        if (mv) M5.Display.fillRect(0, ly, 3, 14, TFT_RED);
        ui::drawText(6, ly, SPIRITS[(start + i) % 6], SPIRIT_COLOR[(start + i) % 6]);
        String s = LIUQIN[y[i].qin];
        s += " "; s += y[i].gz; s += WX_NAME[y[i].wx];
        if (y[i].sy == 1) s += " 世"; else if (y[i].sy == 2) s += " 应";
        uint16_t col = mv ? TFT_RED : (y[i].sy == 1) ? 0xFD20 : (y[i].sy == 2) ? TFT_CYAN : TFT_WHITE;
        ui::drawText(40, ly, s.c_str(), col);
    }
}

// 择日干; 返回 -1 = 用户返回
static int chooseGan() {
    static int sel = 0;
    while (true) {
        ui::clear();
        ui::statusBar("六爻 择日干");
        ui::center(48, "今日天干 六神由此起", TFT_YELLOW);
        ui::center(84, GAN[sel], TFT_WHITE);
        char b[32];
        snprintf(b, sizeof(b), "初爻起%s", SPIRITS[SPIRIT_START[sel]]);
        ui::center(112, b, TFT_CYAN);
        ui::prompt("蓝 确认", "右 换");
        In e = inputWait();
        if (e == In::BACK) return -1;
        if (e == In::OK) return sel;
        sel = (sel + 1) % 10;
    }
}

// 择问事; 返回 -1 = 用户返回
static int chooseQuestion() {
    int sel = 0;
    while (true) {
        ui::clear();
        ui::statusBar("六爻 问何事");
        ui::center(30, "所问何事 以定用神", TFT_YELLOW);
        for (int i = 0; i < 6; i++)
            ui::menuRow(50 + i * 24, 22, Q_TYPE[i], i == sel);
        ui::center(206, Q_TIP[sel], 0x7BEF);
        ui::prompt("蓝 确认", "右 下移");
        In e = inputWait();
        if (e == In::BACK) return -1;
        if (e == In::OK) return sel;
        sel = (sel + 1) % 6;
    }
}

static String buildDuan(int key, const int* moving, int vkey, int q) {
    int yong = Q_YONG[q];
    Yao y[6];
    zhuangGua(key, y);
    String r = String("问") + Q_TYPE[q] + " 用神" + FULL_QIN[yong] + "(" + Q_TIP[q] + ")|";
    int pos[6], n = 0;
    for (int i = 0; i < 6; i++) if (y[i].qin == yong) pos[n++] = i;
    if (n == 0) { r += "用神不上卦, 事体未明, 宜另择时机再问。"; return r; }

    int shi = s_shi[key] ? s_shi[key] - 1 : 0;
    bool chishi = false;
    for (int k = 0; k < n; k++) if (pos[k] == shi) chishi = true;
    r += "用神在 ";
    for (int k = 0; k < n; k++) { r += String(pos[k] + 1); r += "爻 "; }
    r += "|";
    if (chishi) r += "用神持世, 事在掌握, 吉象。|";

    int p0 = pos[0], ywx = y[p0].wx;
    Yao vy[6];
    zhuangGua(vkey, vy);
    if (moving && moving[p0]) {
        int vw = vy[p0].wx;
        r += "用神发动: 由"; r += WX_NAME[ywx]; r += "变"; r += WX_NAME[vw]; r += ", ";
        if (shengIdx(vw) == ywx) r += "变爻生用神, 得助有力。";
        else if (keIdx(vw) == ywx) r += "变爻克用神, 恐有阻碍。";
        else if (shengIdx(ywx) == vw) r += "用神生变爻, 泄气费力。";
        else if (keIdx(ywx) == vw) r += "用神克变爻, 尚可自制。";
        else r += "与变爻比和, 平顺。";
        r += "|";
    } else {
        r += "用神安静, 事体平稳。|";
    }

    if (!chishi) {
        int swx = y[shi].wx;
        r += "世属"; r += WX_NAME[swx]; r += ", ";
        if (shengIdx(ywx) == swx) r += "用神生世, 得助为吉。";
        else if (shengIdx(swx) == ywx) r += "世生用神, 主动付出方成。";
        else if (keIdx(ywx) == swx) r += "用神克世, 事来就我, 有压力。";
        else if (keIdx(swx) == ywx) r += "世克用神, 求之费力争竞。";
        else r += "世与用神比和, 顺遂。";
    }
    return r;
}

void liuyaoRun() {
    if (!loadGua()) {
        ui::clear();
        ui::statusBar("六爻");
        ui::drawWrapped(6, 40, ui::SCR_W - 12,
                        "未找到 /gua64.csv\n请先执行\npio run -t uploadfs", TFT_RED);
        ui::prompt("", "长按返回");
        while (inputWait() != In::BACK) {}
        return;
    }
    int gan = chooseGan();
    if (gan < 0) return;

    int line[6], moving[6];
    for (int i = 0; i < 6; i++) {
        ui::clear();
        ui::statusBar("六爻 起卦");
        char b[48];
        snprintf(b, sizeof(b), "第 %d 爻", i + 1);
        ui::center(56, b, TFT_WHITE);
        ui::center(84, "摇一摇 / 蓝键掷币", TFT_YELLOW);
        int shown[6];
        for (int k = 0; k < 6; k++) shown[k] = (k < i) ? line[k] : 2;
        ui::drawHexagram(46, 108, 44, 5, shown, nullptr);
        ui::prompt("蓝 掷币", "长按返回");

        bool back = false;
        while (true) {
            In e = inputPoll();
            if (e == In::BACK) { back = true; break; }
            if (e == In::OK || e == In::DOWN) break;
            if (shakeDetected()) break;
            delay(10);
        }
        if (back) return;
        delay(350);

        int s = 0;
        for (int k = 0; k < 3; k++) s += rnd_range(2) ? 3 : 2;
        line[i]   = (s == 7 || s == 9) ? 1 : 0;
        moving[i] = (s == 6 || s == 9) ? 1 : 0;

        ui::clear();
        ui::statusBar("六爻 起卦");
        const char* t = (s == 9) ? "老阳 (动)" : (s == 6) ? "老阴 (动)"
                       : (s == 7) ? "少阳" : "少阴";
        ui::center(30, t, (s == 6 || s == 9) ? TFT_RED : TFT_GREEN);
        for (int k = 0; k < 6; k++) shown[k] = (k <= i) ? line[k] : 2;
        ui::drawHexagram(46, 70, 44, 5, shown, moving);
        ui::prompt("蓝 继续", "长按返回");
        if (!waitGo()) return;
    }

    int key = 0, vkey = 0, vline[6], hasMove = 0;
    for (int i = 0; i < 6; i++) {
        key |= line[i] << i;
        vline[i] = moving[i] ? (line[i] ? 0 : 1) : line[i];
        vkey |= vline[i] << i;
        hasMove += moving[i];
    }

    int q = chooseQuestion();
    if (q < 0) return;
    String duan = buildDuan(key, moving, vkey, q);

    int pages[6], np = 0;
    pages[np++] = 0;
    pages[np++] = 1;
    if (hasMove) { pages[np++] = 2; pages[np++] = 3; }
    pages[np++] = 4;
    pages[np++] = 5;

    int pi = 0;
    while (true) {
        int pg = pages[pi];
        if (pg == 0) {
            ui::clear();
            ui::statusBar("六爻 本卦");
            ui::drawHexagram(10, 44, 38, 6, line, moving);
            ui::drawText(58, 34, "本卦", TFT_CYAN);
            ui::drawText(58, 54, guaName(key), TFT_WHITE);
            ui::drawWrapped(58, 80, 72, guaNote(key), 0xE7E0);
        } else if (pg == 1) {
            drawZhuangPage(key, "六爻 本卦装卦", gan, moving);
        } else if (pg == 2) {
            ui::clear();
            ui::statusBar("六爻 变卦");
            ui::drawHexagram(10, 44, 38, 6, vline, nullptr);
            ui::drawText(58, 34, "变卦", TFT_CYAN);
            ui::drawText(58, 54, guaName(vkey), TFT_WHITE);
            ui::drawWrapped(58, 80, 72, guaNote(vkey), 0xE7E0);
        } else if (pg == 3) {
            drawZhuangPage(vkey, "六爻 变卦装卦", gan, nullptr);
        } else if (pg == 4) {
            ui::clear();
            ui::statusBar("六爻 动爻");
            String s = "动爻: ";
            if (!hasMove) s += "无";
            for (int i = 0; i < 6; i++) if (moving[i]) { s += String(i + 1); s += " "; }
            s += "|";
            s += hasMove ? "变卦由动爻阴阳互变而来。|动爻以左侧红标标示。"
                         : "静卦, 以本卦卦意断之。";
            ui::drawWrapped(8, 36, ui::SCR_W - 16, s.c_str(), TFT_WHITE);
        } else {
            ui::clear();
            ui::statusBar("六爻 用神断语");
            ui::drawPaged(6, 30, ui::SCR_W - 12, 10, duan.c_str(), 0, TFT_WHITE);
        }
        ui::prompt(pi < np - 1 ? "蓝 继续" : "蓝 再问", "长按返回");

        In e = inputWait();
        if (e == In::BACK) return;
        if (pi < np - 1) pi++;
        else return liuyaoRun();
    }
}
