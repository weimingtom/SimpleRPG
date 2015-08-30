#include "Loading.h"
#include "Common.h"

//#include "../Sound/Sound.h"
//#include "../Sound/MusicFade.h"

#include "../Map/MapScene.h"

USING_NS_CC;


enum E_STEP {
	STEP_INIT,
	STEP_WAIT,
	STEP_END,
	NR_STEP
};

const float WAIT_TIME = 5.0f;

Scene* Loading::createScene()
{
    // 'scene' is an autorelease object
    Scene *scene = CCScene::create();
	
    // 'layer' is an autorelease object
    Loading *layer = Loading::create();
	
    // add layer as a child to scene
    scene->addChild(layer);
	
    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool Loading::init()
{
	// super init
	if ( !Layer::init() )
	{
		return false;
	}
	
	// シングルタップリスナーを用意する
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(_swallowsTouches);
	
	// 各イベントの割り当て
	listener->onTouchBegan     = CC_CALLBACK_2(Loading::onTouchBegan, this);
	listener->onTouchMoved     = CC_CALLBACK_2(Loading::onTouchMoved, this);
	listener->onTouchEnded     = CC_CALLBACK_2(Loading::onTouchEnded, this);
	listener->onTouchCancelled = CC_CALLBACK_2(Loading::onTouchCancelled, this);
	
	// イベントディスパッチャにシングルタップ用リスナーを追加する
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	
	// 毎フレームでupdateを実行させる
	this->schedule(schedule_selector(Loading::update));
	this->scheduleUpdate();
	
	this->_set_text();
	
	//Common::nend_icon_disable();
	
	this->game_step = STEP_INIT;
    return true;
}

//---------------------------------------------------------
// main loop
//---------------------------------------------------------
void Loading::update(float dt) {
	
	switch (this->game_step) {
		case STEP_INIT:
			this->timer++;
			if (this->timer > Common::sec2frame(1.0f)) {
				this->timer = 0;
				//Common::nend_banner_enable();
				//Common::nend_interstitial_enable();
				this->game_step = STEP_WAIT;
			}
			break;
			
		case STEP_WAIT:
			this->timer++;
			if (this->timer > Common::sec2frame(WAIT_TIME)) {
				this->timer = 0;
				this->_next_scene();
				//Common::nend_banner_disable();
				//Common::nend_interstitial_disable();
				this->game_step = STEP_END;
			}
			break;
			
		case STEP_END:
			break;
			
		default:
			break;
	}
}

//---------------------------------------------------------
// touch function
//---------------------------------------------------------
bool Loading::onTouchBegan(Touch *touch, Event *unused_event) {
	return true;
}

void Loading::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event) {
	
}

void Loading::onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event) {
	
}

void Loading::onTouchCancelled(cocos2d::Touch *touch, cocos2d::Event *unused_event){
	
}

//---------------------------------------------------------
// 次のシーンへの遷移
//---------------------------------------------------------
void Loading::_next_scene() {
	
	auto next_scene = MapScene::createScene();
	float duration = 1.0f;
	
	// BGM フェードアウト
	//this->runAction(MusicFade::create(duration/2, BGM_VOLUME_MIN, false));
	
	auto p_scene = TransitionFade::create(duration, next_scene);
	if (p_scene) {
		Director::getInstance()->replaceScene(p_scene);
	}
}

//---------------------------------------------------------
// テキスト設定
//---------------------------------------------------------
void Loading::_set_text() {
	
	auto font_size = 20;
	TTFConfig ttfConfig("fonts/misaki_gothic.ttf",
						font_size,
						GlyphCollection::DYNAMIC);
	
	auto layer_size = this->getContentSize();
	auto text = Label::createWithTTF(ttfConfig, "");
	text->setPosition(layer_size.width/2, layer_size.height - 150.0f);
	
	// 文章を設定
	text->setString("Now Loading");
	
	auto fade_time     = 0.5f;
	auto fade_in  = FadeIn::create(fade_time);
	auto fade_out = FadeOut::create(fade_time);
	auto seq      = Sequence::create(fade_in, fade_out, nullptr);
	text->runAction(RepeatForever::create(seq));
	
	this->addChild(text);
}
