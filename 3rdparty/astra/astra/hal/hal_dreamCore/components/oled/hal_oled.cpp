//
// Created by Fir on 2024/2/11.
//
#include <cmath>
#include "astra/hal/hal_dreamCore/hal_dreamCore.h"
#include <u8g2.h>
#include <u8x8.h>

void *HALDreamCore::_getCanvasBuffer() {
  return u8g2_GetBufferPtr(&canvasBuffer);
}

unsigned char HALDreamCore::_getBufferTileHeight() {
  return u8g2_GetBufferTileHeight(&canvasBuffer);
}

unsigned char HALDreamCore::_getBufferTileWidth() {
  return u8g2_GetBufferTileWidth(&canvasBuffer);
}

void HALDreamCore::_canvasUpdate() {
  u8g2_SendBuffer(&canvasBuffer);
}

void HALDreamCore::_canvasClear() {
  u8g2_ClearBuffer(&canvasBuffer);
}

void HALDreamCore::_setFont(const unsigned char *_font) {
  u8g2_SetFontMode(&canvasBuffer, 1); /*字体模式选择*/
  u8g2_SetFontDirection(&canvasBuffer, 0); /*字体方向选择*/
  u8g2_SetFont(&canvasBuffer, _font);
}

int HALDreamCore::_getFontWidth(const std::string &_text) {
  return u8g2_GetUTF8Width(&canvasBuffer, _text.c_str());
}

unsigned char HALDreamCore::_getFontHeight() {
  return u8g2_GetMaxCharHeight(&canvasBuffer);
}

void HALDreamCore::_setDrawType(unsigned char _type) {
  u8g2_SetDrawColor(&canvasBuffer, _type);
}

void HALDreamCore::_drawPixel(float _x, float _y) {
  u8g2_DrawPixel(&canvasBuffer, (int16_t)std::round(_x), (int16_t)std::round(_y));
}

void HALDreamCore::_drawEnglish(float _x, float _y, const std::string &_text) {
  u8g2_DrawStr(&canvasBuffer, (int16_t)std::round(_x), (int16_t)std::round(_y), _text.c_str());
}

void HALDreamCore::_drawChinese(float _x, float _y, const std::string &_text) {
  u8g2_DrawUTF8(&canvasBuffer, (int16_t)std::round(_x), (int16_t)std::round(_y), _text.c_str());
}

void HALDreamCore::_drawVDottedLine(float _x, float _y, float _h) {
  for (unsigned char i = 0; i < (unsigned char)std::round(_h); i++) {
    if (i % 8 == 0 | (i - 1) % 8 == 0 | (i - 2) % 8 == 0) continue;
    u8g2_DrawPixel(&canvasBuffer, (int16_t)std::round(_x), (int16_t)std::round(_y) + i);
  }
}

void HALDreamCore::_drawHDottedLine(float _x, float _y, float _l) {
  for (unsigned char i = 0; i < _l; i++) {
    if (i % 8 == 0 | (i - 1) % 8 == 0 | (i - 2) % 8 == 0) continue;
    u8g2_DrawPixel(&canvasBuffer, (int16_t)std::round(_x) + i, (int16_t)std::round(_y));
  }
}

void HALDreamCore::_drawVLine(float _x, float _y, float _h) {
  u8g2_DrawVLine(&canvasBuffer, (int16_t)std::round(_x), (int16_t)std::round(_y), (int16_t)std::round(_h));
}

void HALDreamCore::_drawHLine(float _x, float _y, float _l) {
  u8g2_DrawHLine(&canvasBuffer, (int16_t)std::round(_x), (int16_t)std::round(_y), (int16_t)std::round(_l));
}

void HALDreamCore::_drawBMP(float _x, float _y, float _w, float _h, const unsigned char *_bitMap) {
  u8g2_DrawXBMP(&canvasBuffer, (int16_t)std::round(_x), (int16_t)std::round(_y), (int16_t)std::round(_w), (int16_t)std::round(_h), _bitMap);
}

void HALDreamCore::_drawBox(float _x, float _y, float _w, float _h) {
  u8g2_DrawBox(&canvasBuffer, (int16_t)std::round(_x), (int16_t)std::round(_y), (int16_t)std::round(_w), (int16_t)std::round(_h));
}

void HALDreamCore::_drawRBox(float _x, float _y, float _w, float _h, float _r) {
  u8g2_DrawRBox(&canvasBuffer, (int16_t)std::round(_x), (int16_t)std::round(_y), (int16_t)std::round(_w), (int16_t)std::round(_h), (int16_t)std::round(_r));
}

void HALDreamCore::_drawFrame(float _x, float _y, float _w, float _h) {
  u8g2_DrawFrame(&canvasBuffer, (int16_t)std::round(_x), (int16_t)std::round(_y), (int16_t)std::round(_w), (int16_t)std::round(_h));
}

void HALDreamCore::_drawRFrame(float _x, float _y, float _w, float _h, float _r) {
  u8g2_DrawRFrame(&canvasBuffer, (int16_t)std::round(_x), (int16_t)std::round(_y), (int16_t)std::round(_w), (int16_t)std::round(_h), (int16_t)std::round(_r));
}