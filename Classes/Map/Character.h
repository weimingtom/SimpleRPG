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
    
    static Character* create(cocos2d::Vec2 map, cocos2d::Size tile, cocos2d::TMXTiledMap* ref_map);
    
    void update();
    
    void set_directon(DIRECTON directon);
    void set_face();
    void talk_end();
    
    cocos2d::Vec2 get_map_positon();
    std::vector<std::string> get_serif();
    
private:
    void _init(cocos2d::Vec2 map, cocos2d::Size tile);
    
    void _update_move();
    void _set_animation(DIRECTON directon);
    
    std::vector<std::string> serif;
    bool is_now_talking;
    
    int now_map_x;
    int now_map_y;
    float now_move_amount;
    float MOVE_SPEED; // 仮
    bool is_move; // trueのとき動く
    cocos2d::Size tile_size;
    
    std::map<int, std::string> animation_map;
    DIRECTON now_direction;
    
    cocos2d::TMXTiledMap* ref_map;
    bool _collision_check(Character::DIRECTON dir);
    
    
};


#endif /* defined(__MAP__CHARACTER__) */
