//
//  Enemy.h
//  Choukyu
//
//  Created by shamaton.
//
//

#ifndef __Choukyu__Enemy__
#define __Choukyu__Enemy__


#include "cocos2d.h"
#include "CreateFunc.h"

class Enemy : public cocos2d::Sprite, create_func<Enemy>
{
public:
	virtual bool init(int level); // 初期化処理
	void onEnter() override; // 表示前処理
	using create_func::create;
    
    float get_input_enable_time(); // 入力が可能な時間
	
	bool is_combo_norma_achieve(int attack_count);
	bool is_defeat(int attack_count);
	bool is_defeated();
	const int get_combo_limit();
	
	void start_animation();
	void pre_attack_animation();
	bool is_now_pre_attack_animation();
	
	void dead_animation();
    
    int add_damage(int attack_value);
	int get_damage();
	
	unsigned int get_total_damaged();
	
	cocos2d::Vec2 get_default_pos();
	
	virtual void setDisplayFrameWithAnimationName(const std::string &animationName, ssize_t frameIndex);
	
private:
	cocos2d::Vec2 default_pos;
	bool is_attack_animation_end;
	int level;
	int damaged;       // そのプレーで与えたダメージ
	unsigned int total_damaged; // 最終ボスで与えた
	
	bool is_defeated_enemy; // 倒した
	
	int combo_norma;  // 倒すまでに必要な入力数
	float input_time; // 入力制限時間
	
	int text_count;
	
	bool _is_last_boss(void);
};

#endif /* defined(__Choukyu__Enemy__) */
