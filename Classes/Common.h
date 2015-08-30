//
//  Common.h
//
//  Created by shamaton.
//
//

#ifndef __Common__
#define __Common__

#include <iostream>

#include "cocos2d.h"

//#define NEND_ENABLE
#define NEND_TEST 0 // 仮IDでテスト表示する

#define PLAY_BGM 1
#define PLAY_SE  1

#define COUNT_OF(array) (sizeof(array) / sizeof(array[0]))

enum E_SE_TYPE {
    SE_TYPE_NORMAL,
    SE_TYPE_DRUM,
    SE_TYPE_PIANO,
    NR_SE_TYPE
};

static const int FRAME = 60;

static const cocos2d::Size GAME_RESOLUTION = cocos2d::Size(480.0f, 800.0f);

static const float NEND_BANNER_HEIGHT = 100.0f;

class Common {
public:
    static cocos2d::Vec2 get_layer_position(cocos2d::Size layer_size, cocos2d::Vec2 orig);
	static int sec2frame(float sec);
    static std::vector<std::string> split(const std::string &str, char sep);
    
	static void nend_icon_enable();
	static void nend_icon_disable();
	static void nend_banner_enable();
	static void nend_banner_disable();
	static void nend_interstitial_enable();
	static void nend_interstitial_disable();
};

#endif /* defined(__Common__) */
