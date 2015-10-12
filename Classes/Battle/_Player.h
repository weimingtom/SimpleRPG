//
//  Player.h
//  Choukyu
//
//  Created by shamaton.
//
//

#ifndef __Choukyu__Player__
#define __Choukyu__Player__

#include "cocos2d.h"
#include "CreateFunc.h"

class _Player : public cocos2d::Sprite, create_func<_Player>
{
public:
	virtual bool init(); // 初期化処理
	void onEnter() override; // 表示前処理
	using create_func::create;
	
	//CREATE_FUNC(Players); // create関数作成マクロ
	
	//CC_SYNTHESIZE(Players, _player, Player); // プレーヤー情報
	
	virtual void setDisplayFrameWithAnimationName(const std::string &animationName, ssize_t frameIndex);
	
	void start_animation(void);
	
	void attack_animation(void);
	void damage_animation(void);
	void die_animation(void);
	void win_animation_repeat(void);
	
	bool is_now_animation();
	
	cocos2d::Vec2 get_default_position();
	
private:
	cocos2d::Vec2 default_pos;
};

#endif /* defined(__Choukyu__Player__) */
