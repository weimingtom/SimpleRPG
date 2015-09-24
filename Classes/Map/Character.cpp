//
//  Player.cpp
//
//  Created by shamaton
//
//

#include "Character.h"

enum E_TAG_CHARACTER_ACTION {
    ACTION_NONE,
    ACTION_ANIMATION,
    NR_CHARACTER_ACTION
};

enum E_TAG {
    TAG_NONE,
    NR_TAG
};

USING_NS_CC;

//---------------------------------------------------------
// initial method
//---------------------------------------------------------
Character* Character::create(Vec2 map, Size tile)
{
    Character *_chara = new Character();
    std::string file_name = "player_front_01.png";
    if (_chara && _chara->initWithSpriteFrameName(file_name))
    {
        _chara->_init(map, tile);
        _chara->autorelease();
        _chara->retain();
        return _chara;
    }
    
    CC_SAFE_DELETE(_chara);
    return NULL;
}

void Character::_init(Vec2 map, Size tile) {
    this->tile_size = tile;
    
    this->now_map_x = (int)map.x;
    this->now_map_y = (int)map.y;
    
    // アクションマップ作成
    this->animation_map[DIRECTON::UP]    = "back";
    this->animation_map[DIRECTON::DOWN]  = "front";
    this->animation_map[DIRECTON::LEFT]  = "left";
    this->animation_map[DIRECTON::RIGHT] = "right";
    
    this->_set_animation(DIRECTON::DOWN);
}

//=========================================================
// instance methods
//=========================================================

//=========================================================
// 移動アニメーション切り替え
//=========================================================
void Character::set_directon(DIRECTON direction) {
    // 同じ場合は無視
    if (direction == this->now_direction) {
        return;
    }
    
    // アニメーションを停止
    this->stopActionByTag(ACTION_ANIMATION);
    
    this->_set_animation(direction);
}

void Character::_set_animation(DIRECTON directon) {
    
    auto animation_name = "chara1_walk_" + this->animation_map[directon];
    assert(animation_name != "chara1_walk_");
    
    // アニメーションを生成
    auto cache = AnimationCache::getInstance();
    auto animation = cache->getAnimation(animation_name);
    
    auto animate = Animate::create(animation);
    auto action_req = RepeatForever::create(animate);
    action_req->setTag(ACTION_ANIMATION);
    this->runAction(action_req);
    
    // 更新
    this->now_direction = directon;
}

