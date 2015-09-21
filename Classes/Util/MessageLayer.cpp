#include "MessageLayer.h"

#include "extensions/cocos-ext.h"

#include "../Common.h"

USING_NS_CC;
USING_NS_CC_EXT;

#define IS_DEBUG 1

#define kModalLayerPriority -1

const int FONT_SIZE = 24;
const int SPACE = FONT_SIZE/2;
const int MAX_LINE = 4;

const float LETTER_DELAY = 0.1f;

// ORDER
enum E_ORDER_LAYER_RESULT {
    ORDER_MESSAGE_WINDOW,
    ORDER_MESSAGE,
	NR_ORDER
};

enum E_TAG_LAYER_RESULT {
    TAG_MESSAGE_WINDOW,
    TAG_MESSAGE_WINDOW_TEXT_0, // 開始位置としてのみ利用する
    TAG_MESSAGE_WINDOW_TEXT_1,
    TAG_MESSAGE_WINDOW_TEXT_2,
    TAG_MESSAGE_WINDOW_TEXT_3,
    TAG_MESSAGE_WINDOW_TEXT_4,
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
	auto base_position = Vec2(layer_size.width/2, layer_size.height/4);
    
    Size window_size = Size(layer_size.width - 50, 200);
    Scale9Sprite* pScale = Scale9Sprite::create("window.png", Rect(0, 0, 64, 64), Rect(10, 10, 44, 44));
    pScale->setContentSize(window_size);
    pScale->setPosition(base_position);
    pScale->setTag(TAG_MESSAGE_WINDOW);

    this->addChild(pScale, ORDER_MESSAGE_WINDOW);

	this->scheduleUpdate();
	
	// こっちのレイヤーのタッチを優先にしている
    
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
    
    listener->onTouchBegan     = CC_CALLBACK_2(MessageLayer::onTouchBegan, this);
    listener->onTouchMoved     = CC_CALLBACK_2(MessageLayer::onTouchMoved, this);
    listener->onTouchEnded     = CC_CALLBACK_2(MessageLayer::onTouchEnded, this);
    listener->onTouchCancelled = CC_CALLBACK_2(MessageLayer::onTouchCancelled, this);
	/*
    listener->onTouchBegan = [](Touch *touch,Event*event)->bool{
		return true;
	};
     */
	auto dip = Director::getInstance()->getEventDispatcher();
	dip->addEventListenerWithSceneGraphPriority(listener, this);
	dip->setPriority(listener, kModalLayerPriority);
    
    
    this->message_tests = {
        "あいうえおかきくけこさしすせそ",
        "たたたたたたたたたたたたたたた",
        "たたたたたたたたたたたたたたち",
        "たたたたたたたたたたたたたたつ",
        "",
        "こんにちは",
        "これはメッセージ",
        "ながれる",
        "テスト２",
    };
    
    this->message_start_y_pos = base_position.y + (FONT_SIZE + SPACE) * 2 + SPACE;
    this->_test(nullptr);

    return true;
}

void MessageLayer::update(float flame) {
	if (!this->update_start) {
		return;
	}
}

void MessageLayer::_test(Node* sender) {
    
    if (this->message_tests.size() < 1) {
        return;
    }
    
    // 1ラインずつ取得する
    std::string message = this->message_tests.back();
    this->message_tests.pop_back();
    
    CCLOG("mesa len %lu", message.length());
    
    // メッセージの内容によって、処理を変更
    // 改行
    if (message.length() < 1) {
        CCLOG("br!!");
        // カーソルを表示する
        this->is_disp_br_cursor = true;
    }
    else {
        this->_set_message(message);
    }
    
    
    // はい、いいえ　：　もしくは▼のタイミングで状態をリセットして設定する。
    
}

void MessageLayer::_set_message(std::string message) {
    
    // 規定のラインを送ったらライン送り
    this->message_now_line++;
    if (this->message_now_line > MAX_LINE) {
        this->message_now_line = MAX_LINE;
        // 一番上を削除し、他ラインのタグを更新する
        this->removeChildByTag(TAG_MESSAGE_WINDOW_TEXT_1);
        for (int tag = TAG_MESSAGE_WINDOW_TEXT_2; tag <= TAG_MESSAGE_WINDOW_TEXT_4; tag++) {
            auto _message = this->getChildByTag(tag);
            auto _tag = _message->getTag();
            _message->setTag(_tag - 1);
            // 上に移動する
            auto move_by = MoveBy::create(LETTER_DELAY, Vec2(0.0f, FONT_SIZE + SPACE));
            _message->runAction(move_by);
        }
    }
    
    auto layer_size = this->getContentSize();
    auto x = this->getContentSize().width/2;
    auto y = this->message_start_y_pos - this->message_now_line * (FONT_SIZE + SPACE);
    
    // 文書を生成
    auto label = Label::createWithTTF(message, "fonts/misaki_gothic.ttf", FONT_SIZE);
    label->setTextColor(Color4B::WHITE);
    label->setPosition(x, y);
    label->setTag(TAG_MESSAGE_WINDOW_TEXT_0 + this->message_now_line);
    this->addChild(label, ORDER_MESSAGE);
    
    // 流れるようにする
    int letter_num = 0;
    for(int i = 0; i < label->getStringLength() + label->getStringNumLines(); i++) {
        auto letter = label->getLetter(i);
        if(nullptr != letter) {
            letter_num++;
            letter->setVisible(false);
            // 終了時は次のメッセージを取得するコールバックを設定
            if (letter_num == label->getStringLength()) {
                auto callback = CallFuncN::create( CC_CALLBACK_1(MessageLayer::_test, this));
                auto seq = Sequence::create(DelayTime::create(LETTER_DELAY * (i+1)), Show::create(), callback, nullptr);
                letter->runAction(seq);
            } else {
                auto seq = Sequence::create(DelayTime::create(LETTER_DELAY * (i+1)), Show::create(), nullptr);
                letter->runAction(seq);
            }
        }
    }
}

bool MessageLayer::onTouchBegan(Touch *touch, Event *unused_event)
{
    if (this->is_disp_br_cursor) {
        // タッチしたら次のメッセージを読む
        this->_test(nullptr);
        this->is_disp_br_cursor = false;
    }
	return true;
}

void MessageLayer::onTouchMoved(Touch *touch, Event *unused_event) {
}

void MessageLayer::onTouchEnded(Touch *tounc, Event *unused_event) {
}

void MessageLayer::onTouchCancelled(Touch *tounch, Event *unused_event) {
}


