#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

class Game : public cocos2d::Layer
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  
    
    // implement the "static create()" method manually
    CREATE_FUNC(Game);
	
	// single tap
	virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	virtual void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	virtual void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	virtual void onTouchCancelled(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	
	void update(float frame);
	
private:
	int game_step;
	
	cocos2d::Point touch_st; // タッチ開始点

	int question[3];     // タッチすべき場所
	bool touched[9];     // どこをタッチしたか記憶する
	int touched_count;   // タッチパネルに触れた個数
	int answer_progress; // タッチパネルの正解進捗
	int answer_total;    // 正解すべき数
	int disp_number[9];  // ユーザーに表示する数字
	
	bool is_mistaked;           // 失敗した
	bool is_touch_proc_igonre;  // タッチ関連の処理をしない
	bool is_timeout;            // 入力に間に合わなかった
	bool is_player_attack_skip; // プレイヤーの攻撃演出をスキップ
	
	float input_timer;
	
	int attack_count;          // 攻撃演出用カウンター
	int combo_num;             // コンボ数
	int input_count;           // コマンド入力をした回数(成功、失敗両方)
	
	std::vector<int> commands; // 最大で10000
	
	int judge_great_count;
	int added_attack_value;    // プレー結果から増える攻撃力値
	
	int count_down_disp_num;   // カウントダウン表示制御用
	int wait_counter;          // 演出用汎用カウンタ
	
	///////////////////////
	//  game proc
	///////////////////////
	bool _is_collect(int position);
	void _switch_texture();
	int  _get_img_position_by_xy(int x, int y);
	
	void _init_question();
	void _reset_line();
	void _reset_touch_panel_color();
	
	int _get_judge();
	
	///////////////////////
	//  animation
	///////////////////////
	void _attack_animation(int key);
	
	void _damage_animation();
	bool _is_now_damage_animation(void);
	
	void _charge_animation();
	void _charge_stop_animation_and_hint();
	
	void _success_effect(int judge);
	
	void _countdown_animation(int disp_sec);
	
	void _limit_break_num_effect();
	
	cocos2d::MotionStreak* _create_motion_streak();
	
	///////////////////////
	//  game manager & db
	///////////////////////
	void _update_game_manager(int great_rate);
	void _save_play_data();
	
	///////////////////////
	//  util
	///////////////////////
	cocos2d::Rect get_test_rect(int x, int y, cocos2d::Size size);
	
	///////////////////////
	//  game steps
	///////////////////////
    void _update_start(void);
	void _update_input(float flame);
	void _update_player_attack(void);
	void _update_enemy_attack(void);
	void _update_success(void);
	void _update_fail(void);
	void _update_result(void);
    void _update_result_end(void);
	
	// test
	void test_code();
};


#endif // __HELLOWORLD_SCENE_H__
