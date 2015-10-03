#include "LayerResult.h"

#include "../Common.h"
#include "../Loading/Loading.h"
#include "../Map/MapScene.h"

#include "../Util/Sound/Sound.h"
#include "../Util/Sound/MusicFade.h"

#include "Define.h"

#include "MessageWindow.h"
#include "../GameManager.h"

//#include "../NativeLauncher.h"

USING_NS_CC;

#define IS_DEBUG 1

#define kModalLayerPriority -1

// ORDER
enum E_ORDER_LAYER_RESULT {
	ORDER_BACKGROUND_COLOR,
	ORDER_BACKGROUND_,
	ORDER_LABEL,
	ORDER_OK,
	ORDER_OK_LABEL,
	NR_ORDER
};

enum E_TAG_LAYER_RESULT {
	TAG_LEVEL,
	TAG_COMBO,
	TAG_ATTACK,
	TAG_DAMAGE,
	TAG_TOTAL_DAMAGE,
	TAG_GREAT_RATE,
	NR_TAGS
};

// on "init" you need to initialize your instance
bool LayerResult::init()
{
    // super init
    if ( !Layer::init() )
    {
        return false;
	}
	if ( !LayerColor::initWithColor(Color4B::BLACK, 300, 400)) {
		return false;
	}
	
	this->tweet_text = "";
	this->update_start = false;
	
	auto layer_size = this->getContentSize();
	auto bg_position = Vec2(layer_size.width/2, layer_size.height/2);
	
	// 背景
	auto back_ground    = Sprite::create(RES_BATTLE_DIR + "result_waku.png");
	auto back_ground_bg = Sprite::create(RES_BATTLE_DIR + "result_waku_bg.png");
	
	back_ground->setPosition(bg_position);
	back_ground_bg->setPosition(bg_position);
	
	this->addChild(back_ground,    ORDER_BACKGROUND_);
	this->addChild(back_ground_bg, ORDER_BACKGROUND_COLOR);
	
	// OKボタン
	{
		auto p_credit_item = MenuItemImage::create(RES_BATTLE_DIR + "decide_off.png",
												   RES_BATTLE_DIR + "decide_on.png",
												   CC_CALLBACK_1(LayerResult::next_scene, this));
		auto size = p_credit_item->getContentSize();
		auto position = Vec2(layer_size.width * 1/4, 0.0f);
		p_credit_item->setPosition(position);
		
		auto p_credit = Menu::create(p_credit_item, nullptr);
		p_credit->setPosition(Vec2::ZERO);
		
		this->addChild(p_credit, ORDER_OK);
		
		auto ok_img = Sprite::create(RES_BATTLE_DIR + "ok.png");
		ok_img->setPosition(position);
		auto fade_in  = FadeIn::create(0.5f);
		auto fade_out = FadeOut::create(0.5f);
		auto repeat   = RepeatForever::create(Sequence::create(fade_out, fade_in, nullptr));
		ok_img->runAction(repeat);
		this->addChild(ok_img, ORDER_OK_LABEL);
	}
	
	// twitterボタン
	{
		auto position = Vec2(layer_size.width * 3/4, 0.0f);
		auto p_btn_item = MenuItemImage::create(RES_BATTLE_DIR + "twitter.png",
												RES_BATTLE_DIR + "twitter.png",
												CC_CALLBACK_1(LayerResult::button_tweet, this));
		p_btn_item->setPosition(position);
		
		auto p_btn = Menu::create(p_btn_item, nullptr);
		p_btn->setPosition(Vec2::ZERO);
		
		this->addChild(p_btn, ORDER_OK);
	}

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

void LayerResult::update(float flame) {
	if (!this->update_start) {
		return;
	}
	
	bool is_updated = false;
	
	// コンボ
	if (!is_updated && this->disp_combo != this->combo) {
		this->disp_combo += this->_get_add_value(this->disp_combo, this->combo);
		auto text = (Label *)this->getChildByTag(TAG_COMBO);
		text->setString(std::to_string(this->disp_combo) + " 回斬!!");
		is_updated = true;
	}
	else if (this->combo == 0) {
		auto text = (Label *)this->getChildByTag(TAG_COMBO);
		text->setString("0 回斬!!");
	}
	
	// ダメージ
	if (!is_updated && this->disp_damage != this->damage) {
		this->disp_damage += this->_get_add_value(this->disp_damage, this->damage);
		auto text = (Label *)this->getChildByTag(TAG_DAMAGE);
		text->setString(std::to_string(this->disp_damage) + " ダメージ!!");
		is_updated = true;
	}
	else if (this->damage == 0) {
		auto text = (Label *)this->getChildByTag(TAG_DAMAGE);
		text->setString("0 ダメージ!!");
	}
	
	// Great率
	if (!is_updated && this->disp_grate_rate != this->great_rate) {
		this->disp_grate_rate += this->_get_add_value(this->disp_grate_rate, this->great_rate);
		auto text = (Label *)this->getChildByTag(TAG_GREAT_RATE);
		text->setString(std::to_string(this->disp_grate_rate) + " % GREAT");
		is_updated = true;
	}
	else if (this->great_rate == 0) {
		auto text = (Label *)this->getChildByTag(TAG_GREAT_RATE);
		text->setString("0% GREAT");
	}
	
	// 攻撃力
	if (!is_updated && this->disp_attack != this->attack) {
		this->disp_attack += this->_get_add_value(this->disp_attack, this->attack);
		auto text = (Label *)this->getChildByTag(TAG_ATTACK);
		text->setString(std::to_string(this->disp_attack) + " 攻撃力UP!!");
		is_updated = true;
	}
	else if (this->attack == 0) {
		auto text = (Label *)this->getChildByTag(TAG_ATTACK);
		text->setString("0 攻撃力UP!!");
	}
	
	// 累計
	if (!is_updated && this->disp_total_damage != this->total_damage) {
		this->disp_total_damage += this->_get_add_value(this->disp_total_damage, this->total_damage);
		auto text = (Label *)this->getChildByTag(TAG_TOTAL_DAMAGE);
		text->setString(std::to_string(this->disp_total_damage) + " ダメ蓄積");
		is_updated = true;
	}
	else if (this->total_damage == 0) {
		auto text = (Label *)this->getChildByTag(TAG_TOTAL_DAMAGE);
		text->setString("0 ダメ蓄積");
	}
	
	if (is_updated) {
		this->_point_up_se();
	}
}

int LayerResult::_get_add_value(int disp_value, int value) {
	
	// 一致しているなら何もしない
	if (disp_value == value) {
		return 0;
	}
	
	int divide = FRAME/2;
	int add_value = value/divide;
	
	// 割って0の場合1足す
	if (add_value == 0) {
		return 1;
	}
	// 残りがすくない場合、足りない分を足す
	if (add_value > (value - disp_value)) {
		return (value - disp_value);
	}
	return add_value;
}

void LayerResult::set_result(bool is_defeated) {
	auto layer_size = this->getContentSize();
	
	std::string file_name = is_defeated ? "win.png" : "lose.png";
	auto result = Sprite::create(RES_BATTLE_DIR + file_name);
	result->setPosition(layer_size.width/2, layer_size.height - result->getContentSize().height*2);
	this->addChild(result, ORDER_LABEL);
	
	// tweet text
	if (is_defeated) {
		auto gm = GameManager::getInstance();
		this->tweet_text = "hoge";//this->tweet_text + gm->get_selected_level_str() + "を達成！！";
	}
}

void LayerResult::set_info(int combo, int damage, int attack, int grate_rate) {
	auto layer_size = getContentSize();
	auto game_manager = GameManager::getInstance();
	
	this->combo        = combo;
	this->damage       = damage;
	this->attack       = attack;
	this->great_rate   = grate_rate;
    this->total_damage = 12345;//game_manager->get_total_damage();
	
	auto font_size = 20;
	
    TTFConfig ttfConfig("fonts/misaki_gothic.ttf",
                        font_size,
                        GlyphCollection::DYNAMIC);
	
	auto move_y = 50.0f;
	auto adjust_y = move_y * 2;
	{
        std::string lv_str = "jhoe";//game_manager->get_selected_level_str();
		auto lv_label = Label::createWithTTF(ttfConfig, lv_str);
		lv_label->setPosition(layer_size.width/2, layer_size.height - adjust_y);
		this->addChild(lv_label, ORDER_LABEL, TAG_LEVEL);
	}
	adjust_y += move_y;
	{
		auto label = Label::createWithTTF(ttfConfig, "");
		label->setPosition(layer_size.width/2, layer_size.height - adjust_y);
		this->addChild(label, ORDER_LABEL, TAG_COMBO);
	}
	adjust_y += move_y;
	{
		auto label2 = Label::createWithTTF(ttfConfig, "");
		label2->setPosition(layer_size.width/2, layer_size.height - adjust_y);
		this->addChild(label2, ORDER_LABEL, TAG_DAMAGE);
	}
	adjust_y += move_y;
	{
		auto label = Label::createWithTTF(ttfConfig, "");
		label->setPosition(layer_size.width/2, layer_size.height - adjust_y);
		this->addChild(label, ORDER_LABEL, TAG_GREAT_RATE);
	}
	adjust_y += move_y;
	{
		auto label2 = Label::createWithTTF(ttfConfig, "");
		label2->setPosition(layer_size.width/2, layer_size.height - adjust_y);
		this->addChild(label2, ORDER_LABEL, TAG_ATTACK);
	}
	adjust_y += move_y;
	{
		auto label = Label::createWithTTF(ttfConfig, "");
		label->setPosition(layer_size.width/2, layer_size.height - adjust_y);
		this->addChild(label, ORDER_LABEL, TAG_TOTAL_DAMAGE);
		//label->setVisible(game_manager->get_selected_level() == MAX_LEVEL);
	}
	
	this->update_start = true;
}


void LayerResult::next_scene(Ref* pSender) {
	auto gm = GameManager::getInstance();
	auto next_scene = (true) ? MapScene::createScene() : Loading::createScene();
	float duration = 1.0f;
	
	play_se("ok.wav");
	
	Scene *p_scene = TransitionFade::create(duration, next_scene);
	if (p_scene) {
		Director::getInstance()->replaceScene(p_scene);
	}
}


void LayerResult::button_tweet(Ref *sender) {
	char send_text[128];
	std::string tweet = std::to_string(this->combo) + "コンボ、"+ std::to_string(this->damage) + "ダメージを記録！" + this->tweet_text;
	sprintf(send_text, "%s #決めろ武神覇斬 #shamaton", tweet.c_str());
	//NativeLauncher::openTweetDialog(send_text);
}

bool LayerResult::onTouchBegan(Touch *touch, Event *unused_event)
{
	return true;
}

void LayerResult::onTouchMoved(Touch *touch, Event *unused_event) {
}

void LayerResult::onTouchEnded(Touch *tounc, Event *unused_event) {
}

void LayerResult::onTouchCancelled(Touch *tounch, Event *unused_event) {
}

void LayerResult::_point_up_se() {
	this->se_counter++;
	if (this->se_counter > Common::sec2frame(0.1f)) {
		this->se_counter = 0;
		play_se(RES_BATTLE_DIR + "point_up.wav");
	}
}

