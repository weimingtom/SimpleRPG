#include "MessageLayer.h"

#include "extensions/cocos-ext.h"

#include "../Common.h"

USING_NS_CC;
USING_NS_CC_EXT;

#define IS_DEBUG 1

#define kModalLayerPriority -1

const int FONT_SIZE = 24;
const int SPACE = FONT_SIZE/2;

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
	listener->onTouchBegan = [](Touch *touch,Event*event)->bool{
		return true;
	};
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
    
    // 4ラインまで
    this->message_now_line++;
    if (this->message_now_line > 4) {
        this->message_now_line = 4;
        // 一番上を削除し、他ラインのタグを更新する
        this->removeChildByTag(TAG_MESSAGE_WINDOW_TEXT_1);
        for (int tag = TAG_MESSAGE_WINDOW_TEXT_2; tag <= TAG_MESSAGE_WINDOW_TEXT_4; tag++) {
            auto _message = this->getChildByTag(tag);
            auto _tag = _message->getTag();
            _message->setTag(_tag - 1);
            // 上に移動する
            auto move_by = MoveBy::create(0.2f, Vec2(0.0f, FONT_SIZE + SPACE));
            _message->runAction(move_by);
        }
    }
    
    // 1ラインずつ取得する
    std::string message = this->message_tests.back();
    this->message_tests.pop_back();
    
    this->_make_message(message);
    
    // はい、いいえ　：　もしくは▼のタイミングで状態をリセットして設定する。
    
}

void MessageLayer::_make_message(std::string message) {
    
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
            if (letter_num == label->getStringLength()) {
                auto callback = CallFuncN::create( CC_CALLBACK_1(MessageLayer::_test, this));
                auto seq = Sequence::create(DelayTime::create(0.2f * (i+1)), Show::create(), callback, nullptr);
                letter->runAction(seq);
                CCLOG("end txt");
            } else {
                auto seq = Sequence::create(DelayTime::create(0.2f * (i+1)), Show::create(), nullptr);
                letter->runAction(seq);
                CCLOG("go txt");
            }
        }
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


