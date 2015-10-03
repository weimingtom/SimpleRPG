//
//  Player.h
//
//  Created by shamaton
//
//

#ifndef __MAP__PLAYER__
#define __MAP__PLAYER__

#include "cocos2d.h"
#include "Common.h"

class Player : public cocos2d::Sprite
{
public:
    
    static Player* create();
    
    void move(Common::E_DIRECTON type);
    void set_face_by_pos(int diff_x, int diff_y);
    Common::E_DIRECTON get_direction();
    
    IntVec2 get_next_pos(IntVec2 now_position, Common::E_DIRECTON directon);
    
private:
    void _init();
    
    void _set_animation(Common::E_DIRECTON type);
    
    std::map<int, std::string> animation_map;
    Common::E_DIRECTON now_type;
    
};


#endif /* defined(__MAP__PLAYER__) */
