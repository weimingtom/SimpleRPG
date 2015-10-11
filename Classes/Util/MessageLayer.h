#ifndef __UTIL_MESSAGE_LAYER_H__
#define __UTIL_MESSAGE_LAYER_H__

#include "cocos2d.h"

#include "../Map/Character.h"

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
    
    void set_message(std::vector<std::string> messages);
    void set_message(std::vector<std::string> messages, Character *character);
	
private:
    
    void _init_yesno();
    
    void _set_touch_enabled(bool enabled);
    void _finalize(cocos2d::Node *sender);
    
    void _read_line();
    void _callback_line(cocos2d::Node *sender);
    void _proc_line(std::string line);
    
    void _set_line(std::string line);
    void _set_br();
    void _set_yesno(std::string line);
    void _set_jump(std::string line);
    void _set_end();
    
    std::map<std::string, int> yesno_line;
    void _push_yes(cocos2d::Ref* pSender);
    void _push_no(cocos2d::Ref* pSender);
    void _push_yesno(std::string yesno);
    
    bool is_disp_br_cursor; // 次の矢印が表示されているか
    bool is_end_line;
    
    int message_now_line;  // 表示に使っているライン(1 - 4)
    int message_now_count; // 表示している行番号
    int message_total_count;
    float message_start_y_pos;
    std::vector<std::string> messages;
    
    std::string ab_text; // a or b
    std::vector<float> fixed_line_y_positions;
    float fixed_text_y_move_amount;
    
    Character *character; // 人に話しかけた場合
    
    void _test();
    
    
    std::string event_line;
    void _callback_event(cocos2d::Node *sender);
    void _proc_message(bool is_init = false);
    void _disp_lines(std::vector<std::string> lines, bool is_init);
    
    void _make_mask();
	
};


#endif // __UTIL_MESSAGE_LAYER_H__
