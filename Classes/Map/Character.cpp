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
Character* Character::create(Vec2 map, Size tile, TMXTiledMap* ref_map)
{
    Character *_chara = new Character();
    std::string file_name = "player_front_01.png";
    if (_chara && _chara->initWithSpriteFrameName(file_name))
    {
        _chara->_init(map, tile);
        _chara->ref_map = ref_map;
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
    
    this->MOVE_SPEED = 1.0f;
    
    this->_set_animation(DIRECTON::DOWN);
}

//=========================================================
// instance methods
//=========================================================
void Character::update() {
    if (!this->is_move && arc4random() % 200 < 2) {
        CCLOG("move!!");
        // 方向を決めて動かす
        std::vector<DIRECTON> dirs = {UP, DOWN, LEFT, RIGHT};
        auto dir = dirs[arc4random() % dirs.size()];
        if (!this->_collision_check(dir)) {
            this->set_directon(dir);
            this->is_move = true;
        }
    }
    if (this->is_move) {
        this->_update_move();
    }
}

//---------------------------------------------------------
// 移動処理
//---------------------------------------------------------
void Character::_update_move() {
    bool is_move_end = false;
    
    switch (this->now_direction) {
        case UP:
            this->now_move_amount += MOVE_SPEED;
            this->setPositionY(this->getPositionY() + MOVE_SPEED);
            
            if (this->now_move_amount >= tile_size.height) {
                this->now_map_y -= 1;
                is_move_end = true;
            }
            break;
            
        case DOWN:
            this->now_move_amount += MOVE_SPEED;
            this->setPositionY(this->getPositionY() - MOVE_SPEED);
            
            if (this->now_move_amount >= tile_size.height) {
                this->now_map_y += 1;
                is_move_end = true;
            }
            break;
            
        case LEFT:
            this->now_move_amount += MOVE_SPEED;
            this->setPositionX(this->getPositionX() - MOVE_SPEED);
            
            if (this->now_move_amount >= tile_size.height) {
                this->now_map_x -= 1;
                is_move_end = true;
            }
            break;
            
        case RIGHT:
            this->now_move_amount += MOVE_SPEED;
            this->setPositionX(this->getPositionX() + MOVE_SPEED);
            
            if (this->now_move_amount >= tile_size.height) {
                this->now_map_x += 1;
                is_move_end = true;
            }
            break;
            
        default:
            break;
    }
    
    if (is_move_end) {
        this->is_move = false;
        this->now_move_amount = 0;
    }
    
}

//---------------------------------------------------------
// 移動先の衝突チェック
//---------------------------------------------------------
bool Character::_collision_check(Character::DIRECTON dir) {
    int check_x = this->now_map_x;
    int check_y = this->now_map_y;
    switch (dir) {
        case UP:
            check_y -= 1;
            break;
        case DOWN:
            check_y += 1;
            break;
        case LEFT:
            check_x -= 1;
            break;
        case RIGHT:
            check_x += 1;
            break;
        default:
            assert(0);
            break;
    }
    // 一旦、端は障害物
    if (check_x < 0 || check_y < 0) {
        return true;
    }
    
    auto collision_layer = this->ref_map->getLayer("Collision");
    int col_gid = collision_layer->getTileGIDAt(Vec2(check_x, check_y));
    
    return (col_gid > 0);
}

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

//---------------------------------------------------------
// マップ上の座標を返す
//---------------------------------------------------------
Vec2 Character::get_map_positon() {
    return Vec2(this->now_map_x, this->now_map_y);
}
