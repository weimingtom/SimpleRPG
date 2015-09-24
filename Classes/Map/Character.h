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
    
    static Character* create(cocos2d::Vec2 map, cocos2d::Size tile);
    
    void set_directon(DIRECTON directon);
    
private:
    void _init(cocos2d::Vec2 map, cocos2d::Size tile);
    
    void _set_animation(DIRECTON directon);
    
    int now_map_x;
    int now_map_y;
    float now_move_amount;
    cocos2d::Size tile_size;
    
    std::map<int, std::string> animation_map;
    DIRECTON now_direction;
    
};


#endif /* defined(__MAP__CHARACTER__) */
