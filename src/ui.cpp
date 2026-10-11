#include "ui.h"

#if __has_include("cnfont_generated.h")
  #include "cnfont_generated.h"
  #define HAS_CN_FONT 1
#endif

namespace ui {

void begin() {
    M5.Display.setRotation(0);
    M5.Display.setBrightness(180);
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextWrap(false);
}

void clear(uint16_t color) { M5.Display.fillScreen(color); }

// ---- UTF-8 解码 ------------------------------------------------------
static uint32_t utf8Next(const char*& p) {
    uint8_t c = (uint8_t)*p++;
    if (c < 0x80) return c;
    uint32_t cp = 0;
    int n = 0;
    if      ((c & 0xE0) == 0xC0) { cp = c & 0x1F; n = 1; }
    else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; n = 2; }
    else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; n = 3; }
    else return 0xFFFD;
    for (int i = 0; i < n; i++) {
        if ((*p & 0xC0) != 0x80) break;
        cp = (cp << 6) | ((uint8_t)*p++ & 0x3F);
    }
    return cp;
}

#ifdef HAS_CN_FONT
static const CnGlyph* findGlyph(uint32_t cp) {
    int lo = 0, hi = CN_FONT_COUNT - 1;
    while (lo <= hi) {
        int mid = (lo + hi) >> 1;
        uint32_t k = CN_FONT[mid].cp;
        if (k == cp) return &CN_FONT[mid];
        if (k < cp) lo = mid + 1; else hi = mid - 1;
    }
    return nullptr;
}

static void blitGlyph(int x, int y, const uint8_t* bmp, uint16_t color) {
    for (int r = 0; r < CN_FONT_H; r++) {
        uint16_t bits = ((uint16_t)bmp[r * 2] << 8) | bmp[r * 2 + 1];
        for (int c = 0; c < CN_FONT_W; c++) {
            if (bits & (0x8000 >> c)) M5.Display.drawPixel(x + c, y + r, color);
        }
    }
}
#endif

static int drawCharAt(int x, int y, uint32_t cp, uint16_t color) {
    if (cp == ' ' || cp == 0x3000) return (cp < 0x80) ? 6 : 16;   // 空格什么都不画
    if (cp < 0x80) {
        // ASCII 用 M5 内置 6x8 字体, 垂直居中到 16px 格
        M5.Display.drawChar((int32_t)x, (int32_t)(y + 5), (uint16_t)cp,
                            (uint16_t)color, (uint16_t)TFT_BLACK, 1.0f);
        return 6;
    }
#ifdef HAS_CN_FONT
    const CnGlyph* g = findGlyph(cp);
    if (g) blitGlyph(x, y, g->bmp, color);
    return CN_FONT_W;
#else
    return 16; // 未生成字库时占位, 空白
#endif
}

int textWidth(const char* s) {
    int w = 0;
    while (*s) {
        uint32_t cp = utf8Next(s);
        w += (cp < 0x80) ? 6 : 16;
    }
    return w;
}

int drawText(int x, int y, const char* s, uint16_t color) {
    int cx = x;
    while (*s) {
        uint32_t cp = utf8Next(s);
        cx += drawCharAt(cx, y, cp, color);
    }
    return cx;
}

int drawWrapped(int x, int y, int maxW, const char* s, uint16_t color) {
    int cx = x, cy = y, lineW = 0;
    while (*s) {
        uint32_t cp = utf8Next(s);
        if (cp == '\n' || cp == '|') { cx = x; cy += LINE_H; lineW = 0; continue; }
        int cw = (cp < 0x80) ? 6 : 16;
        if (lineW + cw > maxW && lineW > 0) { cx = x; cy += LINE_H; lineW = 0; }
        drawCharAt(cx, cy, cp, color);
        cx += cw;
        lineW += cw;
    }
    return cy + LINE_H;
}

// 取下一行: 返回该行结束处的指针, 并输出该行像素宽度。
static const char* nextLine(const char* p, int maxW, int& width) {
    int w = 0;
    while (*p) {
        if (*p == '\n' || *p == '|') { ++p; break; }
        const char* q = p;
        uint32_t cp = utf8Next(q);
        int cw = (cp < 0x80) ? 6 : 16;
        if (w + cw > maxW && w > 0) break;
        w += cw;
        p = q;
    }
    width = w;
    return p;
}

int pagedCount(int maxW, const char* s) {
    int n = 0;
    while (*s) { int w; const char* np = nextLine(s, maxW, w); n++; if (np == s) break; s = np; }
    return n;
}

int drawPaged(int x, int y, int maxW, int maxLines, const char* s, int page,
              uint16_t color) {
    if (maxLines < 1) maxLines = 1;
    int first = page * maxLines, last = first + maxLines;
    int line = 0;
    while (*s) {
        int w; const char* np = nextLine(s, maxW, w);
        if (line >= first && line < last) {
            int cx = x;
            for (const char* q = s; q < np;) {
                if (*q == '\n' || *q == '|') { ++q; continue; }
                uint32_t cp = utf8Next(q);
                cx += drawCharAt(cx, y + (line - first) * LINE_H, cp, color);
            }
        }
        line++;
        if (np == s) break;
        s = np;
    }
    return (line + maxLines - 1) / maxLines;   // 总页数
}

void center(int y, const char* s, uint16_t color) {
    int w = textWidth(s);
    drawText((SCR_W - w) / 2, y, s, color);
}

void statusBar(const char* title) {
    M5.Display.fillRect(0, 0, SCR_W, 18, TFT_BLACK);
    drawText(4, 2, title, TFT_CYAN);
    M5.Display.drawFastHLine(0, 18, SCR_W, 0x4208);
}

void statusBarCenter(const char* title) {
    M5.Display.fillRect(0, 0, SCR_W, 18, TFT_BLACK);
    int w = textWidth(title);
    drawText((SCR_W - w) / 2, 2, title, TFT_CYAN);
    M5.Display.drawFastHLine(0, 18, SCR_W, 0x4208);
}

void prompt(const char* left, const char* right) {
    M5.Display.drawFastHLine(0, SCR_H - 18, SCR_W, 0x4208);
    drawText(4, SCR_H - 15, left, 0x7BEF);
    if (right && *right) {
        int w = textWidth(right);
        drawText(SCR_W - w - 4, SCR_H - 15, right, 0x7BEF);
    }
}

void battery(int xRight, int y, uint16_t color) {
    (void)color;
    int lvl = M5.Power.getBatteryLevel();   // 0..100, 未知为负
    char b[8];
    if (lvl < 0) snprintf(b, sizeof(b), "--");
    else snprintf(b, sizeof(b), "%d%%", lvl);
    // 用点阵直接画, 不经过 drawChar(避免其填充背景)
    int w = 0;
    for (const char* p = b; *p; p++) w += 8;
    int x = xRight - w;
    for (const char* p = b; *p; p++) {
#ifdef HAS_CN_FONT
        const CnGlyph* g = findGlyph((uint32_t)(uint8_t)*p);
        if (g) blitGlyph(x, y - 1, g->bmp, TFT_WHITE);
#endif
        x += 8;
    }
}

void menuRow(int y, int h, const char* label, bool selected) {
    M5.Display.fillRect(0, y, SCR_W, h, selected ? 0x0841 : 0x0000);
    if (selected) M5.Display.fillRect(0, y, 3, h, TFT_CYAN);
    uint16_t col = selected ? TFT_CYAN : 0xBDF7;
    const int x = 14, w = SCR_W - x;
    int ty = y + (h - 16) / 2;
    if (textWidth(label) <= w) {
        drawText(x, ty, label, col);
    } else {                       // 一行放不开 -> 滚动播放
        M5.Display.setClipRect(x, y, w, h);
        marquee(ty, label, col, 24);
        M5.Display.clearClipRect();
    }
}

void marquee(int y, const char* s, uint16_t color, int gap) {
    int w = textWidth(s);
    int span = w + gap;
    if (span <= 0) return;
    int off = (int)((millis() / 25) % span);
    drawText(-off, y, s, color);
    drawText(-off + span, y, s, color);
}

int centerScroll(int y, const char* s, uint16_t color) {
    int w = textWidth(s);
    if (w <= SCR_W) {
        drawText((SCR_W - w) / 2, y, s, color);
    } else {
        marquee(y, s, color, 30);   // 放不下 -> 全屏滚动
    }
    return w;
}

void drawHexagram(int x, int y, int lineW, int lineGap,
                  const int* lines, const int* moving) {
    const int seg = (lineW - 8) / 2; // 阴爻两段之间的间隙
    for (int i = 0; i < 6; i++) {
        if (lines[i] == 2) continue;           // 2 = 尚未确定, 不绘制
        int ly = y + (5 - i) * (lineGap + 3); // 自下而上
        uint16_t col = (moving && moving[i]) ? TFT_RED : TFT_WHITE;
        if (lines[i]) {
            M5.Display.fillRect(x, ly, lineW, lineGap, col);
        } else {
            M5.Display.fillRect(x, ly, seg, lineGap, col);
            M5.Display.fillRect(x + lineW - seg, ly, seg, lineGap, col);
        }
    }
}

} // namespace ui
