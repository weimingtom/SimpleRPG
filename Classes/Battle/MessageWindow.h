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

class MessageWindow : public cocos2d::Node
{
public:
    static MessageWindow* create();
	
	void set_disp_timer(float time);
	void set_message(std::string message);
	
	cocos2d::Size get_img_size();
	
private:
    void _init();
};

#endif /* defined(__Choukyu__MessageWindow__) */
