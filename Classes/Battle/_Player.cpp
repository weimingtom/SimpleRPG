//
//  Player.cpp
//  Choukyu
//
//  Created by shamaton.
//
//

#include "_Player.h"
#include "../Common.h"

USING_NS_CC;

enum {
    TAG_SWORD,
    TAG_ANIMATION_DAMAGE
};

//---------------------------------------------------------
// 初期化
//---------------------------------------------------------
_Player* _Player::create()
{
    auto _player = new _Player();
    if (_player && _player->initWithSpriteFrameName("player_stand.png"))
    {
        _player->_init();
        _player->autorelease();
        _player->retain();
        return _player;
    }
    
    CC_SAFE_DELETE(_player);
    return NULL;
}

void _Player::_init() {
    Size visibleSize = Director::getInstance()->getVisibleSize();
	default_pos = Vec2(visibleSize.width - 70,
					   visibleSize.height - 200
					   );
	setPosition(Vec2(visibleSize.width + 200 ,default_pos.y));
    
    setScale(2.0f);
    
    auto sword = Sprite::create(RES_BATTLE_DIR + "sword.png");
    sword->setVisible(false);
    sword->setPosition(Vec2(30.0f, 35.0f));
    sword->setTag(TAG_SWORD);
    addChild(sword);
}

//---------------------------------------------------------
// 開始時のアニメーション
//---------------------------------------------------------
void _Player::start_animation() {
	// 所定の位置へ
	auto move = MoveTo::create(0.5f, default_pos);
	auto seq = Sequence::create(move, nullptr);
	runAction(seq);
}

//---------------------------------------------------------
// 攻撃時のアニメーション
//---------------------------------------------------------
void _Player::attack_animation() {
	
	setSpriteFrame("player_left_atk.png");
	auto sword = getChildByTag(TAG_SWORD);
	sword->setRotationSkewY(0.0f);
	sword->setPosition(Vec2(30.0f, 35.0f));
	sword->setVisible(true);
	
	// 静止する
	auto wait_move = MoveTo::create(0.02f * 3, getPosition());
	
    auto sword_switch = CallFunc::create([&]() {
		auto sword = getChildByTag(TAG_SWORD);
		setSpriteFrame("player_right_atk.png");
		sword->setRotationSkewY(180.0f);
		sword->setPosition(Vec2(-10.0f, 22.0f));
    });
	
	auto wait_move2 = MoveTo::create(0.02f * 6, getPosition());
	
    auto sword_invisivle = CallFunc::create([&]() {
		auto sword = getChildByTag(TAG_SWORD);
		sword->setVisible(false);
    });
	
	auto seq = Sequence::create(wait_move, sword_switch, wait_move2, sword_invisivle, nullptr);
	
	// 並列実行
	auto spawn = Spawn::create(seq, nullptr);
	
	runAction(spawn);
}

//---------------------------------------------------------
// 被ダメージのアニメーション
//---------------------------------------------------------
#define DAMAGE_POS 5.0f
void _Player::damage_animation() {
	
	setSpriteFrame("player_damage.png");
	
	
    auto damage_left = CallFunc::create([&]() {
		setPositionX(default_pos.x - DAMAGE_POS);
    });
	auto wait_left = MoveTo::create(0.08f, Vec2(getPositionX() - DAMAGE_POS, getPositionY()));
	
    auto damage_right = CallFunc::create([&]() {
		setPositionX(default_pos.x + DAMAGE_POS);
    });
	auto wait_right = MoveTo::create(0.08f, Vec2(getPositionX() + DAMAGE_POS, getPositionY()));
	
	auto seq = Sequence::create(damage_left,
								wait_left,
								damage_right,
								wait_right,
								damage_left,
								wait_left,
								damage_right,
								wait_right,
								nullptr);
	auto repeat = RepeatForever::create(seq);
	repeat->setTag(TAG_ANIMATION_DAMAGE);
	runAction(repeat);
}

//---------------------------------------------------------
// 死んだ時のアニメーション
//---------------------------------------------------------
void _Player::die_animation(void) {
	// ダメージアニメーション停止
	auto damage_action = getActionByTag(TAG_ANIMATION_DAMAGE);
	stopAction(damage_action);
	// 画像を切替えて、ポジションを戻すだけ
	setSpriteFrame("player_dead.png");
	setPosition(default_pos);
}

//---------------------------------------------------------
// 勝利時のアニメーション
//---------------------------------------------------------
void _Player::win_animation_repeat() {
	
	// 延々つづける
	
	setSpriteFrame("player_win.png");
	setPosition(default_pos);
	
	auto wait = MoveTo::create(0.2f, default_pos);
	
    auto set_stand = CallFunc::create([&]() {
		setSpriteFrame("player_stand.png");
    });
	
    auto set_win = CallFunc::create([&]() {
		setSpriteFrame("player_win.png");
    });
	
	auto seq = Sequence::create(wait,
								set_stand,
								wait,
								set_win,
								nullptr);
	auto repeat = RepeatForever::create(seq);
	runAction(repeat);
}

//---------------------------------------------------------
// 今アニメーション中か
//---------------------------------------------------------
bool _Player::is_now_animation() {
	auto size = getNumberOfRunningActions();
	return size > 0;
}

//---------------------------------------------------------
// いるべき位置
//---------------------------------------------------------
Vec2 _Player::get_default_position() {
	return default_pos;
}
