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

class _Player : public cocos2d::Sprite
{
public:
    
    static _Player* create();
	
	void start_animation(void);
	
	void attack_animation(void);
	void damage_animation(void);
	void die_animation(void);
	void win_animation_repeat(void);
	
	bool is_now_animation();
	
	cocos2d::Vec2 get_default_position();
	
private:
    void _init();
	cocos2d::Vec2 default_pos;
};

#endif /* defined(__Choukyu__Player__) */
