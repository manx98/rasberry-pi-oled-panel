//
// Created by Fir on 2024/3/21 021.
//

#include <cmath>
#include "astra_logo.h"
#include "astra/utils/clocker.h"
#include "astra/ui/item/item.h"
#include <astra/config/config.h>

namespace astra {

    /**
     *   *     *      *         *
     *    *     powered by *    *
     *       * Astra UI *
     *  *       *     *     *
     *     *  *     *           *
     */
    void drawLogo(uint16_t _time) {
        Clocker render_clocker;
        Clocker timeout_clocker;
        bool isInit = false;
        while (true) {
            render_clocker.waitUntil(getUIConfig().perFrameMills);
            static std::vector<float> yStars;
            static std::vector<float> yStarsTrg;
            static std::vector<float> xStars;
            render_clocker.next();
            static std::string text = "astra UI";
            static std::string copyRight = "powered by";

            HAL::setFont(getUIConfig().logoTitleFont);
            static float xTitle = (HAL::getSystemConfig().screenWeight - HAL::getFontWidth(text)) / 2;
            HAL::setFont(getUIConfig().logoCopyRightFont);
            static float xCopyRight = (HAL::getSystemConfig().screenWeight - HAL::getFontWidth(copyRight)) / 2;
            HAL::setFont(getUIConfig().mainFont);

            //都是左上角坐标
            static float yTitle = 0 - getUIConfig().logoTextHeight - 1;
            static float yCopyRight = 0 - getUIConfig().logoCopyRightHeight - 1;
            static float yTitleTrg = 0;
            static float yCopyRightTrg = 0;

            static float xBackGround = 0;
            static float yBackGround = 0 - HAL::getSystemConfig().screenHeight - 1;

            static float yBackGroundTrg = 0;

            if (timeout_clocker.currentDuration() < _time) {
                yBackGroundTrg = 0;
                //星星坐标初始化 注意星星的坐标代表其中心点的位置 注意仅初始化一次
                if (!isInit) {
                    yStars.clear();
                    yStarsTrg.clear();
                    xStars.clear();

                    for (unsigned char i = 0; i < getUIConfig().logoStarNum; i++) {
                        //设置随机种子
                        srand(HAL::getRandomSeed() * 7);

                        yStars.push_back(0 - getUIConfig().logoStarLength - 1);

                        //产生从1到screenHeight的随机数
                        yStarsTrg.push_back(1 + rand() % (uint16_t) (HAL::getSystemConfig().screenHeight -
                                                                     2 * getUIConfig().logoStarLength - 2 + 1));
                        //产生从1到screenWeight的随机数
                        xStars.push_back(1 + rand() % (uint16_t) (HAL::getSystemConfig().screenWeight -
                                                                  2 * getUIConfig().logoStarLength - 2 + 1));
                    }
                    isInit = true;
                }
                yTitleTrg = HAL::getSystemConfig().screenHeight / 2 - getUIConfig().logoTextHeight / 2;  //居中
                yCopyRightTrg = yTitleTrg - getUIConfig().logoCopyRightHeight - 4;
            } else {
                //一起退场
                yBackGroundTrg = 0 - HAL::getSystemConfig().screenHeight - 1;
                yStarsTrg.assign(getUIConfig().logoStarNum, 0 - getUIConfig().logoStarLength - 1);
                yTitleTrg = 0 - getUIConfig().logoTextHeight - 1;
                yCopyRightTrg = 0 - getUIConfig().logoCopyRightHeight - 1;
            }

            //遮罩先进场 然后是星星 然后是文字
            //一起退场
            HAL::canvasClear();

            HAL::setDrawType(0);
            //遮罩
            HAL::drawBox(xBackGround, yBackGround, HAL::getSystemConfig().screenWeight,
                         HAL::getSystemConfig().screenHeight);
            Animation::move(yBackGround, yBackGroundTrg, getUIConfig().logoAnimationSpeed, render_clocker);
            HAL::setDrawType(1);
            HAL::drawHLine(0, yBackGround + HAL::getSystemConfig().screenHeight, HAL::getSystemConfig().screenWeight);

            //画星星
            for (unsigned char i = 0; i < getUIConfig().logoStarNum; i++) {
                HAL::drawHLine(xStars[i] - getUIConfig().logoStarLength - 1, yStars[i], getUIConfig().logoStarLength);
                HAL::drawHLine(xStars[i] + 2, yStars[i], getUIConfig().logoStarLength);
                HAL::drawVLine(xStars[i], yStars[i] - getUIConfig().logoStarLength - 1, getUIConfig().logoStarLength);
                HAL::drawVLine(xStars[i], yStars[i] + 2, getUIConfig().logoStarLength);

                Animation::move(yStars[i], yStarsTrg[i], getUIConfig().logoAnimationSpeed, render_clocker);
            }

            HAL::setFont(getUIConfig().logoTitleFont);
            HAL::drawEnglish(xTitle, yTitle + getUIConfig().logoTextHeight, text);
            HAL::setFont(getUIConfig().logoCopyRightFont);
            HAL::drawEnglish(xCopyRight, yCopyRight + getUIConfig().logoCopyRightHeight, copyRight);
            Animation::move(yTitle, yTitleTrg, getUIConfig().logoAnimationSpeed, render_clocker);
            Animation::move(yCopyRight, yCopyRightTrg, getUIConfig().logoAnimationSpeed, render_clocker);

            HAL::canvasUpdate();

            if (timeout_clocker.currentDuration() >= _time && yBackGround == - HAL::getSystemConfig().screenHeight - 1) {
                return;
            }
        }
    }
}