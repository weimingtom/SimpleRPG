//
//  Common.cpp
//
//  Created by shamaton
//
//

#include "Common.h"

#ifdef NEND_ENABLE

#include "NendIconModule.h"
#include "NendModule.h"
#include "NendInterstitialModule.h"

#endif

USING_NS_CC;

Vec2 Common::get_layer_position(Size layer_size, Vec2 orig) {
	auto ret = Vec2(
		orig.x - layer_size.width/2,
		orig.y - layer_size.height/2
	);
	return ret;
}

int Common::sec2frame(float sec) {
	return int (FRAME * sec);
}

//---------------------------------------------------------
// 任意の文字をsepで分割
//---------------------------------------------------------
std::vector<std::string> Common::split(const std::string &str, char sep)
{
    std::vector<std::string> v;
    std::stringstream ss(str);
    std::string buffer;
    while( std::getline(ss, buffer, sep) ) {
        v.push_back(buffer);
    }
    return v;
}

#ifdef NEND_ENABLE
//---------------------------------------------------------
// nendの設定
//---------------------------------------------------------
void Common::nend_icon_enable() {
#if NEND_TEST
	// for iOS
	char api_key_for_ios[] = "2349edefe7c2742dfb9f434de23bc3c7ca55ad22";
	char spot_id_for_ios[] = "101281";
	
	// for Android
	char api_key_for_and[] = "0c734134519f25412ae9a9bff94783b81048ffbe";
	char spot_id_for_and[] = "101282";
#else
	// for iOS
	char api_key_for_ios[] = "0739f935f7b63ab639c2df47d62720a265705613";
	char spot_id_for_ios[] = "320021";
	
	// for Android
	char api_key_for_and[] = "ddb8f0153170d67287a336762d9ad790f6db4ff0";
	char spot_id_for_and[] = "320019";
#endif
	
	if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) {
		NendIconModule::createNADIconLoader(api_key_for_ios, spot_id_for_ios);
	} else if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) {
		NendIconModule::createNADIconLoader(api_key_for_and, spot_id_for_and);
	}
	
	NendIconModule::createNADIconViewBottom();
	NendIconModule::load();
	NendIconModule::showNADIconView();
}

// 画面中心に大きなバナーを出す
void Common::nend_banner_enable() {
	// TODO 書き換え
#if NEND_TEST
	// for iOS
	char api_key_for_ios[] = "a6eca9dd074372c898dd1df549301f277c53f2b9";
	char spot_id_for_ios[] = "3172";
	
	// for Android
	char api_key_for_and[] = "c5cb8bc474345961c6e7a9778c947957ed8e1e4f";
	char spot_id_for_and[] = "3174";
#else
    // for iOS
    char api_key_for_ios[] = "53b0c00aa04b5578a4fe79756715b172bed36305";
    char spot_id_for_ios[] = "347325";
    
    // for Android
    char api_key_for_and[] = "42c6b41f26825223fab07486c11f7db5e693eca4";
    char spot_id_for_and[] = "347324";
#endif
	
	if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) {
		NendModule::createNADViewBottom(api_key_for_ios, spot_id_for_ios);
	} else if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) {
		NendModule::createNADViewBottom(api_key_for_and, spot_id_for_and);
	}
	NendModule::showNADView();
}

// インタースティシャル広告
void Common::nend_interstitial_enable() {
	// TODO 書き換え
#if NEND_TEST
	// for iOS
	char api_key_for_ios[] = "308c2499c75c4a192f03c02b2fcebd16dcb45cc9";
	char spot_id_for_ios[] = "213208";
	
	// for Android
	char api_key_for_and[] = "8c278673ac6f676dae60a1f56d16dad122e23516";
	char spot_id_for_and[] = "213206";
#else
	// for iOS
	char api_key_for_ios[] = "308c2499c75c4a192f03c02b2fcebd16dcb45cc9";
	char spot_id_for_ios[] = "213208";
	
	// for Android
	char api_key_for_and[] = "8c278673ac6f676dae60a1f56d16dad122e23516";
	char spot_id_for_and[] = "213206";
#endif
	
	if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) {
		NendInterstitialModule::createNADInterstitial(api_key_for_ios, spot_id_for_ios);
	} else if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) {
		NendInterstitialModule::createNADInterstitial(api_key_for_and, spot_id_for_and);
	}
	NendInterstitialModule::showNADInterstitialView();
}

//---------------------------------------------------------
// nendを非表示にする
//---------------------------------------------------------
void Common::nend_icon_disable() {
	NendIconModule::hideNADIconView();
}

void Common::nend_banner_disable() {
	NendModule::hideNADView();
}

void Common::nend_interstitial_disable() {
	NendInterstitialModule::dismissNADInterstitialView();
}
#endif