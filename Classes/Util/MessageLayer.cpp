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
enum E_ORDER_MESSAGE_LAYER {
    ORDER_MESSAGE_WINDOW,
    ORDER_MESSAGE,
    ORDER_BUTTON,
    ORDER_BUTTON_LABEL,
	NR_ORDER
};

enum E_TAG_MESSAGE_LAYER {
    TAG_MESSAGE_WINDOW,
    TAG_MESSAGE_WINDOW_TEXT_0, // 開始位置としてのみ利用する
    TAG_MESSAGE_WINDOW_TEXT_1,
    TAG_MESSAGE_WINDOW_TEXT_2,
    TAG_MESSAGE_WINDOW_TEXT_3,
    TAG_MESSAGE_WINDOW_TEXT_4,
    TAG_MESSAGE_BR,
    TAG_YES_BUTTON,
    TAG_YES_BUTTON_LABEL,
    TAG_NO_BUTTON,
    TAG_NO_BUTTON_LABEL,
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
	
	
	auto layer_size = this->getContentSize();
	auto base_position = Vec2(layer_size.width/2, layer_size.height/4);
    
    Size window_size = Size(layer_size.width - 50, 200);
    Scale9Sprite* pScale = Scale9Sprite::create(RES_COMMON_DIR + "window.png", Rect(0, 0, 64, 64), Rect(10, 10, 44, 44));
    pScale->setContentSize(window_size);
    pScale->setPosition(base_position);
    pScale->setTag(TAG_MESSAGE_WINDOW);

    this->addChild(pScale, ORDER_MESSAGE_WINDOW);

	this->scheduleUpdate();
    
    this->message_start_y_pos = base_position.y + (FONT_SIZE + SPACE) * 2 + SPACE;
    
    // 改行演出
    auto delay = 0.5f;
    auto br = Sprite::create(RES_COMMON_DIR + "message_br.png");
    br->setPosition(base_position.x + 200, base_position.y - 80);
    
    auto visible = FadeIn::create(.0f);
    auto invisivle = FadeOut::create(.0f);
    auto seq = Sequence::create(visible, DelayTime::create(delay), invisivle, DelayTime::create(delay), nullptr);
    auto rep = RepeatForever::create(seq);
    br->runAction(rep);
    br->setTag(TAG_MESSAGE_BR);
    br->setVisible(false);
    
    this->addChild(br, ORDER_MESSAGE);
    
    this->setVisible(false);
    
    // YES NOボタン
    this->_init_yesno();

    return true;
}

//---------------------------------------------------------
// YES NOボタン作成
//---------------------------------------------------------
void MessageLayer::_init_yesno() {
    
    Size yesno_window_size = Size(100, 50);
    
    auto yes_func = CC_CALLBACK_1(MessageLayer::_push_yes, this);
    auto no_func = CC_CALLBACK_1(MessageLayer::_push_no, this);
    
    auto layer_size = this->getContentSize();
    float xs[2] = { layer_size.width/2 - 80, layer_size.width/2 + 80};
    float y = layer_size.height/8;
    int tags[2] = { TAG_YES_BUTTON, TAG_NO_BUTTON};
    
    std::string text[2] = {"はい", "いいえ"};
    int text_tags[2] = { TAG_YES_BUTTON_LABEL, TAG_NO_BUTTON_LABEL};
    
    for (int i = 0; i < 2; i++) {
        Scale9Sprite* yesno_window = Scale9Sprite::create(RES_COMMON_DIR + "window.png", Rect(0, 0, 64, 64), Rect(10, 10, 44, 44));
        yesno_window->setContentSize(yesno_window_size);
        
        auto item = MenuItemSprite::create(yesno_window, yesno_window, i == 1 ? no_func : yes_func);
        item->setPosition(xs[i], y);
        
        auto menu = Menu::create(item, nullptr);
        menu->setPosition(Vec2::ZERO);
        menu->setTag(tags[i]);
        menu->setVisible(false); // 開始時は消しておく
        this->addChild(menu, ORDER_BUTTON);
        
        // はい、いいえ
        auto label = Label::createWithTTF(text[i], "fonts/misaki_gothic.ttf", FONT_SIZE);
        label->setTextColor(Color4B::WHITE);
        label->setTag(text_tags[i]);
        label->setPosition(xs[i], y - 4);
        label->setVisible(false);
        this->addChild(label, ORDER_BUTTON_LABEL);
    }
}

//---------------------------------------------------------
//
//---------------------------------------------------------
void MessageLayer::_set_touch_enabled(bool enabled)
{
    if (enabled) {
        auto listener = EventListenerTouchOneByOne::create();
        listener->setSwallowTouches(true);
        listener->onTouchBegan = CC_CALLBACK_2(MessageLayer::onTouchBegan, this);
        
        _touchListener = listener;
        _touchListener->retain();
        _eventDispatcher->addEventListenerWithSceneGraphPriority(_touchListener, this);
        _eventDispatcher->setPriority(_touchListener, kModalLayerPriority);
        
    }
    else {
        _eventDispatcher->removeEventListener(_touchListener);
        _touchListener->release();
        _touchListener = nullptr;
    }
}

//---------------------------------------------------------
//
//---------------------------------------------------------
void MessageLayer::set_message(std::vector<std::string> messages, Character *character) {
    this->character = character;
    this->set_message(messages);
}

void MessageLayer::set_message(std::vector<std::string> messages) {
    this->messages = messages;
    
    this->_set_touch_enabled(true);
    this->setVisible(true);
    
    this->_read_line();
}

//---------------------------------------------------------
// 後始末
//---------------------------------------------------------
void MessageLayer::_finalize(cocos2d::Node *sender) {
    
    // 表示済みの文字を削除する
    auto index_tag = TAG_MESSAGE_WINDOW_TEXT_0 + this->message_now_line; // 1以上じゃないとおかしくなる
    for (int tag = TAG_MESSAGE_WINDOW_TEXT_1; tag <= index_tag; tag++) {
        this->removeChildByTag(tag);
    }
    
    this->is_end_line = false;
    this->is_disp_br_cursor = false;
    this->message_now_line = 0;
    this->message_now_count = 0;
    
    this->yesno_line.clear();
    
    this->_set_touch_enabled(false);
    this->setVisible(false);
    
    // 人に話しかけた場合は解除する
    if (this->character != nullptr) {
        this->character->talk_end();
        this->character = nullptr;
    }
}

void MessageLayer::update(float flame) {
}

void MessageLayer::_test() {
}



//---------------------------------------------------------
// 入力からのライン処理
//---------------------------------------------------------
void MessageLayer::_read_line() {
    
    // ライン全部読んだら終わり
    if (this->messages.size() <= this->message_now_count) {
        this->is_end_line = true;
        return;
    }
    
    std::string line = this->messages[this->message_now_count];
    
    this->message_now_count++;
    // ラインの内容で処理を分岐
    this->_proc_line(line);
}

//---------------------------------------------------------
// コールバック時のライン処理
//---------------------------------------------------------
void MessageLayer::_callback_line(Node *sender) {
    
    this->_read_line();
}

//---------------------------------------------------------
// テキスト内容の把握
//---------------------------------------------------------
void MessageLayer::_proc_line(std::string line) {
    
    // メッセージの内容によって、処理を変更
    // 改行
    if (line.length() < 1) {
        CCLOG("br!!");
        // カーソルを表示する
        this->_set_br();
    }
    // YES or NO
    else if (line.find("yes_no;") != std::string::npos) {
        CCLOG("find yes or now");
        this->_set_yesno(line);
    }
    // 指定の行数にとぶ
    else if (line.find("jump;") != std::string::npos) {
        CCLOG("jump");
        this->_set_jump(line);
    }
    // 途中だけど終了
    else if (line.find("end;") != std::string::npos) {
        this->_set_end();
        return;
    }
    else {
        this->_set_line(line);
    }
}

//---------------------------------------------------------
// ラインを流すように設定する
//---------------------------------------------------------
void MessageLayer::_set_line(std::string line) {
    
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
    auto x = this->getContentSize().width/2 - 180;
    auto y = this->message_start_y_pos - this->message_now_line * (FONT_SIZE + SPACE);
    
    // 文書を生成
    auto label = Label::createWithTTF(line, "fonts/misaki_gothic.ttf", FONT_SIZE);
    label->setTextColor(Color4B::WHITE);
    label->setTag(TAG_MESSAGE_WINDOW_TEXT_0 + this->message_now_line);
    this->addChild(label, ORDER_MESSAGE);
    
    label->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    label->setPosition(x, y);
    
    // 流れるようにする
    int letter_num = 0;
    for(int i = 0; i < label->getStringLength() + label->getStringNumLines(); i++) {
        auto letter = label->getLetter(i);
        if(nullptr != letter) {
            letter_num++;
            letter->setVisible(false);
            // 終了時は次のメッセージを取得するコールバックを設定
            if (letter_num == label->getStringLength()) {
                auto callback = CallFuncN::create( CC_CALLBACK_1(MessageLayer::_callback_line, this));
                auto seq = Sequence::create(DelayTime::create(LETTER_DELAY * (i+1)), Show::create(), callback, nullptr);
                letter->runAction(seq);
            } else {
                auto seq = Sequence::create(DelayTime::create(LETTER_DELAY * (i+1)), Show::create(), nullptr);
                letter->runAction(seq);
            }
        }
    }
}

//---------------------------------------------------------
// 改行演出
//---------------------------------------------------------
void MessageLayer::_set_br() {
    this->is_disp_br_cursor = true;
    auto br = this->getChildByTag(TAG_MESSAGE_BR);
    br->setVisible(true);
}

//---------------------------------------------------------
// 指定箇所に飛ぶ
//---------------------------------------------------------
void MessageLayer::_set_jump(std::string line) {
    auto _split = Common::split(line, ';');
    assert(_split.size() == 2);
    
    int jump_line = std::atoi(_split[1].c_str()) - 1;
    assert(jump_line > -1);
    assert(jump_line < 255);
    this->message_now_count = jump_line;
    this->_read_line();
}

//---------------------------------------------------------
// 会話終了
//---------------------------------------------------------
void MessageLayer::_set_end() {
    // 終了フラグ
    this->is_end_line = true;
}

//---------------------------------------------------------
// yes or noライン処理
//---------------------------------------------------------
void MessageLayer::_set_yesno(std::string line) {
    // 規則にそって処理 : yes_no;y_jump,n_jump
    auto _split = Common::split(line, ';');
    auto jumps = Common::split(_split[1], ',');
    
    // 必ず2つある
    assert(_split.size() == 2);
    assert(jumps.size() == 2);
    
    this->yesno_line["yes"] = std::atoi(jumps[0].c_str()) - 1;
    this->yesno_line["no"]  = std::atoi(jumps[1].c_str()) - 1;
    
    // ボタンを表示
    std::vector<int> tags = {TAG_YES_BUTTON, TAG_NO_BUTTON, TAG_YES_BUTTON_LABEL, TAG_NO_BUTTON_LABEL};
    for (auto tag : tags) {
        this->getChildByTag(tag)->setVisible(true);
    }
}

//---------------------------------------------------------
// yes or noを押した
//---------------------------------------------------------
void MessageLayer::_push_yes(Ref* pSender)
{
    CCLOG("yes!!");
    this->_push_yesno("yes");
}
void MessageLayer::_push_no(Ref* pSender)
{
    CCLOG("no!!");
    this->_push_yesno("no");
}
void MessageLayer::_push_yesno(std::string yesno) {
    // 指定の場所に飛ばす
    this->message_now_count = this->yesno_line[yesno];
    this->_read_line();
    // 非表示
    std::vector<int> tags = {TAG_YES_BUTTON, TAG_NO_BUTTON, TAG_YES_BUTTON_LABEL, TAG_NO_BUTTON_LABEL};
    for (auto tag : tags) {
        this->getChildByTag(tag)->setVisible(false);
    }
}


//---------------------------------------------------------
//
//---------------------------------------------------------
bool MessageLayer::onTouchBegan(Touch *touch, Event *unused_event)
{
    if (this->is_disp_br_cursor) {
        // タッチしたら次のメッセージを読む
        this->_read_line();
        this->is_disp_br_cursor = false;
        auto br = this->getChildByTag(TAG_MESSAGE_BR);
        br->setVisible(false);
        return true;
    }
    if (this->is_end_line) {
        this->is_end_line = false;
        auto callback = CallFuncN::create( CC_CALLBACK_1(MessageLayer::_finalize, this));
        auto seq = Sequence::create(DelayTime::create(0.5f), callback, nullptr);
        this->runAction(seq);
        return true;
    }
	return true;
}

void MessageLayer::onTouchMoved(Touch *touch, Event *unused_event) {
}

void MessageLayer::onTouchEnded(Touch *tounc, Event *unused_event) {
}

void MessageLayer::onTouchCancelled(Touch *tounch, Event *unused_event) {
}


