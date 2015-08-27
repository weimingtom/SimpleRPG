//
//  RouteSearch.h
//
//  Created by shamaton.
//
//

#ifndef __MAP_ROUTE_SEARCH__
#define __MAP_ROUTE_SEARCH__

#include "cocos2d.h"

enum E_DIRECTON {
    ROUTE_UP,
    ROUTE_LEFT,
    ROUTE_DOWN,
    ROUTE_RIGHT,
    NR_DIRECTION
};

const static int ROUTE_NONE = -1;

class RouteSearch {
private:
    
    // 単独での生成やコピーをさせないため、コンストラクタをprivateへ
    RouteSearch();
    RouteSearch(const RouteSearch& obj);
    RouteSearch& operator=(const RouteSearch& obj);
    
    ~RouteSearch();
    
    void _reset_search_status();
    
    void _set_default(int map_start_x, int map_start_y);
    
    // マンハッタン距離を斜め移動ありを考慮して求める
    int _get_distance(int from_x, int from_y, int to_x, int to_y);
    
    int _back_trace(int x, int y);
    
    // A*で経路探査する
    int _search(int count);
    
    void _trace_route(int x, int y);
    
    // 経路表示
    void _debug_out();
    
    bool is_initial_done;
    
    int goal_x; // 探索用マップ上でのゴール座標
    int goal_y; // 探索用マップ上でのゴール座標
    
    cocos2d::TMXTiledMap *map;
    
    // 結果
    std::vector<int> routes;
    
public:
    // singleton
    static RouteSearch* getInstance(void);
    
    std::vector<int> search_route(int sx, int sy, int gw, int gh, cocos2d::TMXTiledMap *map);
    
    static cocos2d::Vec2 get_matrix(E_DIRECTON direction);
};

#endif /* defined(__MAP_ROUTE_SEARCH2_) */