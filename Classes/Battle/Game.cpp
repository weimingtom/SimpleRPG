#include "Game.h"

#include "../Common.h"
#include "../Util/Sound/Sound.h"
#include "../Util/Sound/MusicFade.h"
#include "../Loading/Loading.h"
#include "../Map/MapScene.h"
//#include "../Database/PlayerData.h"
//#include "../Database/EnemyData.h"
#include "Define.h"

#include "_Player.h"
#include "Enemy.h"
#include "DamageText.h"
#include "MessageWindow.h"

#include "../GameManager.h"

// TODO 整理
#include "../Util/MessageLayer.h"

USING_NS_CC;

//---------------------------------------------------------
// definition
//---------------------------------------------------------
#define BUTTON_SPACE 100
#define BUTTON_ADJUST_Y 200

#define LINE_SIZE 2.0f
#define LINE_COLOR  Color4F::YELLOW
#define PANEL_COLOR Color3B::YELLOW

#define TOUCH_SIZE Size(15, 15)

#define GAUGE_SIZE Size(460.0f, 10.0f)

#define IS_DEFEAT_DEBUG 0

enum GAME_STEP {
	STEP_INIT,
	STEP_START,
	STEP_INPUT,
	STEP_SUCCESS,
	STEP_FAIL,
	STEP_RESULT,
    STEP_RESULT_END,
	STEP_END,
	NR_STEP
};

enum E_INPUT_JUDGE {
	JUDGE_GOOD,
	JUDGE_GREAT,
	NR_JUDGE
};

//---------------------------------------------------------
//
//---------------------------------------------------------
Scene* Game::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();

    // 'layer' is an autorelease object
    auto layer = Game::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool Game::init()
{
    // super init
    if ( !Layer::init() )
    {
        return false;
	}
	
	// game manager
	auto game_manager = GameManager::getInstance();

	// アニメーションキャッシュ
	auto frame_cache = SpriteFrameCache::getInstance();
	frame_cache->removeSpriteFrames();
	frame_cache->addSpriteFramesWithFile(RES_BATTLE_DIR + "attack_tate.plist");
	frame_cache->addSpriteFramesWithFile(RES_BATTLE_DIR + "attack_yoko.plist");
	frame_cache->addSpriteFramesWithFile(RES_BATTLE_DIR + "attack_naname.plist");

	frame_cache->addSpriteFramesWithFile(RES_BATTLE_DIR + "player.plist");
	frame_cache->addSpriteFramesWithFile(RES_BATTLE_DIR + "number.plist");

	auto animation_cache = AnimationCache::getInstance();
	animation_cache->destroyInstance();
	animation_cache->addAnimationsWithFile(RES_BATTLE_DIR + "attack_tate_animations.plist");
	animation_cache->addAnimationsWithFile(RES_BATTLE_DIR + "attack_yoko_animations.plist");
	animation_cache->addAnimationsWithFile(RES_BATTLE_DIR + "attack_naname_animations.plist");
	
	// ダメージアニメーションの読み込み
	std::string attack_img_filename = "lv" + std::to_string(1) + "_attack.plist";
	frame_cache->addSpriteFramesWithFile(RES_BATTLE_DIR + attack_img_filename);
	
	std::string attack_anime_filename = "lv" + std::to_string(1) + "_attack_animations.plist";
	animation_cache->addAnimationsWithFile(RES_BATTLE_DIR + attack_anime_filename);
	
	// 入力中エフェクト
	frame_cache->addSpriteFramesWithFile(RES_BATTLE_DIR + "charge.plist");
	animation_cache->addAnimationsWithFile(RES_BATTLE_DIR + "charge_animations.plist");
	
	// シングルタップリスナーを用意する
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(_swallowsTouches);

    // 各イベントの割り当て
	listener->onTouchBegan     = CC_CALLBACK_2(Game::onTouchBegan, this);
	listener->onTouchMoved     = CC_CALLBACK_2(Game::onTouchMoved, this);
	listener->onTouchEnded     = CC_CALLBACK_2(Game::onTouchEnded, this);
	listener->onTouchCancelled = CC_CALLBACK_2(Game::onTouchCancelled, this);

	// イベントディスパッチャにシングルタップ用リスナーを追加する
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

	this->input_count     = 0;
	this->combo_num       = 0;
	this->wait_counter    = 0;
	this->attack_count    = 0;
	this->judge_great_count = 0;
	this->count_down_disp_num = 0;
	
	this->input_timer = .0f;
	
	this->is_touch_proc_igonre  = false;
    
    // 最初はすべてを対象にする
    this->question = -1;
    
    this->test_enemy_hp = 100;
    this->test_player_hp = 100;


	auto player = _Player::create();
	player->setTag(TAG_PLAYER);
	addChild(player, ORDER_CHARACTER);
	
	auto enemy = Enemy::create(1);
	enemy->setTag(TAG_ENEMY);
	addChild(enemy, ORDER_CHARACTER);
	
	// メッセージウインドウ
	auto message_window = MessageWindow::create();
	message_window->setPosition(visibleSize.width/2, visibleSize.height - message_window->get_img_size().height/2);
	message_window->setTag(TAG_MESSAGE_WINDOW);
	message_window->set_message("敵が現れた！！");
	this->addChild(message_window, ORDER_UI);
    
    // メッセージウインドウ
    auto message_layer = MessageLayer::create();
    message_layer->setTag(TAG_MESSAGE_WINDOW_LAYER);
    message_layer->setVisible(false);
    this->addChild(message_layer, ORDER_MESSAGE_WINDOW_LAYER);
	
	// ダメージ表示のテクスチャ作成
	for (int i = 0; i < DAMAGE_TEXT_NUM; i++) {
		auto p = Vec2(enemy->get_default_pos().x, enemy->get_default_pos().y + 20.0f * i - 50.0f);
		auto label = DamageText::create(p);
		label->setTag(TAG_DAMEGE_TEXT + i);
		this->addChild(label, ORDER_DAMAGE_TEXT);
	}

	// 攻撃エフェクト
	auto position = Vec2(visibleSize.width/2 - 150,
						 visibleSize.height/2 + 200);
	const char effect_name[3][32] = {"attack_tate09.png", "attack_yoko09.png", "attack_naname09.png"};
	int effect_tags[3]     = {TAG_ATTACK_EFFECT_TATE, TAG_ATTACK_EFFECT_YOKO, TAG_ATTACK_EFFECT_NANAME};
	for (int i = 0; i < COUNT_OF(effect_name); i++) {
		auto effect = Sprite::createWithSpriteFrameName(effect_name[i]);
		effect->setPosition(enemy->get_default_pos());
		effect->setTag(effect_tags[i]);
		this->addChild(effect, ORDER_EFFECT);
	}
	
	// ダメージエフェクト
	auto damage_effect = Sprite::createWithSpriteFrameName("enemy_attack01.png");
	damage_effect->setTag(TAG_DAMAGE_EFFECT);
	damage_effect->setPosition(player->get_default_position());
	damage_effect->setScale(2.0f);
	damage_effect->setVisible(false);
	this->addChild(damage_effect, ORDER_EFFECT);
	
	// バトル画面背景
	auto background = Sprite::create(RES_BATTLE_DIR + "battle_background.jpg");
	auto b_size = background->getContentSize();
	background->setPosition(Vec2(visibleSize.width/2, visibleSize.height - b_size.height/2));
	background->setScale(visibleSize.width / b_size.width);
	addChild(background, ORDER_BACKGROUND);
	
	// チャージ画像
	auto charge = Sprite::createWithSpriteFrameName("charge01.png");
	charge->setTag(TAG_CHARGE);
	charge->setVisible(false);
	charge->setPosition(player->get_default_position());
	addChild(charge, ORDER_EFFECT);
	
	// 入力タイマー表示
	auto adjust_stamina_y = 60.0f;
	auto stamina_layer = LayerColor::create(Color4B(51, 255, 52, 255), GAUGE_SIZE.width, GAUGE_SIZE.height);
	auto stamina_layer_size = stamina_layer->getContentSize();
	stamina_layer->setAnchorPoint(Vec2(0, 0.5f));
	
	stamina_layer->setPosition((visibleSize.width  - GAUGE_SIZE.width)/2,
							   (visibleSize.height - GAUGE_SIZE.height)/2 + adjust_stamina_y);
	stamina_layer->setTag(TAG_INPUT_GAUGE);
	stamina_layer->setScaleX(1.0f);
	this->addChild(stamina_layer, ORDER_GAUGE);
	
	auto stamina_waku = Sprite::create(RES_BATTLE_DIR + "gauge_waku.png");
	stamina_waku->setPosition(visibleSize.width/2,
							  visibleSize.height/2 + adjust_stamina_y);
	this->addChild(stamina_waku, ORDER_GAUGE_WAKU);
	
	// カウントダウン用
	auto count_down = Sprite::createWithSpriteFrameName("number_0.png");
	count_down->setPosition(visibleSize.width/2, visibleSize.height/2);
	count_down->setVisible(false);
	this->addChild(count_down, ORDER_COUNTDOWN, TAG_COUNTDOWN);
	
	// 発動まで
	{
		auto chain = Sprite::create(RES_BATTLE_DIR + "chain.png");
		chain->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
		chain->setScale(0.75f);
		chain->setPosition(visibleSize.width/2, chain->getContentSize().height * 3 - chain->getContentSize().height/2);
		this->addChild(chain, ORDER_UI, TAG_LIMIT_BREAK);
		
		auto num_scale = 0.5f;
		for (int i = 0; i < 4; i++) {
			auto num_img = Sprite::createWithSpriteFrameName("number_0.png");
			num_img->setVisible(i == 0);
			num_img->setScale(num_scale);
			num_img->setAnchorPoint(Vec2::ANCHOR_MIDDLE_RIGHT);
			num_img->setPosition(visibleSize.width/2 - i * num_img->getContentSize().width * num_scale, chain->getContentSize().height * 3);
			this->addChild(num_img, ORDER_UI, TAG_LIMIT_BREAK_NUM + i);
		}
	}
    
    // 表示用の数字を埋める
    std::vector<int> vector;
    for (int i = 0; i < 9; i++) {
        this->disp_numbers.push_back(i+1);
    }
	
	// 問題の初期化
	this->_init_question();
	
    // タッチパネルの作成
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			int position = this->_get_img_position_by_xy(x, y);
			
			// パネル
			auto sprite = Sprite::create(RES_BATTLE_DIR + "input_button.png");
			auto sprite_vec = Vec2(visibleSize.width/2 + origin.x + BUTTON_SPACE * (x-1),
								   BUTTON_ADJUST_Y + origin.y + (y-1) * BUTTON_SPACE + adjust_stamina_y
								   );
			sprite->setPosition(sprite_vec);
			sprite->setTag(TAG_TOUCH_BUTTON + position);
			this->addChild(sprite, ORDER_TOUCH_PANEL);
			
			// 正解した時のエフェクト用
			auto sprite_effect = Sprite::create(RES_BATTLE_DIR + "input_button.png");
			sprite_effect->setPosition(sprite_vec);
			sprite_effect->setTag(TAG_TOUCH_EFFECT + position);
			sprite_effect->setOpacity(0);
			this->addChild(sprite_effect, ORDER_TOUCH_PANEL);
			
			// 数字を作成
			std::string filename = "number_" + std::to_string(_get_img_position_by_xy(x, y)) + ".png";
			auto number = Sprite::createWithSpriteFrameName(filename);
			number->setTag(TAG_TOUCH_NUMBER + position);
			number->setScale(0.5f);
			number->setPosition(sprite_vec);
			addChild(number, ORDER_TOUCH_NUMBER);
		}
	}
	
	
	//this->test_code();

	this->scheduleUpdate();
	
	game_step = STEP_INIT;

    return true;
}

//---------------------------------------------------------
// main loop
//---------------------------------------------------------
void Game::update(float flame) {
	
    std::string hoge = RES_BATTLE_DIR + "battle.mp3";
	switch (game_step) {
		case STEP_INIT:
			play_bgm(hoge.c_str());
			this->runAction(MusicFade::create(1.0f, BGM_VOLUME_DEFAULT, false));
			this->game_step = STEP_START;
			break;
			
		case STEP_START:
			_update_start();
			break;
			
		case STEP_INPUT:
			_update_input(flame);
			break;
			
		case STEP_SUCCESS:
			_update_success();
			break;
			
		case STEP_FAIL:
			_update_fail();
			break;
			
		case STEP_RESULT:
			_update_result();
			break;
            
        case STEP_RESULT_END:
            _update_result_end();
            break;
			
		case STEP_END:
			break;
			
		default:
			//assert(0);
			break;
	}
}

void Game::test_code() {
}

void Game::_update_start(void) {
	auto player = (_Player *)getChildByTag(TAG_PLAYER);
	auto enemy = (Enemy *)getChildByTag(TAG_ENEMY);
	
	wait_counter++;
	// 飛び出す
	if (wait_counter == Common::sec2frame(0.5f)) {
		player->start_animation();
		enemy->start_animation();
	}
	else if (this->wait_counter > Common::sec2frame(2.0f) && this->wait_counter < Common::sec2frame(4.0f)) {
		auto message = (MessageWindow *)getChildByTag(TAG_MESSAGE_WINDOW);
		message->setVisible(true);
		message->set_message("入力開始まで...");
	}
	else if (this->wait_counter > Common::sec2frame(4.0f) && this->wait_counter < Common::sec2frame(7.0f)) {
		int disp_sec = 7 - this->wait_counter/FRAME; // UGLY : magic number
		
		auto message = (MessageWindow *)getChildByTag(TAG_MESSAGE_WINDOW);
		message->set_message(std::to_string(disp_sec));
		
		// カウントダウンアニメーション
		auto count_down = (Sprite *)this->getChildByTag(TAG_COUNTDOWN);
		bool is_now_animation = (count_down->getNumberOfRunningActions() > 0) ? true : false;
		if (!is_now_animation && this->count_down_disp_num != disp_sec) {
			this->count_down_disp_num = disp_sec;
			this->_countdown_animation(disp_sec);
		}
	}
	else if (wait_counter > Common::sec2frame(7.5f) && !player->is_now_animation()) {
		// 入力メッセージとプレーヤーに攻撃前演出
		auto message = (MessageWindow *)getChildByTag(TAG_MESSAGE_WINDOW);
		message->set_message("入力せよ！！");
        message->set_disp_timer(1.5f);
        this->_reset_touch_panel_color();
		wait_counter = 0;
		game_step = STEP_INPUT;
	}
}

void Game::_update_input(float flame) {
	
	input_timer += flame;
	// CCLOG("%f", input_timer);
    
    auto enemy = (Enemy *)getChildByTag(TAG_ENEMY);
	
	auto input_enable_time = enemy->get_input_enable_time();
	if (input_timer > input_enable_time) {
		input_timer = 0.0f;
		
        this->_enemy_attack();
		
	}
	
	// ゲージの表示調整
	float scale = 1.0f - (input_timer/input_enable_time);
	auto gauge = getChildByTag(TAG_INPUT_GAUGE);
	gauge->setScaleX(scale);
	
	float r = 255 * (1.0f - scale);
	float g = 255 * scale;
	float b = 0;
	auto col = Color3B(r, g, b);
	gauge->setColor(col);
	
    // 入力時間切れ、もしくはコンボノルマ達成の場合、そこで発動終了
	if (enemy->is_defeat(this->combo_num)) {
    }
	if (enemy->is_combo_norma_achieve(this->combo_num)) {
	}
    
    // 倒した
    if (this->test_enemy_hp <= 0) {
        this->game_step = STEP_SUCCESS;
    }
    
    // 自分がやられた
    if (this->test_player_hp <= 0) {
        this->game_step = STEP_FAIL;
    }
}

void Game::_update_success(void) {
	auto player = (_Player *)getChildByTag(TAG_PLAYER);
	this->wait_counter++;
	if (this->wait_counter == Common::sec2frame(1.0f)) {
		auto enemy = (Enemy *)getChildByTag(TAG_ENEMY);
		enemy->dead_animation();
		play_se("defeat.wav");
	}
	if (this->wait_counter > Common::sec2frame(2.0f)) {
		this->wait_counter = Common::sec2frame(2.0f);
		if (!player->is_now_animation()) {
			player->win_animation_repeat();
			game_step = STEP_RESULT;
		}
	}
}

void Game::_update_fail(void) {
	if (!_is_now_damage_animation()) {
		auto player = (_Player *)getChildByTag(TAG_PLAYER);
		if (wait_counter++ > 30) {
			wait_counter = 0;
			player->die_animation();
			game_step = STEP_RESULT;
		}
	}
}

void Game::_update_result() {
	// 結果とnextボタンを表示する
	int check = 60;
	if (wait_counter++ >= check) {
		auto enemy = (Enemy *)getChildByTag(TAG_ENEMY);
		auto damage = enemy->get_damage();
		
		// Great率を計算(0 - 1の範囲は1にする)
		auto great_rate = (float)this->judge_great_count * 100.0f / (float)enemy->get_combo_limit();
		if (0.0f < great_rate && great_rate < 1.0f) {
			great_rate = 1.0f;
		}
		
		// ゲームデータを更新
		this->_update_game_manager(great_rate);
		this->_save_play_data();
        
        // メッセージウインドウで結果を表示
        this->test_messages = {
            "てきをたおした！",
            "の経験値を得た",
            "ゴールドをてに入れた",
            "br;",
            "てうと"
        };
        auto message_window = (MessageLayer *)this->getChildByTag(TAG_MESSAGE_WINDOW_LAYER);
        message_window->set_message(this->test_messages);
		//Common::nend_icon_enable();
        CCLOG("hogehoge");
		
		// BGM
		std::string bgm_filename = enemy->is_defeated() ? "win_fanfare.mp3" : "lose_rec.mp3";
		play_bgm(bgm_filename.c_str());
		set_bgm_volume(BGM_VOLUME_MAX);
		
		wait_counter = check;
		game_step = STEP_RESULT_END;
	}
}

void Game::_update_result_end() {
    auto message_window = (MessageLayer *)this->getChildByTag(TAG_MESSAGE_WINDOW_LAYER);
    CCLOG("tag = %d, %d ", message_window->getTag(), message_window->isVisible());
    // 表示が消えたらシーン読み込み
    if (!message_window->isVisible()) {
        auto gm = GameManager::getInstance();
        auto next_scene = (true) ? MapScene::createScene() : Loading::createScene();
        float duration = 1.0f;
        
        // BGM フェードアウト
        this->runAction(MusicFade::create(duration/2, BGM_VOLUME_MIN, true));
        
        Scene *p_scene = TransitionFade::create(duration, next_scene);
        if (p_scene) {
            Director::getInstance()->replaceScene(p_scene);
        }
        game_step = STEP_END;
    }
}

//---------------------------------------------------------
// パネルの反応領域
//---------------------------------------------------------
Rect Game::get_test_rect(int x, int y, Size size) {
	return Rect(x - size.width/2, y - size.height/2, size.width, size.height);
}

//---------------------------------------------------------
// touch function
//---------------------------------------------------------
bool Game::onTouchBegan(Touch *touch, Event *unused_event)
{
	// タップ開始時の処理
	
	// プレーヤー攻撃時は演出スキップ。それ以外は入力時しか受け付けない
    if (this->game_step != STEP_INPUT) {
		return true;
	}
    
    // すでに死んでたら無視
    if (this->test_player_hp <= 0) {
        return true;
    }

	// 開始時の座標を取得
	auto start_pos = touch->getLocation();

	// 点からやる
	bool is_collision = false;
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			int position = this->_get_img_position_by_xy(x, y);
			auto btn = (Sprite*)this->getChildByTag(TAG_TOUCH_BUTTON + position);

			// TODO : 指の領域確認
			auto btn_rect   = btn->getBoundingBox();
			auto touch_rect = this->get_test_rect(start_pos.x, start_pos.y, TOUCH_SIZE);
			if (btn_rect.intersectsRect(touch_rect)) {
				is_collision = true;
				
				// 正解しているかチェックする
                if (position == this->question) {
                    btn->setColor(PANEL_COLOR);
                    //
                    
                    auto judge = this->_get_judge();
                    _success_effect(judge);
                    this->judge_great_count += judge;
                    this->input_timer = 0.0f;
                    this->combo_num++;
                    this->_limit_break_num_effect();
                    play_se("input_success.wav");
                    //
                    this->_player_attack();
				}
			}
		}
	}
	// 触れてない場合は、あさっての方向へ
	if (!is_collision) {
		this->touch_st = Point(-1.0f, -1.0f);
	}

	return true;
}

void Game::onTouchMoved(Touch *touch, Event *unused_event) {
}

void Game::onTouchEnded(Touch *tounc, Event *unused_event)
{
	// パネルをリセット
	this->_reset_touch_panel_color();

	// 問題の更新
	this->input_count++;
	this->_init_question();
    this->_reset_touch_panel_color();
}

void Game::onTouchCancelled(Touch *tounch, Event *unused_event)
{
	// タップイベントのキャンセル時の処理
	CCLOG("on touch cancelled");
}


//=========================================================
// private
//=========================================================

//---------------------------------------------------------
//  game managerの更新
//---------------------------------------------------------
void Game::_update_game_manager(int great_rate) {
    /*
	auto enemy = (Enemy *)this->getChildByTag(TAG_ENEMY);
	auto gm = GameManager::getInstance();
	
	this->added_attack_value = gm->update_attack_value(this->combo_num, enemy->get_damage());
	
	gm->set_max_combo(this->combo_num);
	gm->set_total_damage(enemy->get_total_damaged());
	gm->set_max_damage(enemy->get_damage());
	
	if (enemy->is_defeated()) {
		gm->release_next_level();
	}
	
	// TODO : タイトル判定
	
	gm->play_count_increment();
	
	// 敵情報
	std::map<std::string, int> battle_data;
	battle_data["level"]      = gm->get_selected_level();
	battle_data["combo"]      = this->combo_num;
	battle_data["is_defeat"]  = enemy->is_defeated();
	battle_data["great_rate"] = great_rate;
	gm->update_enemy_data(gm->get_selected_level(), battle_data);
     */
}

//---------------------------------------------------------
// プレーデータの保存
//---------------------------------------------------------
void Game::_save_play_data() {
    /*
	auto gm = GameManager::getInstance();
	// データ作成
	std::map<std::string, int> data_map;
	data_map["attack"]       = gm->get_attack_value();
	data_map["combo"]        = gm->get_max_combo();
	data_map["max_damage"]   = gm->get_max_damage();
	data_map["total_damage"] = gm->get_total_damage();
	data_map["boss_bit"]     = gm->get_released_boss_bit();
	data_map["title"]        = 1; // TODO
	data_map["try_count"]    = gm->get_play_count();
	
	PlayerData::update_play_data(data_map);
	
	EnemyData::update_data(gm->get_selected_level(), gm->get_enemy_data());
     */
}

//---------------------------------------------------------
//  問題の初期化
//---------------------------------------------------------
void Game::_init_question() {
    
    // 次の問題を選択、同じな場合はずらす
    int choice = arc4random() % this->disp_numbers.size();
    if (choice == this->question) {
        choice = (choice + 1) % this->disp_numbers.size();
    }
    this->question = choice;
}

//---------------------------------------------------------
// 色を付けたパネルを元に戻す
//---------------------------------------------------------
void Game::_reset_touch_panel_color() {
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			int position = this->_get_img_position_by_xy(x, y);
			auto btn = (Sprite*)this->getChildByTag(TAG_TOUCH_BUTTON + position);
            auto _col = (this->question == position) ? PANEL_COLOR : Color3B::WHITE;
			btn->setColor(_col);
		}
	}
}

//---------------------------------------------------------
//  xyを配列用に変換する
//---------------------------------------------------------
int Game::_get_img_position_by_xy(int x, int y) {
	return x + y * 3;
}

//---------------------------------------------------------
// 入力の精度判定
//---------------------------------------------------------
int Game::_get_judge() {
	auto enemy = (Enemy *)getChildByTag(TAG_ENEMY);
	auto input_enable_time = enemy->get_input_enable_time();
	int wait_percent = this->input_timer * 100 / input_enable_time;
	
	// 半分以内に入力した
	if (wait_percent < 50) {
		return JUDGE_GREAT;
	}
	return JUDGE_GOOD;
}

//---------------------------------------------------------
// 攻撃アニメーションの演出
//---------------------------------------------------------
void Game::_attack_animation(int key) {

    std::string str = "attack_naname_anime";
    auto effect = (Sprite *)this->getChildByTag(TAG_ATTACK_EFFECT_NANAME);
    effect->setRotationX(0.0f);
    effect->setRotationY(180.0f);

    // アニメーションを生成
    auto cache = AnimationCache::getInstance();
    auto animation = cache->getAnimation(str);

    auto action = Animate::create(animation);
    auto action_req = Repeat::create(action, 1);
    action_req->setTag(999);
    effect->runAction(action_req);
	
	// SE
	play_se("player_attack.wav");
}

//---------------------------------------------------------
// 敵の攻撃演出
//---------------------------------------------------------
void Game::_damage_animation() {
	
	auto damage = (Sprite *)getChildByTag(TAG_DAMAGE_EFFECT);
	damage->setVisible(true);
	
    // アニメーションを生成
    auto cache = AnimationCache::getInstance();
    auto animation = cache->getAnimation("enemy_attack_anime");
	
    auto action = Animate::create(animation);
    auto action_req = Repeat::create(action, 1);
    action_req->setTag(999);
    damage->runAction(action_req);
	
	// SE
	std::string se_file = "lv" + std::to_string(1) + "_attack.wav";
	play_se(se_file.c_str());
}

//---------------------------------------------------------
// 敵の攻撃演出中か
//---------------------------------------------------------
bool Game::_is_now_damage_animation(void) {
	auto damage_effect = (Sprite *)getChildByTag(TAG_DAMAGE_EFFECT);
	return damage_effect->getNumberOfRunningActions() > 0;
}



//---------------------------------------------------------
// 入力成功時のパネル演出
//---------------------------------------------------------
void Game::_success_effect(int judge) {
	// GREAT時は色を付ける
	auto color = Color3B::WHITE;
	if (judge == JUDGE_GREAT) {
		color = Color3B::ORANGE;
	}
	
    int index = question;
    auto button_effect = (Sprite *)getChildByTag(TAG_TOUCH_EFFECT + index);
    button_effect->setOpacity(255);
    button_effect->setScale(1.0f);
    button_effect->setColor(color);
    auto scale = button_effect->getScale();
    auto scale_to = ScaleTo::create(0.5f, scale * 2.0f);
    auto fade_out = FadeOut::create(0.5f);
    auto spawn    = Spawn::create(scale_to, fade_out, nullptr);
    button_effect->runAction(spawn);
}

//---------------------------------------------------------
// カウントダウン表示アニメーション
//---------------------------------------------------------
void Game::_countdown_animation(int disp_sec) {
	auto duration = 1.0f;
	
	auto count_down = (Sprite *)this->getChildByTag(TAG_COUNTDOWN);
	count_down->setSpriteFrame("number_" + std::to_string(disp_sec) + ".png");
	count_down->setVisible(disp_sec > 0);
	count_down->setOpacity(255);
	count_down->setScale(1.0f);
	auto scale_to = ScaleTo::create(duration, 2.0f);
	auto fade_out = FadeOut::create(duration);
	auto spawn    = Spawn::create(scale_to, fade_out, nullptr);
	count_down->runAction(spawn);
}

//---------------------------------------------------------
// 現在つないでいるコンボ表示
//---------------------------------------------------------
void Game::_limit_break_num_effect() {
	auto disp_combo_num = this->combo_num;
	for (int i = 0; i < 4; i++ ) {
		auto num_img = (Sprite *)this->getChildByTag(TAG_LIMIT_BREAK_NUM + i);
		int disp_keta = disp_combo_num % 10;
		disp_combo_num /= 10;
		num_img->setSpriteFrame("number_" + std::to_string(disp_keta) + ".png");
		num_img->setVisible(disp_keta || disp_combo_num > 0 || i == 0);
	}
}

// new!!
void Game::_player_attack() {
    auto player = (_Player *)this->getChildByTag(TAG_PLAYER);
    auto enemy = (Enemy *)this->getChildByTag(TAG_ENEMY);
    int damage = enemy->add_damage(123);//(game_manager->get_attack_value());
    player->attack_animation();
    _attack_animation(this->attack_count);
    auto damage_text = (DamageText *)getChildByTag(TAG_DAMEGE_TEXT + this->attack_count % DAMAGE_TEXT_NUM);
    damage_text->do_animation(damage);
    this->attack_count++;
    
    this->test_enemy_hp -= 10;
}


void Game::_enemy_attack() {
    auto player = (_Player *)getChildByTag(TAG_PLAYER);
    auto enemy  = (Enemy *)getChildByTag(TAG_ENEMY);
    if (!enemy->is_now_pre_attack_animation()) {
        player->damage_animation();
        // ダメージ表示
        auto damage_text = (DamageText *)getChildByTag(TAG_DAMEGE_TEXT);
        damage_text->setPosition(player->getPosition());
        damage_text->do_player_side_animation(9999);
        auto damage_animation = (Sprite *)getChildByTag(TAG_DAMAGE_EFFECT);
        damage_animation->setVisible(false);
        
        enemy->pre_attack_animation();
        _damage_animation();
        
        this->test_player_hp -= 10;
    }
}

/* EOF */