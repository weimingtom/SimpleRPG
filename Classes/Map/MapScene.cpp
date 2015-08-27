#include "MapScene.h"

#include "RouteSearch.h"

USING_NS_CC;

enum E_TAG {
    //
    // (0, 0) - (1024, 1024) までタイル用に予約する
    //
    TAG_NONE = 11000000,
    TAG_MAP,
    TAG_PLAYER,
    TAG_TOUCH_POINT,
    NR_TAGS
};

enum E_GS_ORDER {
    GS_NONE,
    GS_MAP,
    GS_PLAYER,
    GS_TOUCH_POINT,
    NR_GSS
};

const float MOVE_SPEED = 1.0f;

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
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(MapScene::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
    auto label = LabelTTF::create("Hello World", "Arial", 24);
    
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer
    this->addChild(sprite, 0);
    
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
    
    // プレーヤー
    auto frame_cache = SpriteFrameCache::getInstance();
    frame_cache->removeSpriteFrames();
    frame_cache->addSpriteFramesWithFile("player.plist");
    
    auto animation_cache = AnimationCache::getInstance();
    //animation_cache->destroyInstance();
    animation_cache->addAnimationsWithFile("player_animations.plist");
    
    auto player = Sprite::createWithSpriteFrameName("player_front_01.png");
    player->setPosition(visibleSize.width/2, visibleSize.height/2);
    player->setTag(TAG_PLAYER);
    this->addChild(player, GS_PLAYER);
    
    auto animation = animation_cache->getAnimation("walk_front");
    auto action = Animate::create(animation);
    auto action_req = RepeatForever::create(action);
    action_req->setTag(999);
    player->runAction(action_req);
    
    // タッチしたとこ
    auto dot_rect = Rect(0, 0, 32, 32);
    auto dot = Sprite::create();
    dot->setTextureRect(dot_rect);
    dot->setColor(Color3B::RED);
    dot->setOpacity(128);
    dot->setTag(TAG_TOUCH_POINT);
    this->addChild(dot, GS_TOUCH_POINT);
    
    // 開始時の位置設定
    this->now_pos_x = 8;
    this->now_pos_y = 8;
    this->_init_map();
    
    // test
    this->_test();
    
    // 毎フレームでupdateを実行させる
    this->schedule(schedule_selector(MapScene::update));
    this->scheduleUpdate();
    
    return true;
}

void MapScene::_test() {
}


static const int WIDTH  = 15;
static const int HEIGHT = 15;
void MapScene::_init_map() {
    auto layer_size = this->getContentSize();
    
    auto _tile_map = TMXTiledMap::create("test.tmx");
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
    auto map_layer = _tile_map->getLayer("BG");
    
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
}


Sprite* MapScene::_make_tile(int x, int y, TMXLayer *layer) {
    
    auto gid      = layer->getTileGIDAt(Vec2(x, y));
    auto tile_set = layer->getTileSet();
   
    Rect rect = tile_set->getRectForGID(gid);
    rect = CC_RECT_PIXELS_TO_POINTS(rect);
    
    auto tile = Sprite::createWithTexture(layer->getTexture(), rect);
    tile->setBatchNode(layer);
    return tile;
}

uint32_t MapScene::_get_tile_tag(int x, int y) {
    auto tag = y * 10000 + x;
    return tag;
}

//---------------------------------------------------------
// main loop
//---------------------------------------------------------
void MapScene::update(float delta) {
    // 移動
    if (this->now_route != ROUTE_NONE) {
        this->_player_move();
    }
}

//---------------------------------------------------------
// マップ上を移動する
//---------------------------------------------------------
void MapScene::_player_move() {
    
    // 移動途中でのクリックは移動先の考慮が必要。メモ
    
    // 歩き終わったか
    bool is_move_end = false;
    
    // 歩く
    auto _tile_map = this->_get_map();
    auto tile_size = _tile_map->getTileSize();
    auto map_size  = _tile_map->getMapSize();
    
    auto _touch_rect = this->getChildByTag(TAG_TOUCH_POINT);
    
    Vec2 new_touch_pos = _touch_rect->getPosition();
    
    auto goal_tag = this->_get_tile_tag(this->goal_x, this->goal_y);
    switch (this->now_route) {
        case ROUTE_UP:
        {
            // 移動開始時にマップを追加
            if (this->now_move_amount == 0) {
                this->_add_x_line_tile(-(HEIGHT/2 + 1));
            }
            
            // マップを移動
            for (auto tag : this->disp_tile_tags) {
                auto tile = this->getChildByTag(tag);
                tile->setPositionY(tile->getPositionY() - MOVE_SPEED);
                
                // タッチポイントに合致する場合
                if (tag == goal_tag) {
                    new_touch_pos = tile->getPosition();
                }
            }
            
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
            }
            
            // マップを移動
            for (auto tag : this->disp_tile_tags) {
                auto tile = this->getChildByTag(tag);
                tile->setPositionY(tile->getPositionY() + MOVE_SPEED);
                // タッチポイントに合致する場合
                if (tag == goal_tag) {
                    new_touch_pos = tile->getPosition();
                }
            }
            
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
            }
            
            // マップを移動
            for (auto tag : this->disp_tile_tags) {
                auto tile = this->getChildByTag(tag);
                tile->setPositionX(tile->getPositionX() + MOVE_SPEED);
                // タッチポイントに合致する場合
                if (tag == goal_tag) {
                    new_touch_pos = tile->getPosition();
                }
            }
            
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
            }
            
            // マップを移動
            for (auto tag : this->disp_tile_tags) {
                auto tile = this->getChildByTag(tag);
                tile->setPositionX(tile->getPositionX() - MOVE_SPEED);
                // タッチポイントに合致する場合
                if (tag == goal_tag) {
                    new_touch_pos = tile->getPosition();
                }
            }
            
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
    _touch_rect->setPosition(new_touch_pos);
    
    // 歩き終わってまだroutesがある場合、再度セットする
    if (is_move_end) {
        this->now_route = ROUTE_NONE;
        this->now_move_amount = 0;
        if (this->routes.size()) {
            this->now_route = this->routes.back();
            this->routes.pop_back();
        } else {
            // タッチを消す
        }
    }
}

void MapScene::_add_x_line_tile(int fixed_y) {
    auto _tile_map  = this->_get_map();
    auto _map_layer = _tile_map->getLayer("BG");
    
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


void MapScene::_add_y_line_tile(int fixed_x) {
    auto _tile_map  = this->_get_map();
    auto _map_layer = _tile_map->getLayer("BG");
    
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

//=========================================================
// touch function
//=========================================================
//---------------------------------------------------------
//
//---------------------------------------------------------
bool MapScene::onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event)
{
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



void MapScene::menuCloseCallback(Ref* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

TMXTiledMap* MapScene::_get_map() {
    auto tile_map = (TMXTiledMap *)this->getChildByTag(TAG_MAP);
    return tile_map;
}
