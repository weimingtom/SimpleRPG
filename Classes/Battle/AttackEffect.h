//
//  AttackEffect.h
//  Choukyu
//
//  Created by shamaton.
//
//

#ifndef __Choukyu__AttackEffect__
#define __Choukyu__AttackEffect__

#include "cocos2d.h"
#include "CreateFunc.h"

class AttackEffect : public cocos2d::Layer, create_func<AttackEffect>
{
public:
	virtual bool init(cocos2d::Vec2 player_pos, cocos2d::Vec2 enemy_pos); // 初期化処理
	void onEnter() override; // 表示前処理
	using create_func::create;
	
	void do_enemy_animation(void);
	bool is_now_enemy_animation(void);
	
private:
};

#endif /* defined(__Choukyu__AttackEffect__) */
