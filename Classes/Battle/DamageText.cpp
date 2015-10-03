//
//  DamageText.cpp
//  Choukyu
//
//  Created by shamaton.
//
//


#include "DamageText.h"
#include "Define.h"

USING_NS_CC;

#define FONT_SIZE 30

bool DamageText::init(Vec2 position) {
	if (!Label::init()) {
		return false;
	}
	init_position = position;
	setPosition(init_position);
	setSystemFontSize(FONT_SIZE);
	//setSystemFontName("Arial");
	
	TTFConfig ttfConfig("fonts/misaki_gothic.ttf",
                        FONT_SIZE,
                        GlyphCollection::DYNAMIC);
	setTTFConfig(ttfConfig);
	return true;
}

void DamageText::onEnter() {
	Label::onEnter();
}


void DamageText::do_animation(int damage) {
	std::string text = std::to_string(damage);
	setString(text);
	
	this->stopAllActions();
	this->setPosition(this->init_position);
	
	auto after_position = Vec2(getPositionX(), getPositionY() +  50.0f);
	
	auto move1 = MoveTo::create(0.5f, after_position);
	auto ease_move = EaseIn::create(move1, 2.5f);
	
	auto move2 = MoveTo::create(1.0f, after_position);
	
	auto func = CallFunc::create([&]() {
		setString("");
		setPosition(this->init_position);
	});
	// 直列実行
	auto seq = Sequence::create(ease_move, move2, func, nullptr);
	
	// 並列実行
	auto spawn = Spawn::create(seq, nullptr);
	
	runAction(spawn);
}

void DamageText::do_player_side_animation(int damage) {
	std::string text = std::to_string(damage);
	setString(text);
	
	// 跳ねるようなアクション
	auto action = MoveTo::create(0.5f, Vec2(getPositionX(), getPositionY() - 30.0f));
	auto easeAction = EaseBounceOut::create(action);
	
	auto func = CallFunc::create([&]() {
		setString("");
		setPosition(this->init_position);
	});
	
	auto seq = Sequence::create(easeAction, func, nullptr);
	runAction(seq);
}

bool DamageText::is_now_animating(void) {
    auto size = getNumberOfRunningActions();
    return size > 0;
}
