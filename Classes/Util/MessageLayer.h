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
    
    void set_message();
	
private:
    
    //==========================
    //  メッセージ処理関連
    //==========================
    
    void _set_touch_enabled(bool enabled);
    void _finalize();
    
    void _read_line();
    void _callback_line(cocos2d::Node *sender);
    void _proc_line(std::string line);
    
    void _set_line(std::string line);
    void _set_br();
    
    bool is_disp_br_cursor; // 次の矢印が表示されているか
    bool is_end_line;
    
    int message_now_line;
    float message_start_y_pos;
    std::vector<std::string> message_tests;
    
    void _test();
	
};


#endif // __GAME_LAYER_RESULT_H__
