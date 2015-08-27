#ifndef __MAP_MAPSCENE_H__
#define __MAP_MAPSCENE_H__

#include "cocos2d.h"

class MapScene : public cocos2d::Layer
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  
    
    // implement the "static create()" method manually
    CREATE_FUNC(MapScene);
    
    
    virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    virtual void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    virtual void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    virtual void onTouchCancelled(cocos2d::Touch *touch, cocos2d::Event *unused_event);
    
private:
    //========================
    // variables
    //========================
    int now_pos_x;  // 現在地 X
    int now_pos_y;  // 現在地 Y
    int goal_x;     // 移動先 X
    int goal_y;     // 移動先 Y
    
    float now_move_amount; // 移動中の制御用
    
    std::vector<int> routes; // 探索結果の道順
    int now_route;           // 移動中の方向
    
    std::vector<int> disp_tile_tags; // 表示されているタイルのタグ一覧
    
    
    //========================
    // functions
    //========================
    void _init_map();
    
    virtual void update(float delta);
    void _player_move();
    
    void _update_now_pos_x(int add_x);
    void _update_now_pos_y(int add_y);
    
    void _add_x_line_tile(int fixed_y);
    void _add_y_line_tile(int fixed_x);
    void _remove_x_line_tile(int fixed_y);
    void _remove_y_line_tile(int fixed_x);
    
    cocos2d::Sprite* _make_tile(int x, int y, cocos2d::TMXLayer *layer);
    uint32_t _get_tile_tag(int x, int y);
    
    cocos2d::TMXTiledMap* _get_map();
    
    //
    void _test();
};

#endif // __MAP_MAPSCENE_H__
