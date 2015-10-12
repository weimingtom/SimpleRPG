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

class DamageText : public cocos2d::Label
{
public:
    static DamageText* create(cocos2d::Vec2 position);
	
	void do_animation(int damage);
	void do_player_side_animation(int damage);
	
	bool is_now_animating(void);
	
private:
    void _init(cocos2d::Vec2 position);
	cocos2d::Vec2 init_position;
};

#endif /* defined(__Choukyu__DamageText__) */
