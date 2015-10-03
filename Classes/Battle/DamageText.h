//
//  DamageText.h
//  Choukyu
//
//  Created by shamaton.
//
//

#ifndef __Choukyu__DamageText__
#define __Choukyu__DamageText__

#include "cocos2d.h"
#include "CreateFunc.h"

class DamageText : public cocos2d::Label, create_func<DamageText>
{
public:
	virtual bool init(cocos2d::Vec2 position); // 初期化処理
	void onEnter() override; // 表示前処理
	using create_func::create;
	
	void do_animation(int damage);
	void do_player_side_animation(int damage);
	
	bool is_now_animating(void);
	
private:
	cocos2d::Vec2 init_position;
};

#endif /* defined(__Choukyu__DamageText__) */
