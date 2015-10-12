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
    this->animation_map[Common::E_DIRECTON::UP]    = "back";
    this->animation_map[Common::E_DIRECTON::DOWN]  = "front";
    this->animation_map[Common::E_DIRECTON::LEFT]  = "left";
    this->animation_map[Common::E_DIRECTON::RIGHT] = "right";
    
    this->MOVE_SPEED = 1.0f;
    
    this->_set_animation(Common::E_DIRECTON::DOWN);
    
    // かり
    this->serif = {
        "ほれほれ",
        "たたたたたたたたたたたたたたつ",
        "br;",
        "こんにちは",
        "これはメッセージ",
        "ながれる　あれ",
        "テスト２",
        "yes_no;9,11",
        "こたえはイエス！",
        "end;",
        "答えはノー",
        "もう一度きくぞ",
        "jump;8"
    };
}

//=========================================================
// instance methods
//=========================================================
void Character::update() {
    if (this->is_move) {
        this->_update_move();
    }
    // 話し中は何もしない
    if (this->is_now_talking) {
        return;
    }
    
    if (!this->is_move && arc4random() % 200 < 2) {
        // 方向を決めて動かす
        std::vector<Common::E_DIRECTON> dirs = {
            Common::E_DIRECTON::UP,
            Common::E_DIRECTON::DOWN,
            Common::E_DIRECTON::LEFT,
            Common::E_DIRECTON::RIGHT
        };
        auto dir = dirs[arc4random() % dirs.size()];
        if (!this->_collision_check(dir)) {
            this->set_directon(dir);
            this->is_move = true;
        }
    }
}

//---------------------------------------------------------
// 移動処理
//---------------------------------------------------------
void Character::_update_move() {
    bool is_move_end = false;
    
    switch (this->now_direction) {
        case Common::E_DIRECTON::UP:
            this->now_move_amount += MOVE_SPEED;
            this->setPositionY(this->getPositionY() + MOVE_SPEED);
            
            if (this->now_move_amount >= tile_size.height) {
                this->now_map_y -= 1;
                is_move_end = true;
            }
            break;
            
        case Common::E_DIRECTON::DOWN:
            this->now_move_amount += MOVE_SPEED;
            this->setPositionY(this->getPositionY() - MOVE_SPEED);
            
            if (this->now_move_amount >= tile_size.height) {
                this->now_map_y += 1;
                is_move_end = true;
            }
            break;
            
        case Common::E_DIRECTON::LEFT:
            this->now_move_amount += MOVE_SPEED;
            this->setPositionX(this->getPositionX() - MOVE_SPEED);
            
            if (this->now_move_amount >= tile_size.height) {
                this->now_map_x -= 1;
                is_move_end = true;
            }
            break;
            
        case Common::E_DIRECTON::RIGHT:
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
bool Character::_collision_check(Common::E_DIRECTON dir) {
    int check_x = this->now_map_x;
    int check_y = this->now_map_y;
    switch (dir) {
        case Common::E_DIRECTON::UP:
            check_y -= 1;
            break;
        case Common::E_DIRECTON::DOWN:
            check_y += 1;
            break;
        case Common::E_DIRECTON::LEFT:
            check_x -= 1;
            break;
        case Common::E_DIRECTON::RIGHT:
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
void Character::set_directon(Common::E_DIRECTON direction) {
    // 同じ場合は無視
    if (direction == this->now_direction) {
        return;
    }
    
    this->_set_animation(direction);
}

void Character::_set_animation(Common::E_DIRECTON directon) {
    
    auto animation_name = "chara1_walk_" + this->animation_map[directon];
    assert(animation_name != "chara1_walk_");
    
    // アニメーションを停止
    this->stopActionByTag(ACTION_ANIMATION);
    
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

void Character::set_face(Common::E_DIRECTON player_direction) {
    // プレーヤーと反対を向ける、もっと綺麗に書けそう
    if (player_direction == Common::E_DIRECTON::UP) {
        this->_set_animation(Common::E_DIRECTON::DOWN);
    }
    else if (player_direction == Common::E_DIRECTON::DOWN) {
        this->_set_animation(Common::E_DIRECTON::UP);
    }
    else if (player_direction == Common::E_DIRECTON::LEFT) {
        this->_set_animation(Common::E_DIRECTON::RIGHT);
    }
    else if (player_direction == Common::E_DIRECTON::RIGHT) {
        this->_set_animation(Common::E_DIRECTON::LEFT);
    }
    this->is_now_talking = true;
}

void Character::talk_end() {
    this->is_now_talking = false;
}

//---------------------------------------------------------
// マップ上の座標を返す
//---------------------------------------------------------
IntVec2 Character::get_map_positon() {
    return IntVec2(this->now_map_x, this->now_map_y);
}

//---------------------------------------------------------
// セリフを取得
//---------------------------------------------------------
std::vector<std::string> Character::get_serif() {
    return this->serif;
}