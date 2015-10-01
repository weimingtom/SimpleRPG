//
//  Player.cpp
//
//  Created by shamaton
//
//

#include "Player.h"

enum E_TAG_PLAYER_ACTION {
    PLAYER_ACTION_NONE,
    PLAYER_ACTION_ANIMATION,
    NR_PLAYER_ACTION
};

enum E_TAG {
    TAG_NONE,
    NR_TAG
};

USING_NS_CC;

//---------------------------------------------------------
// initial method
//---------------------------------------------------------
Player* Player::create()
{
    Player *_player = new Player();
    std::string file_name = "player_front_01.png";
    if (_player && _player->initWithSpriteFrameName(file_name))
    {
        _player->_init();
        _player->autorelease();
        _player->retain();
        return _player;
    }
    
    CC_SAFE_DELETE(_player);
    return NULL;
}

void Player::_init() {
    // アクションマップ作成
    this->animation_map[Common::E_DIRECTON::UP]    = "back";
    this->animation_map[Common::E_DIRECTON::DOWN]  = "front";
    this->animation_map[Common::E_DIRECTON::LEFT]  = "left";
    this->animation_map[Common::E_DIRECTON::RIGHT] = "right";
    
    this->_set_animation(Common::E_DIRECTON::DOWN);
}

//=========================================================
// instance methods
//=========================================================

//=========================================================
// 移動アニメーション切り替え
//=========================================================
void Player::move(Common::E_DIRECTON type) {
    // 同じ場合は無視
    if (type == this->now_type) {
        return;
    }
    
    this->_set_animation(type);
}

void Player::_set_animation(Common::E_DIRECTON type) {
    
    auto animation_name = "player_walk_" + this->animation_map[type];
    assert(animation_name != "player_walk_");
    
    // アニメーションを停止
    this->stopActionByTag(PLAYER_ACTION_ANIMATION);
    
    // アニメーションを生成
    auto cache = AnimationCache::getInstance();
    auto animation = cache->getAnimation(animation_name);
    
    auto animate = Animate::create(animation);
    auto action_req = RepeatForever::create(animate);
    action_req->setTag(PLAYER_ACTION_ANIMATION);
    this->runAction(action_req);
    
    // 更新
    this->now_type = type;
}

//---------------------------------------------------------
// 指定座標の方を向く
//---------------------------------------------------------
// UGLY プレーヤーが位置情報をもってない
void Player::set_face_by_pos(int diff_x, int diff_y) {
    // 上
    if (diff_x == 0 && diff_y == -1 ) {
        this->_set_animation(Common::E_DIRECTON::UP);
    }
    // 下
    else if (diff_x == 0 && diff_y == 1 ) {
        this->_set_animation(Common::E_DIRECTON::DOWN);
    }
    // 左
    else if (diff_x == -1 && diff_y == 0 ) {
        this->_set_animation(Common::E_DIRECTON::LEFT);
    }
    // 右
    else if (diff_x == 1 && diff_y == 0 ) {
        this->_set_animation(Common::E_DIRECTON::RIGHT);
    }
    else {
        assert(0);
    }
}

Common::E_DIRECTON Player::get_direction() {
    return this->now_type;
}


