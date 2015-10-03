//
//  Common.cpp
//  
//
//  Created by shamaton
//
//

#include "Sound.h"

#include "SimpleAudioEngine.h"
using namespace CocosDenshion;

//=====================================================
// SOUND EFFECT
//=====================================================
void preload_se(std::string filename) {
	SimpleAudioEngine::getInstance()->preloadEffect(filename.c_str());
}


void play_se(std::string filename) {
	float volume = SimpleAudioEngine::getInstance()->getEffectsVolume();
	if (volume < 0 && volume > 1) {
		set_se_volume(0.5);
	}
#if PLAY_SE
	SimpleAudioEngine::getInstance()->playEffect(filename);
#endif
}

void set_se_volume(float volume) {
	SimpleAudioEngine::getInstance()->setEffectsVolume(volume);
}

//=====================================================
// BGM
//=====================================================
//音楽データを読み込む
void preload_bgm(std::string filename) {
	SimpleAudioEngine::getInstance()->preloadBackgroundMusic(filename.c_str());
}

//音楽を再生する
void play_bgm(std::string filename) {
	float volume = SimpleAudioEngine::getInstance()->getBackgroundMusicVolume();
	if (volume < 0 && volume > 1) {
		set_bgm_volume(0.5);
	}
#if PLAY_BGM
	SimpleAudioEngine::getInstance()->playBackgroundMusic(filename, true);
#endif
}

//一時停止
void pause_bgm() {
	SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

//再開
void resume_bgm() {
	SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}

//音楽を先頭に戻す
void rewind_bgm() {
	SimpleAudioEngine::getInstance()->rewindBackgroundMusic();
}

//今、BGMが流れているかどうか
bool is_playing_bgm() {
	return SimpleAudioEngine::getInstance()->isBackgroundMusicPlaying();
}

// ボリュームの設定
void set_bgm_volume(float volume) {
	SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(volume);
}
