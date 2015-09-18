#ifndef __GAME_LAYER_RESULT_H__
#define __GAME_LAYER_RESULT_H__

#include "cocos2d.h"

class MessageLayer : public cocos2d::Layer
{
public:
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  
    
    // implement the "static create()" method manually
    CREATE_FUNC(MessageLayer);
	
	// single tap
	virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	virtual void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	virtual void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	virtual void onTouchCancelled(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	
	void update(float frame);
	
	
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
    
    //==========================
    //  メッセージ処理関連
    //==========================
    int message_now_line;
    std::vector<std::string> message_tests;
    
    void _test(cocos2d::Node *sender);
	
};


#endif // __GAME_LAYER_RESULT_H__
