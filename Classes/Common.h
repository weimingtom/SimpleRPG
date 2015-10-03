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
#include "Util/Util.h"

//#define NEND_ENABLE
#define NEND_TEST 0 // 仮IDでテスト表示する

#define PLAY_BGM 1
#define PLAY_SE  1

#define COUNT_OF(array) (sizeof(array) / sizeof(array[0]))

static const std::string RES_COMMON_DIR = "common/";
static const std::string RES_CHARA_DIR  = "map/character/";
static const std::string RES_FILED_DIR  = "map/field/";
static const std::string RES_BATTLE_DIR = "battle/";

static const int FRAME = 60;

static const cocos2d::Size GAME_RESOLUTION = cocos2d::Size(480.0f, 800.0f);

static const float NEND_BANNER_HEIGHT = 100.0f;

// これってUtilにするべきかも
class Common {
public:
    
    enum E_DIRECTON {
        UP,
        DOWN,
        LEFT,
        RIGHT
    };
    
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
