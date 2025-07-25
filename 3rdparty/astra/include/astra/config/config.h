//
// Created by Fir on 2024/1/25.
//

#pragma once
#ifndef ASTRA_CORE_SRC_SYSTEM_H_
#define ASTRA_CORE_SRC_SYSTEM_H_

#include <u8g2.h>

namespace astra {
/**
 * @brief config of astra ui. astra ui的配置结构体
 */
struct config {
  float tileAnimationSpeed = 70;
  float listAnimationSpeed = 70;
  float selectorYAnimationSpeed = 60;
  float selectorXAnimationSpeed = 70;
  float selectorWidthAnimationSpeed = 70;
  float selectorHeightAnimationSpeed = 55;
  float windowAnimationSpeed = 25;
  float sideBarAnimationSpeed = 15;
  float fadeAnimationSpeed = 100;
  float cameraAnimationSpeed = 80;
  float logoAnimationSpeed = 70;
  int perFrameMills = 6; // 每一帧渲染时间（低于硬件速度会导致丢帧）

  bool tileUnfold = true;
  bool listUnfold = true;

  bool menuLoop = true;

  bool lightMode = false;

  float listBarWeight = 5;
  float listTextMargin = 4; //文字边距
  float selectorRadius = 0.5f;
  float selectorMargin = 4; //选择框与文字左边距

  float tilePicWidth = 30;
  float tilePicHeight = 30;
  float tilePicMargin = 8;
  float tilePicTopMargin = 12; //图标上边距

  float tileTextBottomMargin = 4; //标题下边距(top: 52)

  float tileBarHeight = 2; //磁贴进度条高度

  float tileSelectBoxLineLength = 5;  //磁贴选择框线长
  float tileSelectBoxMargin = 3; //选择框边距
  float tileSelectBoxWidth = tileSelectBoxMargin * 2 + tilePicWidth; //选择框宽
  float tileSelectBoxHeight = tileSelectBoxMargin * 2 + tilePicHeight; //选择框高

  float popMargin = 4; //弹窗边距
  float popRadius = 2; //弹窗圆角半径
  float popSpeed = 90; //弹窗动画速度

  float logoStarLength = 2; //logo星星长度
  float logoTextHeight = 14; //logo文字高度
  float logoCopyRightHeight = 8; //logo文字高度
  unsigned char logoStarNum = 16; //logo星星数量

  const unsigned char *logoTitleFont = u8g2_font_open_iconic_arrow_1x_t;
  const unsigned char *logoCopyRightFont = u8g2_font_boutique_bitmap_7x7_t_gb2312;
  const unsigned char *mainFont = u8g2_font_boutique_bitmap_9x9_t_gb2312 ;

  float checkBoxWidth = 8;
  float checkBoxHeight = 8;
  float checkBoxTopMargin = 4; //与选项上边缘的距离
  float checkBoxRightMargin = 10; //与屏幕右边缘的距离
  float checkBoxRadius = 1;
  float progressInnerPadding = 1; // 进度条内距
  float progressHeight = 4; // 进度条高度
  float progressInfAnimationSpeed = 25; //无限滚动条速度
  float number_input_margin_x = 4; // 数字输入框左右边距
  float text_flow_speed = 30; // 文本滚动速度(ms/px)
  float text_flow_boundary_wait_time = 500; // 文本滚动边界等待时间ms
  float list_icon_size = 16; // 列表图标宽度
  float list_icon_margin = 3; // 列表图标边距
};

static config &getUIConfig() {
  static config astraConfig;
  return astraConfig;
}
}
#endif //ASTRA_CORE_SRC_SYSTEM_H_