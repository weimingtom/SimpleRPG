#ifndef __LOADING_H__
#define __LOADING_H__

#include "cocos2d.h"

class Loading : public cocos2d::Layer
{
	
private:
	int game_step;
	int timer;
	
	void _set_text();
	
	void _next_scene();
	
public:
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();
	
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	
	// implement the "static create()" method manually
	CREATE_FUNC(Loading);
	
	// single tap
	virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	virtual void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	virtual void onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	virtual void onTouchCancelled(cocos2d::Touch *touch, cocos2d::Event *unused_event);
	
	// for update
	void update(float dt);
};

#endif // __LOADING_H__
