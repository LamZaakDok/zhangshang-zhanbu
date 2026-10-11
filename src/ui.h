#pragma once
#include <M5Unified.h>

// 统一显示层: 中文由 cnfont_generated.h 提供(16x16 点阵),
// 未生成时自动回退到 M5 内置字体(中文不显示)。
namespace ui {

constexpr int SCR_W = 135;
constexpr int SCR_H = 240;
constexpr int LINE_H = 18;

void begin();
void clear(uint16_t color = TFT_BLACK);

// 返回绘制结束后的 x 坐标
int  drawText(int x, int y, const char* s, uint16_t color = TFT_WHITE);
int  textWidth(const char* s);

// 按宽度自动换行, 返回下一行 y。文本中的 '\n' 或 '|' 视为强制换行。
int  drawWrapped(int x, int y, int maxW, const char* s, uint16_t color = TFT_WHITE);

// 分页文本: 按宽度和每页行数排版, 返回总页数; drawPaged 绘制第 page 页(从 0)
int  pagedCount(int maxW, const char* s);
int  drawPaged(int x, int y, int maxW, int maxLines, const char* s, int page,
               uint16_t color = TFT_WHITE);

// 居中绘制一行
void center(int y, const char* s, uint16_t color = TFT_WHITE);

// 顶部标题栏 / 底部操作提示
void statusBar(const char* title);
void statusBarCenter(const char* title);   // 标题居中
void prompt(const char* left, const char* right);

// 列表项(统一菜单样式): 选中用底色+青色左条, 不用白框
void menuRow(int y, int h, const char* label, bool selected);

// 底部滚动字幕(靠 millis 自动滚动)
void marquee(int y, const char* s, uint16_t color = 0x7BEF, int gap = 30);

// 居中显示; 一行放不下则自动横向滚动(需调用方周期重绘)
int  centerScroll(int y, const char* s, uint16_t color = TFT_WHITE);

// 电量指示(填充条 + 百分比, 无边框), 右对齐到 xRight
void battery(int xRight, int y, uint16_t color = TFT_WHITE);

// 绘制六爻卦象: lines 自下而上 6 位(1=阳 0=阴), moving 可为 nullptr
void drawHexagram(int x, int y, int lineW, int lineGap,
                  const int* lines, const int* moving);

} // namespace ui
