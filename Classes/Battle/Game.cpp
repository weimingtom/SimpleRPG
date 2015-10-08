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
	STEP_PLAYER_ATTACK,
	STEP_SUCCESS,
	STEP_ENEMY_ATTACK,
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
	
	// 問題の読み込み
	std::string commands_file = "lv" + std::to_string(1) + "_commands.plist";
	ValueVector list_vec = FileUtils::getInstance()->getValueVectorFromFile(RES_BATTLE_DIR + commands_file);
	for(Value list : list_vec){
		ValueMap list_map = list.asValueMap();
		int command = list_map.at("command").asInt();
		//list.asString().c_str();
		this->commands.push_back(command);
	}

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

	// ラインを書く
	Size size = Director::getInstance()->getWinSize();
	auto node = DrawNode::create();
	this->addChild(node, ORDER_TOUCH_LINE, TAG_LINE_NUM);

	// fixed line
	auto fixed_line = DrawNode::create();
	this->addChild(fixed_line, ORDER_TOUCH_LINE, TAG_FIXED_LINE);


	this->input_count     = 0;
	this->combo_num       = 0;
	this->wait_counter    = 0;
	this->attack_count    = 0;
	this->judge_great_count = 0;
	this->count_down_disp_num = 0;
	
	this->input_timer = .0f;
	
	this->is_mistaked           = false;
	this->is_touch_proc_igonre  = false;
	this->is_timeout            = false;
	this->is_player_attack_skip = false;


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
	
	this->_reset_touch_panel_color();
	
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
			
		case STEP_PLAYER_ATTACK:
			_update_player_attack();
			break;
			
		case STEP_ENEMY_ATTACK:
			_update_enemy_attack();
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
		_charge_animation();
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
        this->is_timeout = true;
		
		// test
		//game_step = STEP_ENEMY_ATTACK;
		//game_step = STEP_PLAYER_ATTACK;
	}
	
	// ゲージの表示調整
	float scale = this->is_timeout ? 0.0f : 1.0f - (input_timer/input_enable_time);
	auto gauge = getChildByTag(TAG_INPUT_GAUGE);
	gauge->setScaleX(scale);
	
	float r = 255 * (1.0f - scale);
	float g = 255 * scale;
	float b = 0;
	auto col = Color3B(r, g, b);
	gauge->setColor(col);
	
    // 入力時間切れ、もしくはコンボノルマ達成の場合、そこで発動終了
	if (this->is_timeout || enemy->is_combo_norma_achieve(this->combo_num)) {
		// アニメーションとヒントを消す
		_charge_stop_animation_and_hint();
		game_step = STEP_PLAYER_ATTACK;
		
		// 攻撃する場合、セリフを出す
		if (this->combo_num > 0) {
			auto player = (_Player *)this->getChildByTag(TAG_PLAYER);
			auto message = (MessageWindow *)getChildByTag(TAG_MESSAGE_WINDOW);
			message->set_message(player->get_attack_serif());
			message->setVisible(true);
		}
	}
}

void Game::_update_player_attack(void) {
	auto game_manager = (GameManager *)GameManager::getInstance();
	auto player = (_Player *)getChildByTag(TAG_PLAYER);
	auto enemy  = (Enemy *)getChildByTag(TAG_ENEMY);
	
	if (player->is_now_animation()) {
		// to do nothing
	} else {
		// 終了
#if IS_DEFEAT_DEBUG
		this->combo_num = game_manager->get_combo_norma(); // for debug
#endif
		if (this->combo_num == this->attack_count) {
			//game_step = STEP_INPUT;
			if (wait_counter++ > Common::sec2frame(1.0f)) {
				// リセットしとく
				this->attack_count = 0;
				wait_counter = 0;
				if (enemy->is_defeat(this->combo_num)) {
					game_step = STEP_SUCCESS;
					
					// 倒す時のセリフを
					auto message = (MessageWindow *)getChildByTag(TAG_MESSAGE_WINDOW);
					message->set_message(enemy->get_dead_serif(this->combo_num));
					message->set_disp_timer(2.0f);
				}
				else {
					game_step = STEP_ENEMY_ATTACK;
					enemy->pre_attack_animation();
					
					// 攻撃される時のセリフを
					auto message = (MessageWindow *)getChildByTag(TAG_MESSAGE_WINDOW);
					message->set_message(enemy->get_attack_serif());
					message->set_disp_timer(2.0f);
				}
			}
			return;
		}
        // ダメージを与える
        auto enemy = (Enemy *)getChildByTag(TAG_ENEMY);
		
		// 攻撃演出
		if (!this->is_player_attack_skip) {
            int damage = enemy->add_damage(123);//(game_manager->get_attack_value());
			player->attack_animation();
			_attack_animation(this->attack_count);
			auto damage_text = (DamageText *)getChildByTag(TAG_DAMEGE_TEXT + this->attack_count % DAMAGE_TEXT_NUM);
			damage_text->do_animation(damage);
			this->attack_count++;
		}
		else {
			for (int i = this->attack_count; i < this->combo_num; i++) {
                enemy->add_damage(456);//(game_manager->get_attack_value());
			}
			this->attack_count = this->combo_num;
		}
	}
	// すべての演出を終えた、もしくはスキップするまで演出する
	
}

void Game::_update_enemy_attack(void) {
	auto player = (_Player *)getChildByTag(TAG_PLAYER);
	auto enemy  = (Enemy *)getChildByTag(TAG_ENEMY);
	if (!enemy->is_now_pre_attack_animation()) {
		player->damage_animation();
		_damage_animation();
		wait_counter = 0;
		game_step = STEP_FAIL;
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
		// ダメージ値
		if (wait_counter == 0) {
			auto damage_text = (DamageText *)getChildByTag(TAG_DAMEGE_TEXT);
			damage_text->setPosition(player->getPosition());
			damage_text->do_player_side_animation(9999);
			auto damage_animation = (Sprite *)getChildByTag(TAG_DAMAGE_EFFECT);
			damage_animation->setVisible(false);
		}
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
        std::vector<std::string> test_message = {
            "てきをたおした！",
            "の経験値を得た",
            "ゴールドをてに入れた",
            "",
            "てうと"
        };
        auto message_window = (MessageLayer *)this->getChildByTag(TAG_MESSAGE_WINDOW_LAYER);
        message_window->set_message(test_message);
		//Common::nend_icon_enable();
		
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
	if (this->game_step == STEP_PLAYER_ATTACK) {
		this->is_player_attack_skip = true;
		return true;
	}
	else if (this->game_step != STEP_INPUT) {
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
	bool is_mistaked = this->is_mistaked;
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
    // リセット
    this->question = -1;
    
	this->is_mistaked = false;

	//int index        = this->input_count % this->commands.size(); // 入力ミスで入れ替え
	int index        = this->combo_num % this->commands.size();
	int next_command = this->commands[index];
	//int next_command = this->input_count % COUNT_OF(flag_position); // for test

	// 問題を作る
    this->question = arc4random() % COUNT_OF(this->disp_number) ;
	
	// 表示用の数字を埋める
	std::vector<int> vector;
	for (int i = 0; i < COUNT_OF(disp_number); i++) {
		vector.push_back(i+1);
	}
	
	for (int i = 0; i < COUNT_OF(disp_number); i++) {
		disp_number[i] = vector[i];
	}
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
// 入力時のプレーヤー演出
//---------------------------------------------------------
void Game::_charge_animation() {
	auto charge = (Sprite *)getChildByTag(TAG_CHARGE);
	charge->setVisible(true);
	
    // アニメーションを生成
    auto cache = AnimationCache::getInstance();
    auto animation = cache->getAnimation("charge_anime");
	
    auto action = Animate::create(animation);
    auto action_req = RepeatForever::create(action);
    action_req->setTag(999);
    charge->runAction(action_req);
}

//---------------------------------------------------------
// 入力時の演出を止めて、ヒントを出す
//---------------------------------------------------------
void Game::_charge_stop_animation_and_hint() {
	auto charge = (Sprite *)getChildByTag(TAG_CHARGE);
	charge->setVisible(false);
	
	auto charge_action = charge->getActionByTag(999);
	charge->stopAction(charge_action);
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



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
//                ここからしたは参考
//
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#if 0
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
    STEP_PLAYER_ATTACK,
    STEP_SUCCESS,
    STEP_ENEMY_ATTACK,
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

// TODO:plistにする
// 6 7 8
// 3 4 5
// 0 1 2
const int flag_position[16][3] = {
    {0, 3, 6}, // UP
    {1, 4, 7}, // UP
    {2, 5, 8}, // UP
    
    {6, 3, 0}, // DOWN
    {7, 4, 1}, // DOWN
    {8, 5, 2}, // DOWN
    
    {2, 1, 0}, // LEFT
    {5, 4, 3}, // LEFT
    {8, 7, 6}, // LEFT
    
    {0, 1, 2}, // RIGHT
    {3, 4, 5}, // RIGHT
    {6, 7, 8}, // RIGHT
    
    {2, 4, 6}, // UP_LEFT
    {0, 4, 8}, // UP_RIGHT
    {8, 4, 0}, // DOWN_LEFT
    {6, 4, 2}, // DOWN_RIGHT
};
// todo : MAP化
const int flag_position_effect[16] = {
    UP, UP, UP,
    DOWN, DOWN, DOWN,
    LEFT, LEFT, LEFT,
    RIGHT, RIGHT, RIGHT,
    UP_LEFT,
    UP_RIGHT,
    DOWN_LEFT,
    DOWN_RIGHT,
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
    
    // 問題の読み込み
    std::string commands_file = "lv" + std::to_string(1) + "_commands.plist";
    ValueVector list_vec = FileUtils::getInstance()->getValueVectorFromFile(RES_BATTLE_DIR + commands_file);
    for(Value list : list_vec){
        ValueMap list_map = list.asValueMap();
        int command = list_map.at("command").asInt();
        //list.asString().c_str();
        this->commands.push_back(command);
    }
    
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
    
    // ラインを書く
    Size size = Director::getInstance()->getWinSize();
    auto node = DrawNode::create();
    this->addChild(node, ORDER_TOUCH_LINE, TAG_LINE_NUM);
    
    // fixed line
    auto fixed_line = DrawNode::create();
    this->addChild(fixed_line, ORDER_TOUCH_LINE, TAG_FIXED_LINE);
    
    
    this->input_count     = 0;
    this->touched_count   = 0;
    this->answer_progress = 0;
    this->answer_total    = 0;
    this->combo_num       = 0;
    this->wait_counter    = 0;
    this->attack_count    = 0;
    this->judge_great_count = 0;
    this->count_down_disp_num = 0;
    
    this->input_timer = .0f;
    
    this->is_mistaked           = false;
    this->is_touch_proc_igonre  = false;
    this->is_timeout            = false;
    this->is_player_attack_skip = false;
    
    
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
    
    // ヒント画像
    auto adjust_hint_y = 20.0f;
    auto chain_img = Sprite::create(RES_BATTLE_DIR + "chain.png");
    chain_img->setPosition(visibleSize.width/2 - 100.0f,
                           visibleSize.height/2 + adjust_hint_y);
    this->addChild(chain_img, ORDER_HINTS, TAG_CHAIN);
    
    for (int i = 0; i < COUNT_OF(this->question); i++) {
        auto hint = Sprite::createWithSpriteFrameName("number_0.png");
        hint->setScale(0.5f);
        auto h_size = hint->getContentSize();
        hint->setPosition(visibleSize.width/2 + (i-1) * h_size.width + 50.0f,
                          visibleSize.height/2 + adjust_hint_y);
        hint->setVisible(false);
        this->addChild(hint, ORDER_HINTS, TAG_HINTS + i);
    }
    
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
    
    this->_switch_texture();
    
    //this->test_code();
    
    this->scheduleUpdate();
    
    // MotionStreakを作成
    this->addChild(this->_create_motion_streak(), ORDER_UI, TAG_MOTION_STREAK);
    
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
            
        case STEP_PLAYER_ATTACK:
            _update_player_attack();
            break;
            
        case STEP_ENEMY_ATTACK:
            _update_enemy_attack();
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
        // ヒント表示開始
        for (int i = 0; i < COUNT_OF(this->question); i++) {
            auto hint = this->getChildByTag(TAG_HINTS + i);
            hint->setVisible(true);
        }
        
        // 入力メッセージとプレーヤーに攻撃前演出
        auto message = (MessageWindow *)getChildByTag(TAG_MESSAGE_WINDOW);
        message->set_message("入力せよ！！");
        message->set_disp_timer(1.5f);
        _charge_animation();
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
        this->is_timeout = true;
        
        // test
        //game_step = STEP_ENEMY_ATTACK;
        //game_step = STEP_PLAYER_ATTACK;
    }
    
    // ゲージの表示調整
    float scale = this->is_timeout ? 0.0f : 1.0f - (input_timer/input_enable_time);
    auto gauge = getChildByTag(TAG_INPUT_GAUGE);
    gauge->setScaleX(scale);
    
    float r = 255 * (1.0f - scale);
    float g = 255 * scale;
    float b = 0;
    auto col = Color3B(r, g, b);
    gauge->setColor(col);
    
    // 入力時間切れ、もしくはコンボノルマ達成の場合、そこで発動終了
    if (this->is_timeout || enemy->is_combo_norma_achieve(this->combo_num)) {
        // アニメーションとヒントを消す
        _charge_stop_animation_and_hint();
        game_step = STEP_PLAYER_ATTACK;
        
        // 攻撃する場合、セリフを出す
        if (this->combo_num > 0) {
            auto player = (_Player *)this->getChildByTag(TAG_PLAYER);
            auto message = (MessageWindow *)getChildByTag(TAG_MESSAGE_WINDOW);
            message->set_message(player->get_attack_serif());
            message->setVisible(true);
        }
    }
}

void Game::_update_player_attack(void) {
    auto game_manager = (GameManager *)GameManager::getInstance();
    auto player = (_Player *)getChildByTag(TAG_PLAYER);
    auto enemy  = (Enemy *)getChildByTag(TAG_ENEMY);
    
    if (player->is_now_animation()) {
        // to do nothing
    } else {
        // 終了
#if IS_DEFEAT_DEBUG
        this->combo_num = game_manager->get_combo_norma(); // for debug
#endif
        if (this->combo_num == this->attack_count) {
            //game_step = STEP_INPUT;
            if (wait_counter++ > Common::sec2frame(1.0f)) {
                // リセットしとく
                this->attack_count = 0;
                wait_counter = 0;
                if (enemy->is_defeat(this->combo_num)) {
                    game_step = STEP_SUCCESS;
                    
                    // 倒す時のセリフを
                    auto message = (MessageWindow *)getChildByTag(TAG_MESSAGE_WINDOW);
                    message->set_message(enemy->get_dead_serif(this->combo_num));
                    message->set_disp_timer(2.0f);
                }
                else {
                    game_step = STEP_ENEMY_ATTACK;
                    enemy->pre_attack_animation();
                    
                    // 攻撃される時のセリフを
                    auto message = (MessageWindow *)getChildByTag(TAG_MESSAGE_WINDOW);
                    message->set_message(enemy->get_attack_serif());
                    message->set_disp_timer(2.0f);
                }
            }
            return;
        }
        // ダメージを与える
        auto enemy = (Enemy *)getChildByTag(TAG_ENEMY);
        
        // 攻撃演出
        if (!this->is_player_attack_skip) {
            int damage = enemy->add_damage(123);//(game_manager->get_attack_value());
            player->attack_animation();
            _attack_animation(this->attack_count);
            auto damage_text = (DamageText *)getChildByTag(TAG_DAMEGE_TEXT + this->attack_count % DAMAGE_TEXT_NUM);
            damage_text->do_animation(damage);
            this->attack_count++;
        }
        else {
            for (int i = this->attack_count; i < this->combo_num; i++) {
                enemy->add_damage(456);//(game_manager->get_attack_value());
            }
            this->attack_count = this->combo_num;
        }
    }
    // すべての演出を終えた、もしくはスキップするまで演出する
    
}

void Game::_update_enemy_attack(void) {
    auto player = (_Player *)getChildByTag(TAG_PLAYER);
    auto enemy  = (Enemy *)getChildByTag(TAG_ENEMY);
    if (!enemy->is_now_pre_attack_animation()) {
        player->damage_animation();
        _damage_animation();
        wait_counter = 0;
        game_step = STEP_FAIL;
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
        // ダメージ値
        if (wait_counter == 0) {
            auto damage_text = (DamageText *)getChildByTag(TAG_DAMEGE_TEXT);
            damage_text->setPosition(player->getPosition());
            damage_text->do_player_side_animation(9999);
            auto damage_animation = (Sprite *)getChildByTag(TAG_DAMAGE_EFFECT);
            damage_animation->setVisible(false);
        }
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
        std::vector<std::string> test_message = {
            "てきをたおした！",
            "の経験値を得た",
            "ゴールドをてに入れた",
            "",
            "てうと"
        };
        auto message_window = (MessageLayer *)this->getChildByTag(TAG_MESSAGE_WINDOW_LAYER);
        message_window->set_message(test_message);
        //Common::nend_icon_enable();
        
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
    if (this->game_step == STEP_PLAYER_ATTACK) {
        this->is_player_attack_skip = true;
        return true;
    }
    else if (this->game_step != STEP_INPUT) {
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
                this->touch_st.x = btn->getPositionX();
                this->touch_st.y = btn->getPositionY();
                // タッチしたポジションを記憶
                this->touched[position] = true;
                is_collision = true;
                this->touched_count++;
                
                // 正解しているかチェックする
                if (this->_is_collect(position)) {
                    this->answer_progress++;
                    btn->setColor(PANEL_COLOR);
                }
            }
        }
    }
    // 触れてない場合は、あさっての方向へ
    if (!is_collision) {
        this->touch_st = Point(-1.0f, -1.0f);
    }
    
    
    // MotionStreakを作成
    this->removeChildByTag(TAG_MOTION_STREAK);
    auto motion_streak = this->_create_motion_streak();
    this->addChild(motion_streak, ORDER_UI, TAG_MOTION_STREAK);
    
    motion_streak->setPosition(start_pos);
    
    return true;
}

void Game::onTouchMoved(Touch *touch, Event *unused_event)
{
    // スワイプの処理
    
    // 処理しない、もしくは開始時にタッチパネルに触れていない
    if (this->is_touch_proc_igonre || this->touched_count == 0) {
        return;
    }
    
    auto node = (DrawNode*)this->getChildByTag(TAG_LINE_NUM);
    node->clear();
    
    // 進行中の線
    if (this->touch_st.x > 0 && this->touch_st.y > 0) {
        auto start_vec = Vec2(this->touch_st.x, this->touch_st.y);
        auto end_vec   = Vec2(touch->getLocation().x, touch->getLocation().y);
        node->drawSegment(start_vec, end_vec, LINE_SIZE, LINE_COLOR);
    }
    
    // ポッチで連結している部分に線を引く
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            
            if (this->touch_st.x > 0 && this->touch_st.y > 0) {
                int position = this->_get_img_position_by_xy(x, y);
                auto btn = (Sprite*)this->getChildByTag(TAG_TOUCH_BUTTON + position);
                
                auto btn_rect = btn->getBoundingBox();
                auto touch_rect = this->get_test_rect(touch->getLocation().x, touch->getLocation().y, TOUCH_SIZE);
                
                // 触れていたら線をつなぐ
                if (!this->touched[position] && btn_rect.intersectsRect(touch_rect)) {
                    auto node2 = (DrawNode*)this->getChildByTag(TAG_FIXED_LINE);
                    
                    auto start_vec = Vec2(btn->getPositionX(), btn->getPositionY());
                    auto end_vec   = Vec2(this->touch_st.x, this->touch_st.y);
                    node2->drawSegment(start_vec, end_vec, LINE_SIZE, LINE_COLOR);
                    
                    // 起点を更新する
                    this->touch_st.x = btn->getPositionX();
                    this->touch_st.y = btn->getPositionY();
                    
                    // 触ったことにする
                    this->touched[position] = true;
                    this->touched_count++;
                    
                    // 正解しているかチェックする
                    if (this->_is_collect(position)) {
                        this->answer_progress++;
                        btn->setColor(PANEL_COLOR);
                    }
                    break;
                }
            }
        }
    }
    
    // タッチ数と、正解数が違う時点で間違えている
    // もしくは入力時間切れ
    if ((this->touched_count != this->answer_progress) || this->is_timeout) {
        this->is_mistaked = true;
        // 間違えた時点で処理させない
        this->is_touch_proc_igonre = true;
        // 線を消す
        this->_reset_line();
        this->_reset_touch_panel_color();
    }
    
    // タッチ演出
    MotionStreak* motion_streak = (MotionStreak *)this->getChildByTag(TAG_MOTION_STREAK);
    motion_streak->setPosition(touch->getLocation());
    
}

void Game::onTouchEnded(Touch *tounc, Event *unused_event)
{
    // タップ終了時の処理
    
    // パネルにタッチしてない
    if (this->touched_count == 0) {
        CCLOG("not started");
        return;
    }
    
    // 正解判定はタッチが終了するまでわからないので、ここでやる
    // 意図して、正解よりも余分にタッチしたかもしれない(基本ないだろうが)
    CCLOG("t %d, p %d", answer_total, answer_progress);
    if (this->answer_total != this->answer_progress) {
        this->is_mistaked = true;
    }
    
    // 途中、もしくは最終的に間違ってた
    if (this->is_mistaked) {
        CCLOG("mistake!!");
    }
    else {
        CCLOG("collect!");
        auto judge = this->_get_judge();
        _success_effect(judge);
        this->judge_great_count += judge;
        this->input_timer = 0.0f;
        this->combo_num++;
        this->_limit_break_num_effect();
        play_se("input_success.wav");
    }
    
    // 線を消す
    this->_reset_line();
    this->_reset_touch_panel_color();
    
    // 判定をリセット
    for (int i = 0; i < COUNT_OF(this->touched); i++) {
        this->touched[i] = false;
    }
    this->is_touch_proc_igonre = false;
    
    // 問題の更新
    bool is_mistaked = this->is_mistaked;
    this->input_count++;
    this->_init_question();
    if (!is_mistaked) {
        this->_switch_texture();
    }
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
    // リセット
    for (int i = 0; i < COUNT_OF(this->question); i++) {
        this->question[i] = -1;
    }
    this->touched_count   = 0;
    this->answer_progress = 0;
    this->answer_total    = 0;
    this->is_mistaked = false;
    
    //int index        = this->input_count % this->commands.size(); // 入力ミスで入れ替え
    int index        = this->combo_num % this->commands.size();
    int next_command = this->commands[index];
    //int next_command = this->input_count % COUNT_OF(flag_position); // for test
    
    // 問題を作る
    for (int i = 0; i < COUNT_OF(flag_position[next_command]); i++) {
        if (flag_position[next_command][i] >= 0) {
            this->question[i] = flag_position[next_command][i];
            this->answer_total++;
        }
    }
    
    // 表示用の数字を埋める
    std::vector<int> vector;
    for (int i = 0; i < COUNT_OF(disp_number); i++) {
        vector.push_back(i+1);
    }
    // 表示がランダムになるレベルなら、シャッフル
    auto game_manager = GameManager::getInstance();
    //if (game_manager->is_random_number_level()) {
    //	random_shuffle(vector.begin(), vector.end());
    //}
    
    for (int i = 0; i < COUNT_OF(disp_number); i++) {
        disp_number[i] = vector[i];
    }
}

//---------------------------------------------------------
// 今表示されている線を消す
//---------------------------------------------------------
void Game::_reset_line() {
    auto fixed_line = (DrawNode*)this->getChildByTag(TAG_FIXED_LINE);
    auto input_line = (DrawNode*)this->getChildByTag(TAG_LINE_NUM);
    fixed_line->clear();
    input_line->clear();
}

//---------------------------------------------------------
// 色を付けたパネルを元に戻す
//---------------------------------------------------------
void Game::_reset_touch_panel_color() {
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            int position = this->_get_img_position_by_xy(x, y);
            auto btn = (Sprite*)this->getChildByTag(TAG_TOUCH_BUTTON + position);
            btn->setColor(Color3B::WHITE);
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
//  触れたパネルが正解か判断する
//---------------------------------------------------------
bool Game::_is_collect(int position) {
    bool is_collect = false;
    for (int i = 0; i < COUNT_OF(this->question); i++) {
        if (position == this->question[this->answer_progress]) {
            is_collect = true;
            break;
        }
    }
    return is_collect;
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
    int hoge = key % COUNT_OF(flag_position);
    int direction = flag_position_effect[hoge];
    
    std::string str;
    Sprite *effect = nullptr;
    switch (direction) {
        case UP:
            str = "attack_tate_anime";
            effect = (Sprite *)this->getChildByTag(TAG_ATTACK_EFFECT_TATE);
            effect->setRotationX(180.0f);
            break;
            
        case DOWN:
            str = "attack_tate_anime";
            effect = (Sprite *)this->getChildByTag(TAG_ATTACK_EFFECT_TATE);
            effect->setRotationX(0.0f);
            break;
            
        case LEFT:
            str = "attack_yoko_anime";
            effect = (Sprite *)this->getChildByTag(TAG_ATTACK_EFFECT_YOKO);
            effect->setRotationY(180.0f);
            break;
            
        case RIGHT:
            str = "attack_yoko_anime";
            effect = (Sprite *)this->getChildByTag(TAG_ATTACK_EFFECT_YOKO);
            effect->setRotationY(0.0f);
            break;
            
        case UP_LEFT:
            str = "attack_naname_anime";
            effect = (Sprite *)this->getChildByTag(TAG_ATTACK_EFFECT_NANAME);
            effect->setRotationX(180.0f);
            effect->setRotationY(180.0f);
            break;
            
        case UP_RIGHT:
            str = "attack_naname_anime";
            effect = (Sprite *)this->getChildByTag(TAG_ATTACK_EFFECT_NANAME);
            effect->setRotationX(180.0f);
            effect->setRotationY(0.0f);
            break;
            
        case DOWN_LEFT:
            str = "attack_naname_anime";
            effect = (Sprite *)this->getChildByTag(TAG_ATTACK_EFFECT_NANAME);
            effect->setRotationX(0.0f);
            effect->setRotationY(180.0f);
            break;
            
        case DOWN_RIGHT:
            str = "attack_naname_anime";
            effect = (Sprite *)this->getChildByTag(TAG_ATTACK_EFFECT_NANAME);
            effect->setRotationX(0.0f);
            effect->setRotationY(0.0f);
            break;
            
        default:
            break;
    }
    
    // アニメーションを生成
    auto cache = AnimationCache::getInstance();
    auto animation = cache->getAnimation(str);
    
    
    auto action = Animate::create(animation);
    auto action_req = Repeat::create(action, 1);
    //auto action_req = RepeatForever::create(action);
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
    auto gm = GameManager::getInstance();
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
// 入力時のプレーヤー演出
//---------------------------------------------------------
void Game::_charge_animation() {
    auto charge = (Sprite *)getChildByTag(TAG_CHARGE);
    charge->setVisible(true);
    
    // アニメーションを生成
    auto cache = AnimationCache::getInstance();
    auto animation = cache->getAnimation("charge_anime");
    
    auto action = Animate::create(animation);
    auto action_req = RepeatForever::create(action);
    action_req->setTag(999);
    charge->runAction(action_req);
}

//---------------------------------------------------------
// 入力時の演出を止めて、ヒントを出す
//---------------------------------------------------------
void Game::_charge_stop_animation_and_hint() {
    auto charge = (Sprite *)getChildByTag(TAG_CHARGE);
    charge->setVisible(false);
    
    auto charge_action = charge->getActionByTag(999);
    charge->stopAction(charge_action);
    
    // コマンドを消す
    auto chain = (Sprite *)this->getChildByTag(TAG_CHAIN);
    chain->setVisible(false);
    for (int i = 0; i < COUNT_OF(this->question); i++) {
        auto hint = (Sprite *)this->getChildByTag(TAG_HINTS + i);
        hint->setVisible(false);
    }
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
    
    for (int i = 0; i < COUNT_OF(question); i++) {
        int index = question[i];
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

MotionStreak* Game::_create_motion_streak() {
    return MotionStreak::create(1.0f, 1.0f, 50.0f, Color3B::YELLOW, RES_BATTLE_DIR + "touch_line.png");
}

#endif