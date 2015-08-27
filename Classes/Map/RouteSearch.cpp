//
//  RouteSearch.cpp
//
//  Created by shamaton.
//
//

#include "RouteSearch.h"

#include <stdio.h> // for memset

USING_NS_CC;

//---------------------------------------------------------
//
//---------------------------------------------------------
static const int WIDTH  = 15;
static const int HEIGHT = 15;
static const int CENTER_X = 7;
static const int CENTER_Y = 7;

static const int ROUTE_NOT_FOUND = -9;

enum {
    SEARCH_NO_CHECK	= 0,
    SEARCH_OPEN		= 1,
    SEARCH_CLOSE	= 2,
};

typedef struct {
    int x;
    int y;
} POINT;

#define CHECK_MATRIX_SIZE 4
POINT _CheckMatrix[] = {
    {  0,  1 },	// UP
    {  1,  0 },	// LEFT
    {  0, -1 },	// DOWN
    { -1,  0 },	// RIGHT
};

typedef struct
{
    int status;   // -1:障害物 0:可能 1:経路
    
    int cost;
    int SearchStatus; // 0:未調査 1:オープン 2:クローズ
    int parent;       // 親の向き
} mapcell;

mapcell _data[HEIGHT][WIDTH];

//---------------------------------------------------------
//
//---------------------------------------------------------
RouteSearch* RouteSearch::getInstance(void) {
    
    static RouteSearch _instance;
    if (!_instance.is_initial_done) {
        _instance.is_initial_done = true;
    }
    return &_instance;
}

RouteSearch::RouteSearch() {
    
}

RouteSearch::~RouteSearch() {
    
}

//---------------------------------------------------------
// マトリックスを返す
//---------------------------------------------------------
Vec2 RouteSearch::get_matrix(E_DIRECTON direction) {
    auto point = _CheckMatrix[direction];
    return Vec2(point.x, point.y);
}

//-----------------------------------------------------------
// 現在地を中心としたマップ(15x15)を作成し、ルートを検索する
// sx, sy : マップ上での現在地(start)
// gx, gy : マップ上での到達点(goal)
// map    : マップ元情報
//-----------------------------------------------------------
std::vector<int> RouteSearch::search_route(int sx, int sy, int gw, int gh, TMXTiledMap *tile_map) {
    // 検索時に初期化
    this->map = tile_map;
    
    // 現在地を中心とした、到達点の相対座標
    this->goal_x = gw;
    this->goal_y = gh;
    
    memset(_data, 0, sizeof(_data));
    this->routes.clear();
    this->routes.shrink_to_fit();
    
    // 現在地を中心として、周囲の15x15で探索用マップを作成
    this->_set_default(sx, sy);
    // this->_reset_search_status(); // 0クリアで不要かも
    
    // 開始点をOPENにする
    _data[CENTER_X][CENTER_Y].SearchStatus = SEARCH_OPEN;
    
    // 探索
    int ret = this->_search(0);
    
    if (ret == ROUTE_NOT_FOUND) {
        CCLOG("経路は見つかりませんでした\n");
        this->_debug_out();
        return this->routes;
    }
    
    // 結果を取得
    this->_trace_route(this->goal_x, this->goal_y);
    this->_debug_out();
    
    return this->routes;
}

/*************** private method ***************/
//---------------------------------------------------------
// 探索用マップを実マップから作成
//---------------------------------------------------------
void RouteSearch::_set_default(int map_start_x, int map_start_y)
{
    auto collision_layer = this->map->getLayer("Collision");
    auto map_size = this->map->getMapSize();
    
    // ループさせて、設定しておく
    for (int h = 0; h < HEIGHT; h++) {
        for (int w = 0; w < WIDTH; w++) {
            int _w = w - WIDTH/2;
            int _h = h - HEIGHT/2;
            int _x = (map_start_x + _w + (int)map_size.width)  % (int)map_size.width;
            int _y = (map_start_y + _h + (int)map_size.height) % (int)map_size.height;
            // タイルが設定されていれば、-1
            int col_gid = collision_layer->getTileGIDAt(Vec2(_x, _y));
            _data[h][w].status = (col_gid != 0) ? -1 : 0;
        }
    }
    
    return;
}

//---------------------------------------------------------
// マンハッタン距離を斜め移動ありを考慮して求める
//---------------------------------------------------------
int RouteSearch::_get_distance(int from_x, int from_y, int to_x, int to_y)
{
    int cx = from_x - to_x;
    int cy = from_y - to_y;
    
    if (cx < 0) cx *= -1;
    if (cy < 0) cy *= -1;
    
    // 推定移動コストを計算
    return cx + cy;
    
    // 斜め移動あるとき
    if (cx < cy) {
        return (cx + (cy - cx));
    } else {
        return (cy + (cx - cy));
    }
}

//---------------------------------------------------------
// 探索結果をたどってコスト算出
//---------------------------------------------------------
int RouteSearch::_back_trace(int x, int y)
{
    if (x == CENTER_X && y == CENTER_Y) return 1;
    
    int parent_way = _data[y][x].parent;
    
    return this->_back_trace(x + _CheckMatrix[parent_way].x,
                             y + _CheckMatrix[parent_way].y
                             ) + 1;
}

//---------------------------------------------------------
// A*で経路探査する
//---------------------------------------------------------
int RouteSearch::_search(int count){
    
    int cost_min = 9999;
    int BackCost = 0;
    int CX = 0;
    int CY = 0;
    
    mapcell *n = NULL;
    
    // コストが最小のオープンノードを探す
    // TODO: ここを全マス探査しないようにすると、
    //       もっと早くなるかも
    for (int h = 0; h < HEIGHT; h++){
        for (int w = 0; w < WIDTH; w++){
            
            if (_data[h][w].SearchStatus != SEARCH_OPEN) continue;
            if (this->_get_distance(w, h, this->goal_x, this->goal_y) > cost_min) continue;
            
            cost_min = this->_get_distance(w, h, this->goal_x, this->goal_y);
            n = &_data[h][w];
            
            CX = w;
            CY = h;
        }
    }
    
    // オープンノードがなければ終了(ゴールが見つからない)
    if(n == NULL) return ROUTE_NOT_FOUND;
    
    // ノードをクローズする
    n->SearchStatus = SEARCH_CLOSE;
    
    BackCost = this->_back_trace(CX, CY);
    
    for (int i = 0; i < CHECK_MATRIX_SIZE; i++){
        int check_x = CX + _CheckMatrix[i].x;
        int check_y = CY + _CheckMatrix[i].y;
        
        if (check_x < 0) continue;
        if (check_y < 0) continue;
        
        if (check_x >= WIDTH ) continue;
        if (check_y >= HEIGHT) continue;
        
        // 通れないところをよける
        if (_data[check_y][check_x].status == -1) continue;
        
        int estimate_cost = BackCost + this->_get_distance(check_x, check_y, this->goal_x, this->goal_y) + 1;
        
        mapcell *cell = &_data[check_y][check_x];
        
        if (_data[check_y][check_x].SearchStatus == SEARCH_NO_CHECK) {
            
            cell->parent = (i + CHECK_MATRIX_SIZE/2) % CHECK_MATRIX_SIZE;
            cell->SearchStatus = SEARCH_OPEN;
            
        } else if (estimate_cost < cell->cost){
            
            cell->parent       = (i + CHECK_MATRIX_SIZE/2) % CHECK_MATRIX_SIZE;
            cell->cost         = estimate_cost;
            cell->SearchStatus = SEARCH_OPEN;
            
        }
    }
    
    // 見つかったら探索終了
    if (CX == this->goal_x && CY == this->goal_y) {
        return -1;
    }
    
    return this->_search(count + 1);
}

//---------------------------------------------------------
// 探索結果を取得
//---------------------------------------------------------
void RouteSearch::_trace_route(int x, int y)
{
    if(x == CENTER_X && y == CENTER_X){
        // CCLOG("開始ノード>");
        return;
    }
    
    POINT *parent_way = &_CheckMatrix[_data[y][x].parent];
    
    _data[y][x].status = 1;
    
    this->_trace_route(x + parent_way->x, y + parent_way->y);
    
    this->routes.push_back(_data[y][x].parent);
    
    if(x == this->goal_x && y == this->goal_y){
        // CCLOG("ゴール\n");
        return;
    }else{
        // CCLOG("(%d,%d)>", x, y);
    }
    
    return;
}

//---------------------------------------------------------
// 初期化（削除予定）
//---------------------------------------------------------
void RouteSearch::_reset_search_status()
{
    for(int h = 0; h < HEIGHT; h++){
        for(int w = 0; w < WIDTH; w++){
            _data[h][w].cost = 0;
            _data[h][w].SearchStatus = 0;
        }
    }
    return;
}

//---------------------------------------------------------
// 確認表示用
//---------------------------------------------------------
void RouteSearch::_debug_out()
{
    for(int h = 0; h < HEIGHT; h++){
        std::string trace_str;
        for(int w = 0; w < WIDTH; w++){
            std::string str = " ";
            if(w == this->goal_x  && h == this->goal_y) str = "*";
            if(w == CENTER_X      && h == CENTER_Y)     str = "@";
            
            if(_data[h][w].status >  0) str += "o";
            if(_data[h][w].status <  0) str += "#";
            if(_data[h][w].status == 0) str += ".";
            
            trace_str += str;
        }
        CCLOG("%s", trace_str.c_str());
    }
    CCLOG("OK.\n");
    return;
}