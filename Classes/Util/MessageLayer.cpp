#include "MessageLayer.h"

#include "extensions/cocos-ext.h"

#include "../Common.h"

USING_NS_CC;
USING_NS_CC_EXT;

#define IS_DEBUG 1

#define kModalLayerPriority -1

// ORDER
enum E_ORDER_LAYER_RESULT {
    ORDER_MESSAGE_WINDOW,
    ORDER_MESSAGE,
	NR_ORDER
};

enum E_TAG_LAYER_RESULT {
    TAG_MESSAGE_WINDOW,
	NR_TAGS
};

// on "init" you need to initialize your instance
bool MessageLayer::init()
{
    // super init
    if ( !Layer::init() )
    {
        return false;
	}
	//if ( !LayerColor::initWithColor(Color4B::GREEN, GAME_RESOLUTION.width, GAME_RESOLUTION.height)) {
	//	return false;
	//}
    //this->setContentSize(GAME_RESOLUTION);
	
	this->tweet_text = "";
	this->update_start = false;
	
	auto layer_size = this->getContentSize();
	auto bg_position = Vec2(layer_size.width/2, layer_size.height/2);
    
    CCLOG("ssziede %f, %f", layer_size.width, layer_size.height);
    
    //this->setPosition(Common::get_layer_position(this->getContentSize(), Vec2(layer_size.width/2, layer_size.height/2)));
	
    
    Size window_size = Size(layer_size.width - 50, 200);
    Scale9Sprite* pScale = Scale9Sprite::create("window.png", Rect(0, 0, 64, 64), Rect(10, 10, 44, 44));
    pScale->setContentSize(window_size);
    //pScale->setPosition(base_position);
    pScale->setTag(TAG_MESSAGE_WINDOW);
    pScale->setPosition(layer_size.width/2, layer_size.height/2);

    this->addChild(pScale, ORDER_MESSAGE_WINDOW);

	this->scheduleUpdate();
	
	// こっちのレイヤーのタッチを優先にしている
    
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = [](Touch *touch,Event*event)->bool{
		return true;
	};
	auto dip = Director::getInstance()->getEventDispatcher();
	dip->addEventListenerWithSceneGraphPriority(listener, this);
	dip->setPriority(listener, kModalLayerPriority);

    return true;
}

void MessageLayer::update(float flame) {
	if (!this->update_start) {
		return;
	}
}

bool MessageLayer::onTouchBegan(Touch *touch, Event *unused_event)
{
	return true;
}

void MessageLayer::onTouchMoved(Touch *touch, Event *unused_event) {
}

void MessageLayer::onTouchEnded(Touch *tounc, Event *unused_event) {
}

void MessageLayer::onTouchCancelled(Touch *tounch, Event *unused_event) {
}


