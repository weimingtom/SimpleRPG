//
//  MessageWindow.h
//  Choukyu
//
//  Created by shamaton.
//
//

#ifndef __Choukyu__MessageWindow__
#define __Choukyu__MessageWindow__

#include "cocos2d.h"
#include "CreateFunc.h"

class MessageWindow : public cocos2d::Node, create_func<MessageWindow>
{
public:
	virtual bool init(); // 初期化処理
	void onEnter() override; // 表示前処理
	using create_func::create;
	
	void set_disp_timer(float time);
	void set_message(std::string message);
	
	cocos2d::Size get_img_size();
	
private:
};

#endif /* defined(__Choukyu__MessageWindow__) */
