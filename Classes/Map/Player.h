//
//  Player.h
//
//  Created by shamaton
//
//

#ifndef __MAP__PLAYER__
#define __MAP__PLAYER__

#include "cocos2d.h"

class Player : public cocos2d::Sprite
{
public:
    enum ACTION_TYPE {
        NONE,
        MOVE_UP,
        MOVE_DOWN,
        MOVE_LEFT,
        MOVE_RIGHT,
    };
    
    static Player* create();
    
    void move(Player::ACTION_TYPE type);
    
private:
    void _init();
    
    void _set_animation(ACTION_TYPE type);
    
    std::map<int, std::string> animation_map;
    ACTION_TYPE now_type;
    
};


#endif /* defined(__MAP__PLAYER__) */
