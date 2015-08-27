#ifndef __MAP_MAPSCENE2_H__
#define __MAP_MAPSCENE2_H__

#include "cocos2d.h"

class MapScene : public cocos2d::Layer
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(MapScene);
    
    
    virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    virtual void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    virtual void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    virtual void onTouchCancelled(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    
private:
    
    void _init_map();
    
    virtual void update(float delta);
    void _player_move(); // マップ上を移動する
    
    int now_pos_x;
    int now_pos_y;
    int goal_x;
    int goal_y;
    
    float now_move_amount;
    cocos2d::Vec2 before_move_map_pos;   // マップを動く前の座標
    
    std::vector<int> routes;
    int now_route;
    
    cocos2d::Vec2 _calc_touch_pos(int tx, int ty, cocos2d::TMXTiledMap *map);
    
    
    void _test();
    
    void _update_now_pos_x(int add_x);
    void _update_now_pos_y(int add_y);
    
    void _add_x_line_tile(int fixed_y);
    void _add_y_line_tile(int fixed_x);
    void _remove_x_line_tile(int fixed_y);
    void _remove_y_line_tile(int fixed_x);
    
    cocos2d::Sprite* _make_tile(int x, int y, cocos2d::TMXLayer *layer);
    uint32_t _get_tile_tag(int x, int y);
    cocos2d::Vec2 _get_tile_xy(int tag);
    
    cocos2d::TMXTiledMap* _get_map();
    
    // 表示されているタイルのタグ一覧
    std::vector<int> disp_tile_tags;
};

#endif // __MAP_MAPSCENE2_H__
