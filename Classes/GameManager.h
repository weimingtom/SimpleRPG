//
//  GameManager.h
//
//  Created by shamaton.
//
//

#ifndef __GameManager__
#define __GameManager__

#include "Common.h"

// プレイヤー情報
typedef struct {
    int attack;
    int defense;
    int hp;
} tPlayerInfo;

class GameManager {
private:
    std::string load_map_str; // 読み込むマップのファイル名
    int map_init_x; // マップ初期化時のX座標
    int map_init_y; // マップ初期化時のY座標
    
    tPlayerInfo player_info;
    
	// 単独での生成やコピーをさせないため、コンストラクタをprivateへ
	GameManager();
	GameManager(const GameManager& obj);
	GameManager& operator=(const GameManager& obj);
	
	~GameManager();
	
public:
	// singleton
	static GameManager* getInstance(void);
    
    void set_load_map_name(std::string name);
    std::string get_load_map_name();
    
    void set_map_init_position(int x, int y);
    cocos2d::Vec2 get_map_init_position();
	
};

#endif /* defined(__GameManager__) */
