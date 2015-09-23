//
//  Character.h
//
//  Created by shamaton
//
//

#ifndef __MAP__CHARACTER__
#define __MAP__CHARACTER__

#include "cocos2d.h"

class Character : public cocos2d::Sprite
{
public:
    enum DIRECTON {
        NONE,
        UP,
        DOWN,
        LEFT,
        RIGHT,
    };
    
    static Character* create();
    
    void set_directon(DIRECTON directon);
    
private:
    void _init();
    
    void _set_animation(DIRECTON directon);
    
    std::map<int, std::string> animation_map;
    DIRECTON now_direction;
    
};


#endif /* defined(__MAP__CHARACTER__) */
