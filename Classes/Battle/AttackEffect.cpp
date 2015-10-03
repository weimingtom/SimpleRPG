//
//  DamageText.cpp
//
//  Created by shamaton.
//
//


#include "AttackEffect.h"

#include "GameManager.h"

USING_NS_CC;

enum TAGS {
	TAG_ENEMY_ATTACK = 1,
	NR_TAGS
};



bool AttackEffect::init(Vec2 player_pos, Vec2 enemy_pos) {
	if (!Layer::init()) {
		return false;
	}
	
	auto game_manager = (GameManager *)GameManager::getInstance();
	
	// 攻撃アニメーションの読み込み
	/*
	std::string attack_img_filename = "lv" + std::to_string(1) + "_attack.plist";
	auto cache = SpriteFrameCache::getInstance();
	cache->addSpriteFramesWithFile(attack_img_filename);
	
	std::string attack_anime_filename = "lv" + std::to_string(1) + "_attack_animations.plist";
	auto animation_cache = AnimationCache::getInstance();
	animation_cache->addAnimationsWithFile(attack_anime_filename);
	 */
	
	auto enemy_attack = Sprite::createWithSpriteFrameName("enemy_attack01.png");
	enemy_attack->setTag(TAG_ENEMY_ATTACK);
	//enemy_attack->setPosition(enemy_pos);
	addChild(enemy_attack);

	return true;
}

void AttackEffect::onEnter() {
	Layer::onEnter();
}

void AttackEffect::do_enemy_animation(void) {
	auto attack = (Sprite *)getChildByTag(TAG_ENEMY_ATTACK);
	
    // アニメーションを生成
    auto cache = AnimationCache::getInstance();
    auto animation = cache->getAnimation("enemy_attack_anime");
	
	
    auto action = Animate::create(animation);
    auto action_req = Repeat::create(action, 1);
    action_req->setTag(999);
    attack->runAction(action_req);
}

bool AttackEffect::is_now_enemy_animation(void) {
	auto attack = (Sprite *)getChildByTag(TAG_ENEMY_ATTACK);
	return attack->getNumberOfRunningActions() > 0;
}
