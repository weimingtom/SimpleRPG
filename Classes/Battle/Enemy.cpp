//
//  Enemy.cpp
//  Choukyu
//
//  Created by shamaton.
//
//

#include "Enemy.h"

#include "Common.h"
#include "Define.h"
#include "DamageText.h"
#include "../GameManager.h"

USING_NS_CC;

// 攻撃時のセリフ
const std::string attack_serifs[MAX_LEVEL] = {
	"キシャァァァン！！",
	"ふふっ、そんなものですか？",
	"ぬるい！返り討ちにしてくれる！",
	"甘い、死へといざなおう",
	"ものたりないな・・"
};

// やられる時のセリフ
const std::string dead_serifs[MAX_LEVEL] = {
	"グワァァァン！！",
	"な、なんと・・・",
	"お見事・・！",
	"まさかお前が・・！",
	"大した努力だ・・！"
};

// 特別なセリフ
const std::string special_serif = "お主こそ最強・・！";

//---------------------------------------------------------
// 初期化
//---------------------------------------------------------
bool Enemy::init(int level) {
	if (!Sprite::init()) {
		return false;
	}
	// init
	this->is_attack_animation_end = false;
	this->level = level;
	
	std::string str = "lv" + std::to_string(level) + ".png";
	this->setTexture(RES_BATTLE_DIR + str);
	
	// 最高レベルだけ通常
	float scale = (level == 5) ? 1.0f : 1.5f;
	this->setScale(scale);
	
	auto game_manager = GameManager::getInstance();
    this->combo_norma   = 10;//game_manager->get_combo_norma();
    this->input_time    = 5.0f;//game_manager->get_input_time() + game_manager->get_extend_input_time();
	this->damaged       = 0;
    this->total_damaged = 100;//game_manager->get_total_damage();
	
    Size visibleSize = Director::getInstance()->getVisibleSize();
	
	this->default_pos = Vec2(0 + 90,
					   visibleSize.height - 200
					   );
	this->setPosition(Vec2(-200, this->default_pos.y));
	
	return true;
}

void Enemy::onEnter() {
	Sprite::onEnter();
}

// UGLY:compile error safe
void Enemy::setDisplayFrameWithAnimationName(const std::string &animationName, ssize_t frameIndex) {
	//Sprite::setDisplayFrameWithAnimationName(animationName, frameIndex);
}

//---------------------------------------------------------
// 入力可能な時間
//---------------------------------------------------------
float Enemy::get_input_enable_time() {
	return this->input_time;
}

//---------------------------------------------------------
// コンボノルマ
//---------------------------------------------------------
const int Enemy::get_combo_limit() {
	return this->combo_norma;
}

bool Enemy::is_combo_norma_achieve(int attack_count) {
	bool is_achieve = false;
	if (attack_count >= this->combo_norma) {
		is_achieve = true;
	}
	return is_achieve;
}

//---------------------------------------------------------
// 撃破関連の処理
//---------------------------------------------------------
bool Enemy::is_defeat(int attack_count) {
	bool is_defeated = false;
	if (attack_count >= this->combo_norma) {
		is_defeated = true;
	}
	// 最終ボスの場合累計ダメージでも倒すことにする
	else if (this->_is_last_boss() && this->total_damaged >= MAX_TOTAL_DAMAGE) {
		is_defeated = true;
	}
	this->is_defeated_enemy = is_defeated;
	return this->is_defeated_enemy;
}

bool Enemy::is_defeated() {
	return this->is_defeated_enemy;
}

//---------------------------------------------------------
// ダメージまわりの処理
//---------------------------------------------------------
int Enemy::add_damage(int attack_value) {
	int damage_range[MAX_LEVEL][2] = {
		{ 100,  100}, //  100 -  200
		{ 200,  300}, //  200 -  500
		{1000, 2000}, // 1000 - 3000
		{2000, 3000}, // 2000 - 5000
		{   0,    1}  // 補正なし
	};
	int index  = level - 1;
	int base   = damage_range[index][0] + attack_value;
	int adjust = (arc4random() * arc4random()) % (damage_range[index][1] + int(attack_value * 1.25f - attack_value));
	int damage = base + adjust;
	if (damage > MAX_ATTACK_VALUE) {
		damage = MAX_ATTACK_VALUE;
	}
	this->damaged += damage;
	// 最終ボスの場合、ダメージを蓄積する
	if (this->_is_last_boss()) {
		this->total_damaged += damage;
	}
	return damage;
}

int Enemy::get_damage(void) {
	return this->damaged;
}

unsigned int Enemy::get_total_damaged(void) {
	return this->total_damaged;
}

//---------------------------------------------------------
// プレー開始時のアニメーション
//---------------------------------------------------------
void Enemy::start_animation(void) {
	
	// 所定の位置へ
	auto move = MoveTo::create(0.5f, default_pos);
	auto seq = Sequence::create(move, nullptr);
	runAction(seq);
}

//---------------------------------------------------------
// 攻撃演出前のアニメーション
//---------------------------------------------------------
void Enemy::pre_attack_animation() {
	// 何も動かない
	auto move = MoveTo::create(0.1f, default_pos);
	// 色を変更
    auto change_color = CallFunc::create([&]() {
		setColor(Color3B::GRAY);
    });
	
	// 色をリセットする
    auto reset_color = CallFunc::create([&]() {
		setColor(Color3B::WHITE);
    });
	// end
    auto set_flag = CallFunc::create([&]() {
		is_attack_animation_end = true;
    });
	
	
	// 直列実行
	auto seq = Sequence::create(
								change_color,
								move,
								reset_color,
								move,
								change_color,
								move,
								reset_color,
								set_flag,
								nullptr);
	
	// 並列実行
	auto spawn = Spawn::create(seq, nullptr);
	runAction(spawn);
	
}

//---------------------------------------------------------
// 死んだ時のアニメーション
//---------------------------------------------------------
void Enemy::dead_animation(void) {
	setColor(Color3B::RED);
	
	auto fade_out = FadeOut::create(0.5f);
	runAction(fade_out);
}

//---------------------------------------------------------
// 今アニメーション中か
//---------------------------------------------------------
bool Enemy::is_now_pre_attack_animation() {
	return getNumberOfRunningActions() > 0;
}

//---------------------------------------------------------
// 敵のいるべき位置
//---------------------------------------------------------
Vec2 Enemy::get_default_pos(void) {
	return default_pos;
}

//---------------------------------------------------------
// 攻撃時、撃破時のセリフ
//---------------------------------------------------------
std::string Enemy::get_attack_serif() {
	auto index = this->level - 1;
	return attack_serifs[index];
}

std::string Enemy::get_dead_serif(int attack_count) {
	auto index = this->level - 1;
	// ラスボスをコンボで倒した時はセリフを変える
	if (this->is_combo_norma_achieve(attack_count) && this->_is_last_boss()) {
		return special_serif;
	}
	return dead_serifs[index];
}

//---------------------------------------------------------
// ラスボスか
//---------------------------------------------------------
bool Enemy::_is_last_boss(void) {
	return this->level == MAX_LEVEL;
}