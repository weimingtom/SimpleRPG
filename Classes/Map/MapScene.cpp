#include "MapScene.h"

#include "../Loading/Loading.h"

#include "../Util/MessageLayer.h"

#include "Common.h"
#include "GameManager.h"

#include "RouteSearch.h"
#include "Player.h"
#include "Character.h"

#include "spine/Json.h"

USING_NS_CC;

enum E_TAG {
    //
    // (0, 0) - (1024, 1024) までタイル用に予約する
    //
    TAG_NONE = 11000000,
    TAG_MAP,
    TAG_PLAYER,
    TAG_CHARACTER, // 100人くらい予約しとく
    TAG_TOUCH_POINT = TAG_CHARACTER + 100,
    TAG_LAYER_MESSAGE_WINDOW,
    NR_TAGS
};

enum E_GS_ORDER {
    GS_NONE,
    GS_MAP,
    GS_CHARACTER,
    GS_PLAYER,
    GS_TOUCH_POINT,
    GS_MAP_EDGE_HIDE,
    GS_MESSAGE_WINDOW,
    NR_GSS
};

static const std::string MAP_LAYER_TYPE_MAP = "Map";
static const std::string MAP_LAYER_TYPE_COLLISION = "Collision";

const float MOVE_SPEED = 1.0f;

static const int WIDTH  = 15;
static const int HEIGHT = 15;

Scene* MapScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = MapScene::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool MapScene::init()
{
    if ( !Layer::init() ) {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    // 初期化
    this->now_route = ROUTE_NONE;
    
    // シングルタップリスナーを用意する
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(_swallowsTouches);
    
    // 各イベントの割り当て
    listener->onTouchBegan     = CC_CALLBACK_2(MapScene::onTouchBegan, this);
    listener->onTouchMoved     = CC_CALLBACK_2(MapScene::onTouchMoved, this);
    listener->onTouchEnded     = CC_CALLBACK_2(MapScene::onTouchEnded, this);
    listener->onTouchCancelled = CC_CALLBACK_2(MapScene::onTouchCancelled, this);
    
    // イベントディスパッチャにシングルタップ用リスナーを追加する
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
    // キー入力
    if ( 0 ) {
        
        auto listener = EventListenerKeyboard::create();
        
        listener->onKeyPressed = [&](EventKeyboard::KeyCode keyCode, Event* event) {
            auto str = String::createWithFormat("%c", keyCode);
            CCLOG("%s", str->getCString());
        };
        
        listener->onKeyReleased = [](EventKeyboard::KeyCode keyCode, Event* event) {
            // 放されたときの処理
        };
        
        _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
        CCLOG("seww!!");
    }
    
    
    auto frame_cache = SpriteFrameCache::getInstance();
    frame_cache->removeSpriteFrames();
    frame_cache->addSpriteFramesWithFile("player.plist");
    frame_cache->addSpriteFramesWithFile("character.plist");
    
    auto animation_cache = AnimationCache::getInstance();
    //animation_cache->destroyInstance();
    animation_cache->addAnimationsWithFile("player_animations.plist");
    animation_cache->addAnimationsWithFile("character_animations.plist");
    
    // タッチしたとこ
    auto dot_rect = Rect(0, 0, 32, 32);
    auto dot = Sprite::create();
    dot->setTextureRect(dot_rect);
    dot->setColor(Color3B::RED);
    dot->setOpacity(128);
    dot->setTag(TAG_TOUCH_POINT);
    dot->setPosition(-100, -100);
    this->addChild(dot, GS_TOUCH_POINT);
    
    // 開始時の位置設定
    auto gm = GameManager::getInstance();
    this->now_pos_x = gm->get_map_init_position().x;
    this->now_pos_y = gm->get_map_init_position().y;
    this->_init_map(gm->get_load_map_name());
    this->_init_jump_info();
    
    // プレーヤー
    auto player = Player::create();
    auto center_pos = Vec2(visibleSize.width/2, visibleSize.height/2);
    player->setPosition(center_pos);
    player->setTag(TAG_PLAYER);
    this->addChild(player, GS_PLAYER);
    
    // キャラテスト
    int testx = 6, testy = 6;
    auto tile_size = this->_get_map()->getTileSize();
    auto chara = Character::create(Vec2(testx, testy), tile_size, this->_get_map());
    chara->setPosition(center_pos.x + (testx - this->now_pos_x) * tile_size.width, center_pos.y - (testy - this->now_pos_y) * tile_size.height);
    chara->setTag(TAG_CHARACTER);
    this->addChild(chara, GS_CHARACTER);
    this->chara_tags.push_back(TAG_CHARACTER);
    
    // test
    this->_test();
    
    // 毎フレームでupdateを実行させる
    this->schedule(schedule_selector(MapScene::update));
    this->scheduleUpdate();
    
    return true;
}

void MapScene::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event * event)
{
    auto str = String::createWithFormat("%c", keyCode);
    CCLOG("hoge %s", str->getCString());
}

void MapScene::_test() {
    
    auto message_layer = MessageLayer::create();
    message_layer->setTag(TAG_LAYER_MESSAGE_WINDOW);
    this->addChild(message_layer, GS_MESSAGE_WINDOW);
    
    
    auto closeItem = MenuItemImage::create("CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(MapScene::_test_button, this));
    
    auto layer_size = this->getContentSize();
    closeItem->setPosition(Vec2(layer_size.width - closeItem->getContentSize().width/2 ,
                                closeItem->getContentSize().height/2));
    
    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 999);
}


void MapScene::_test_button(Ref* pSender)
{
    CCLOG("push!!");
    auto message_window = (MessageLayer *)this->getChildByTag(TAG_LAYER_MESSAGE_WINDOW);
    message_window->set_message({"a", "b"});
}



//---------------------------------------------------------
// main loop
//---------------------------------------------------------
void MapScene::update(float delta) {
    // 移動
    if (this->now_route != ROUTE_NONE) {
        this->_player_move();
    }
    // 他更新
    auto chara = (Character *)this->getChildByTag(TAG_CHARACTER);
    chara->update();
}

//---------------------------------------------------------
// マップ上を移動する
//---------------------------------------------------------
void MapScene::_player_move() {
    
    // 歩き終わったか
    bool is_move_end = false;
    
    // 歩く
    auto _tile_map = this->_get_map();
    auto tile_size = _tile_map->getTileSize();
    
    switch (this->now_route) {
        case ROUTE_UP:
        {
            // 移動開始時にマップを追加
            if (this->now_move_amount == 0) {
                this->_add_x_line_tile(-(HEIGHT/2 + 1));
                auto player = (Player *)this->getChildByTag(TAG_PLAYER);
                player->move(Common::E_DIRECTON::UP);
            }
            
            // マップを移動
            this->_update_object_pos(0, -MOVE_SPEED);
            
            this->now_move_amount += MOVE_SPEED;
            
            if (this->now_move_amount >= tile_size.height) {
                is_move_end = true;
                // 現在地更新
                this->_update_now_pos_y(-1);
                
                // 不要なタイルを削除し、一個先を追加しておく
                this->_remove_x_line_tile((HEIGHT/2 + 1));
            }
        }
            break;
        case ROUTE_DOWN:
        {
            // 移動開始時にマップを追加
            if (this->now_move_amount == 0) {
                this->_add_x_line_tile((HEIGHT/2 + 1));
                auto player = (Player *)this->getChildByTag(TAG_PLAYER);
                player->move(Common::E_DIRECTON::DOWN);
            }
            
            // マップを移動
            this->_update_object_pos(0, MOVE_SPEED);
            
            this->now_move_amount += MOVE_SPEED;
            
            if (this->now_move_amount >= tile_size.height) {
                is_move_end = true;
                // 現在地更新
                this->_update_now_pos_y(+1);
                
                // 不要なタイルを削除し、一個先を追加しておく
                this->_remove_x_line_tile(-(HEIGHT/2 + 1));
            }
        }
            break;
        case ROUTE_LEFT:
        {
            // 移動開始時にマップを追加
            if (this->now_move_amount == 0) {
                this->_add_y_line_tile(-(WIDTH/2 + 1));
                auto player = (Player *)this->getChildByTag(TAG_PLAYER);
                player->move(Common::E_DIRECTON::LEFT);
            }
            
            // マップを移動
            this->_update_object_pos(MOVE_SPEED, 0);
            
            this->now_move_amount += MOVE_SPEED;
            
            if (this->now_move_amount >= tile_size.height) {
                is_move_end = true;
                // 現在地更新
                this->_update_now_pos_x(-1);
                
                // 不要なタイルを削除し、一個先を追加しておく
                this->_remove_y_line_tile((WIDTH/2 + 1));
            }
        }
            break;
        case ROUTE_RIGHT:
        {
            // 移動開始時にマップを追加
            if (this->now_move_amount == 0) {
                this->_add_y_line_tile((WIDTH/2 + 1));
                auto player = (Player *)this->getChildByTag(TAG_PLAYER);
                player->move(Common::E_DIRECTON::RIGHT);
            }
            
            // マップを移動
            this->_update_object_pos(-MOVE_SPEED, 0);
            
            this->now_move_amount += MOVE_SPEED;
            
            if (this->now_move_amount >= tile_size.height) {
                is_move_end = true;
                // 現在地更新
                this->_update_now_pos_x(+1);
                
                // 不要なタイルを削除し、一個先を追加しておく
                this->_remove_y_line_tile(-(WIDTH/2 + 1));
            }
        }
            break;
        default:
            assert(0);
            break;
    }
    
    // 歩き終わってまだroutesがある場合、再度セットする
    if (is_move_end) {
        this->now_route = ROUTE_NONE;
        this->now_move_amount = 0;
        
        // マップ切り替えの場合
        if (this->_check_jump_info()) {
            this->routes.clear();
            this->routes.shrink_to_fit();
            this->_load_next_map();
        }
        
        // 歩く
        if (this->routes.size()) {
            this->now_route = this->routes.back();
            this->routes.pop_back();
        } else {
            // タッチを消す
            auto _touch_rect = this->getChildByTag(TAG_TOUCH_POINT);
            _touch_rect->setPosition(-100, -100);
        }
    }
}

//---------------------------------------------------------
// マップとキャラを更新
//---------------------------------------------------------
void MapScene::_update_object_pos(float add_x, float add_y) {
    
    auto goal_tag = this->_get_tile_tag(this->goal_x, this->goal_y);
    
    // マップを移動
    for (auto tag : this->disp_tile_tags) {
        auto tile = this->getChildByTag(tag);
        auto pos  = tile->getPosition();
        tile->setPosition(pos.x + add_x, pos.y + add_y);
        // タッチポイントに合致する場合
        if (tag == goal_tag) {
            auto _touch_rect = this->getChildByTag(TAG_TOUCH_POINT);
            _touch_rect->setPosition(tile->getPosition());
        }
    }
    // きゃら
    for (auto tag : this->chara_tags) {
        auto chara = this->getChildByTag(tag);
        auto pos  = chara->getPosition();
        chara->setPosition(pos.x + add_x, pos.y + add_y);
    }
    
    
}

//---------------------------------------------------------
// x方向に1列追加する
//---------------------------------------------------------
void MapScene::_add_x_line_tile(int fixed_y) {
    auto _tile_map  = this->_get_map();
    auto _map_layer = _tile_map->getLayer(MAP_LAYER_TYPE_MAP);
    
    auto map_size  = _tile_map->getMapSize();
    auto tile_size = _tile_map->getTileSize();
    
    auto layer_size = this->getContentSize();
    auto center_pos = Vec2(layer_size.width/2, layer_size.height/2);
    
    // y座標は固定
    auto fixed_pos_y = -fixed_y * tile_size.height;
    
    auto _add_y = (this->now_pos_y + fixed_y + (int)map_size.height) % (int)map_size.height;
    
    for (int w = 0; w < WIDTH; w++) {
        
        int _w = w - WIDTH/2;
        int _x = (this->now_pos_x + _w + (int)map_size.width)  % (int)map_size.width;
        
        auto add_tile = this->_make_tile(_x, _add_y, _map_layer);
        auto tag = this->_get_tile_tag(_x, _add_y);
        add_tile->setTag(tag);
        this->disp_tile_tags.push_back(tag);
        
        // 指定の座標にオフセット
        int sx =  _w * tile_size.width;
        
        auto _pos = Vec2(center_pos.x + sx, center_pos.y + fixed_pos_y);
        add_tile->setPosition(_pos);
        
        this->addChild(add_tile, GS_MAP);
    }
}

//---------------------------------------------------------
// y方向に1列追加する
//---------------------------------------------------------
void MapScene::_add_y_line_tile(int fixed_x) {
    auto _tile_map  = this->_get_map();
    auto _map_layer = _tile_map->getLayer(MAP_LAYER_TYPE_MAP);
    
    auto map_size  = _tile_map->getMapSize();
    auto tile_size = _tile_map->getTileSize();
    
    auto layer_size = this->getContentSize();
    auto center_pos = Vec2(layer_size.width/2, layer_size.height/2);
    
    // x座標は固定
    auto fixed_pos_x = fixed_x * tile_size.height;
    
    auto _add_x = (this->now_pos_x + fixed_x + (int)map_size.width) % (int)map_size.width;
    
    for (int h = 0; h < HEIGHT; h++) {
        
        int _h = h - HEIGHT/2;
        int _y = (this->now_pos_y + _h + (int)map_size.height) % (int)map_size.height;
        
        auto add_tile = this->_make_tile(_add_x, _y, _map_layer);
        auto tag = this->_get_tile_tag(_add_x, _y);
        add_tile->setTag(tag);
        this->disp_tile_tags.push_back(tag);
        
        // 指定の座標にオフセット
        int sy =  -_h * tile_size.height;
        
        auto _pos = Vec2(center_pos.x + fixed_pos_x, center_pos.y + sy);
        add_tile->setPosition(_pos);
        
        this->addChild(add_tile, GS_MAP);
    }
}

//---------------------------------------------------------
// x方向に1列削除する
//---------------------------------------------------------
void MapScene::_remove_x_line_tile(int fixed_y) {
    auto map_size = this->_get_map()->getMapSize();
    auto _add_y   = (this->now_pos_y + fixed_y + (int)map_size.width) % (int)map_size.width;
    for (int w = 0; w < WIDTH; w++) {
        
        int _w = w - WIDTH/2;
        int _x = (this->now_pos_x + _w + (int)map_size.width)  % (int)map_size.width;
        
        auto remove_tag = this->_get_tile_tag(_x, _add_y);
        this->removeChildByTag(remove_tag);
        // 該当タグを削除
        this->disp_tile_tags.erase(
            remove(this->disp_tile_tags.begin(), this->disp_tile_tags.end(), remove_tag),
            this->disp_tile_tags.end()
        );
    }
}

//---------------------------------------------------------
// y方向に1列削除する
//---------------------------------------------------------
void MapScene::_remove_y_line_tile(int fixed_x) {
    auto map_size = this->_get_map()->getMapSize();
    auto _add_x   = (this->now_pos_x + fixed_x + (int)map_size.width) % (int)map_size.width;
    for (int h = 0; h < HEIGHT; h++) {
        
        int _h = h - HEIGHT/2;
        int _y = (this->now_pos_y + _h + (int)map_size.height) % (int)map_size.height;
        
        auto remove_tag = this->_get_tile_tag(_add_x, _y);
        this->removeChildByTag(remove_tag);
        
        // 該当タグを削除
        this->disp_tile_tags.erase(
            remove(this->disp_tile_tags.begin(), this->disp_tile_tags.end(), remove_tag),
            this->disp_tile_tags.end()
        );
    }
}

//---------------------------------------------------------
// 現在の位置情報更新
//---------------------------------------------------------
void MapScene::_update_now_pos_x(int add_x) {
    auto map_size = this->_get_map()->getMapSize();
    this->now_pos_x = (this->now_pos_x + add_x + (int)map_size.width) % (int)map_size.width;
}

void MapScene::_update_now_pos_y(int add_y) {
    auto map_size = this->_get_map()->getMapSize();
    this->now_pos_y = (this->now_pos_y + add_y + (int)map_size.height) % (int)map_size.height;
}

//---------------------------------------------------------
// 別のマップに移動するべきか
//---------------------------------------------------------
bool MapScene::_check_jump_info() {
    std::string jump_info_map_key = this->_get_jump_info_map_key(this->now_pos_x, this->now_pos_y);
    
    // 情報はない
    if (this->jump_info_map[jump_info_map_key] == 0) {
        return false;
    }
    
    // 情報取得
    for (auto info : this->jump_infos) {
        if (info.map_x == this->now_pos_x && info.map_y == this->now_pos_y) {
            auto gm = GameManager::getInstance();
            gm->set_load_map_name(info.move_map_name);
            gm->set_map_init_position(info.move_x, info.move_y);
            this->is_map_jump = true;
        }
    }
    
    // 念のため
    assert(this->is_map_jump);
    
    return true;
}

//---------------------------------------------------------
// プレーヤーの周囲を確認
//---------------------------------------------------------
bool MapScene::_check_near_player(int near_x, int near_y) {
    // 宝箱があるか
    if (0) {
        return false;
    }
    
    // キャラがいるか
    for (auto tag : this->chara_tags) {
        auto chara = (Character *)this->getChildByTag(tag);
        auto pos = chara->get_map_positon();
        if ( near_x == (int)pos.x && near_y == (int)pos.y) {
            // プレーヤーの方を向けて、会話を表示
            auto player = (Player *)this->getChildByTag(TAG_PLAYER);
            player->set_face_by_pos(near_x - this->now_pos_x, near_y - this->now_pos_y); // マップの回り込み考慮してない
            chara->set_face(player->get_direction());
            // メッセージ表示
            auto message_window = (MessageLayer *)this->getChildByTag(TAG_LAYER_MESSAGE_WINDOW);
            message_window->set_message(chara->get_serif(), chara);
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
// 次のマップへ
//---------------------------------------------------------
void MapScene::_load_next_map() {
    
    auto next_scene = Loading::createScene();
    float duration = 1.0f;
    
    auto p_scene = TransitionFade::create(duration, next_scene);
    if (p_scene) {
        Director::getInstance()->replaceScene(p_scene);
    }
}

//=========================================================
// touch function
//=========================================================
//---------------------------------------------------------
//
//---------------------------------------------------------
bool MapScene::onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event)
{
    // 移動中は何もしない
    if (this->is_map_jump) {
        return true;
    }
    
    auto _tile_map = this->_get_map();
    
    auto touch_pos = touch->getLocation();
    
    // 表示されているマップから場所を検索する
    auto map_size  = _tile_map->getMapSize();
    auto tile_size = _tile_map->getTileSize();
    
    int sx = this->now_pos_x;
    int sy = this->now_pos_y;
    
    // 移動中の場合、移動後の座標からルート検索する
    if (this->now_route != ROUTE_NONE) {
        // matrix分、引いておく
        auto _adjust = RouteSearch::get_matrix((E_DIRECTON)this->now_route);
        sx -= _adjust.x;
        sy -= _adjust.y;
    }
    // 回り込み考慮
    sx = (sx + (int)map_size.width) % (int)map_size.width;
    sy = (sy + (int)map_size.height) % (int)map_size.height;
    
    int find_map_x = -1;
    int find_map_y = -1;
    int find_w;
    int find_h;
    Vec2 find_tile_pos;
    // 先にY座標を調べる
    for (int h = 0; h < HEIGHT; h++) {
        int _h = h - HEIGHT/2;
        int _x = sx; // yの検索なのでxはどこでもよい
        int _y = (sy + _h + (int)map_size.height) % (int)map_size.height;
            
        auto tile = this->getChildByTag(this->_get_tile_tag(_x, _y));
        auto tile_pos = tile->getPosition();
        if ((tile_pos.y - tile_size.height/2 <= touch_pos.y) && (touch_pos.y < tile_pos.y + tile_size.height/2)) {
            find_map_y = _y;
            find_h = h;
        }
    }
    
    // 見つからなかった
    if (find_map_y == -1) {
        CCLOG("not found y...");
        return true;
    }
    
    // 次にX座標で走査する
    for (int w = 0; w < WIDTH; w++) {
        int _w = w - WIDTH/2;
        int _x = (sx + _w + (int)map_size.width)  % (int)map_size.width;
        int _y = find_map_y;
        
        auto tile = this->getChildByTag(this->_get_tile_tag(_x, _y));
        auto tile_pos = tile->getPosition();
        if ((tile_pos.x - tile_size.width/2 <= touch_pos.x) && (touch_pos.x < tile_pos.x + tile_size.width/2)) {
            find_map_x = _x;
            find_w = w;
            find_tile_pos = tile_pos;
        }
    }
    
    // 見つからなかった
    if (find_map_x == -1) {
        CCLOG("not found x...");
        return true;
    }
    else {
        CCLOG("found !! (%d, %d) ", find_map_x, find_map_y);
    }
    
    
    // タッチしたところが現在地なら何もしない
    if (find_map_x == sx && find_map_y == sy) {
        return true;
    }
    // 移動距離が１の場合検索する
    int distance_x = abs(find_map_x - this->now_pos_x);
    int distance_y = abs(find_map_y - this->now_pos_y);
    if ((distance_x + distance_y) == 1) {
        if (this->_check_near_player(find_map_x, find_map_y)) {
            return true;
        }
    }
    
    // ルートを検索して、可能なら移動する
    auto route_search = RouteSearch::getInstance();
    auto find_routes = route_search->search_route(sx, sy, find_w, find_h, _tile_map);
    
    // 探索結果なし
    if (find_routes.empty()) {
        return true;
    }
    
    this->goal_x = find_map_x;
    this->goal_y = find_map_y;
    
    // popして使いたいので、反転しておく
    std::reverse(find_routes.begin(), find_routes.end());
    this->routes.clear();
    this->routes.shrink_to_fit();
    this->routes = find_routes;
    
    // タッチ位置を表示
    auto touch_rect = this->getChildByTag(TAG_TOUCH_POINT);
    touch_rect->setPosition(find_tile_pos);
    
    // 移動中でなければ、最初の移動分を設定しておく
    if (this->now_route == ROUTE_NONE) {
        this->now_route = this->routes.back();
        this->routes.pop_back();
    }
    
    return true;
}

void MapScene::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event) {
}

void MapScene::onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event) {
}

void MapScene::onTouchCancelled(cocos2d::Touch *touch, cocos2d::Event *unused_event){
}

//---------------------------------------------------------
// マップ切り替え情報を取得
//---------------------------------------------------------
void MapScene::_init_jump_info() {
    auto tile_map = this->_get_map();
    auto layer = tile_map->getLayer(MAP_LAYER_TYPE_MAP);
    auto jump_size = layer->getProperty("jump_num").asInt();
    for (int i = 0; i < jump_size; i++) {
        std::string key = "jump_" + std::to_string(i+1);
        auto info_str = layer->getProperty(key).asString();
        // JSONで情報取得
        Json *json = Json_create(info_str.c_str());
        // 起点
        int _x = Json_getInt(json, "x", -1);
        int _y = Json_getInt(json, "y", -1);
        // 飛び先
        std::string _move_map_name = Json_getString(json, "move", "");
        int _move_x = Json_getInt(json, "move_x", -1);
        int _move_y = Json_getInt(json, "move_y", -1);
        
        // check
        std::vector<int> checks = {_x, _y, _move_x, _move_y};
        for (auto check : checks) {
            assert(check > -1);
        }
        assert(_move_map_name.size() > 0);
        
        CCLOG("info %d,%d  %d,%d %s", _x, _y, _move_x, _move_y, _move_map_name.c_str());
        
        // push
        jump_info_t jump_info = {_x, _y, _move_x, _move_y, _move_map_name};
        this->jump_infos.push_back(jump_info);
        std::string map_key = this->_get_jump_info_map_key(_x, _y);
        this->jump_info_map[map_key] = 1;
    }
}

std::string MapScene::_get_jump_info_map_key(int x, int y) {
    return std::to_string(x) + "_" + std::to_string(y);
}



//---------------------------------------------------------
// 表示するマップの初期化
//---------------------------------------------------------
void MapScene::_init_map(std::string tmx_name) {
    auto layer_size = this->getContentSize();
    
    auto _tile_map = TMXTiledMap::create(tmx_name + ".tmx");
    // 元は非表示にする
    _tile_map->setVisible(false);
    _tile_map->setTag(TAG_MAP);
    this->addChild(_tile_map);
    
    Vector<Node*> array = _tile_map->getChildren();
    SpriteBatchNode *tileChild = NULL;
    
    for (int i = 0; i < array.size(); i++){
        tileChild = (SpriteBatchNode*)array.at(i);
        if (!tileChild) {
            break;
        }
        tileChild->setVisible(false);
        tileChild->getTexture()->setAntiAliasTexParameters();
    }
    
    auto map_size  = _tile_map->getMapSize();
    auto map_layer = _tile_map->getLayer(MAP_LAYER_TYPE_MAP);
    
    auto tile_size = _tile_map->getTileSize();
    
    // とりあえず中心にして
    auto center_pos = Vec2(layer_size.width/2, layer_size.height/2);
    
    // ループさせて、設定しておく
    for (int h = 0; h < HEIGHT; h++) {
        for (int w = 0; w < WIDTH; w++) {
            // 設定されているやつをとばす
            
            int _w = w - WIDTH/2;
            int _h = h - HEIGHT/2;
            int _x = (this->now_pos_x + _w + (int)map_size.width)  % (int)map_size.width;
            int _y = (this->now_pos_y + _h + (int)map_size.height) % (int)map_size.height;
            
            auto tile = this->_make_tile(_x, _y, map_layer);
            
            // 指定の座標にオフセット
            int sx =  _w * tile_size.width;
            int sy = -_h * tile_size.height;
            
            auto _pos = Vec2(center_pos.x + sx, center_pos.y + sy);
            tile->setPosition(_pos);
            this->addChild(tile, GS_MAP);
            
            auto tag = this->_get_tile_tag(_x, _y);
            tile->setTag(tag);
            this->disp_tile_tags.push_back(tag);
        }
    }
    
    // 帯をつける
    auto dot_rect = Rect(0, 0, layer_size.width, tile_size.height);
    std::vector<int> edges = {-(HEIGHT/2+1), HEIGHT/2+1};
    for (auto y : edges) {
        auto dot = Sprite::create();
        dot->setTextureRect(dot_rect);
        dot->setColor(Color3B::GREEN);
        dot->setPosition(layer_size.width/2, layer_size.height/2 + y * tile_size.height);
        this->addChild(dot, GS_MAP_EDGE_HIDE);
    }
}

//---------------------------------------------------------
// タイルのスプライトを作成する
//---------------------------------------------------------
Sprite* MapScene::_make_tile(int x, int y, TMXLayer *layer) {
    auto gid      = layer->getTileGIDAt(Vec2(x, y));
    auto tile_set = layer->getTileSet();
    
    Rect rect = tile_set->getRectForGID(gid);
    rect = CC_RECT_PIXELS_TO_POINTS(rect);
    
    auto tile = Sprite::createWithTexture(layer->getTexture(), rect);
    tile->setBatchNode(layer);
    return tile;
}

//---------------------------------------------------------
// xy座標から規則そってタグを取得する
//---------------------------------------------------------
uint32_t MapScene::_get_tile_tag(int x, int y) {
    auto tag = y * 10000 + x;
    return tag;
}

//---------------------------------------------------------
// 全体マップ情報を取得
//---------------------------------------------------------
TMXTiledMap* MapScene::_get_map() {
    auto tile_map = (TMXTiledMap *)this->getChildByTag(TAG_MAP);
    return tile_map;
}
