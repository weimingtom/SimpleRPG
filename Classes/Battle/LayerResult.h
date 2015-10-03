#ifndef __GAME_LAYER_RESULT_H__
#define __GAME_LAYER_RESULT_H__

#include "cocos2d.h"

class LayerResult : public cocos2d::LayerColor
{
public:
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  
    
    // implement the "static create()" method manually
    CREATE_FUNC(LayerResult);
	
	// single tap
	virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	virtual void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	virtual void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	virtual void onTouchCancelled(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	
	void next_scene(cocos2d::Ref* pSender);
	void button_tweet(Ref* sender);
	
	void update(float frame);
	
	void set_info(int combo, int damage, int attack, int grate_rate);
	void set_result(bool is_defeated);
	
private:
	int game_step;
	
	float input_timer;
	
	bool update_start;
	int combo;
	int damage;
	int attack;
	int total_damage;
	int great_rate;
	
	int disp_combo;
	int disp_damage;
	int disp_attack;
	int disp_total_damage;
	int disp_grate_rate;
	
	int se_counter;
	int wait_counter; // 演出用汎用カウンタ
	
	std::string tweet_text;
	
	int _get_add_value(int disp_value, int value);
	void _point_up_se();
};


#endif // __GAME_LAYER_RESULT_H__
