#include "MessageLayer.h"

#include "extensions/cocos-ext.h"

#include "../Common.h"

USING_NS_CC;
USING_NS_CC_EXT;
using namespace std;

#define IS_DEBUG 1

#define kModalLayerPriority -1

const int FONT_SIZE = 24;
const int SPACE = FONT_SIZE/2;
const int MAX_LINE = 4;

const float LETTER_DELAY = 0.1f;
const float MOVE_TIME = 0.5f;

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
    // 交互に使う
    TAG_MESSAGE_WINDOW_TEXT_A_1,
    TAG_MESSAGE_WINDOW_TEXT_A_2,
    TAG_MESSAGE_WINDOW_TEXT_A_3,
    TAG_MESSAGE_WINDOW_TEXT_A_4,
    TAG_MESSAGE_WINDOW_TEXT_B_1,
    TAG_MESSAGE_WINDOW_TEXT_B_2,
    TAG_MESSAGE_WINDOW_TEXT_B_3,
    TAG_MESSAGE_WINDOW_TEXT_B_4,
    TAG_MESSAGE_BR,
    TAG_CLIPPING,
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
    auto pScale = ui::Scale9Sprite::create(RES_COMMON_DIR + "window.png", Rect(0, 0, 64, 64), Rect(10, 10, 44, 44));
    pScale->setContentSize(window_size);
    pScale->setPosition(base_position);
    pScale->setTag(TAG_MESSAGE_WINDOW);

    this->addChild(pScale, ORDER_MESSAGE_WINDOW);

	this->scheduleUpdate();
    
    // 位置を取得して、ラベルを作成しておく
    this->ab_text = "a";
    this->fixed_text_y_move_amount = (MAX_LINE+1) * (FONT_SIZE + SPACE);
    
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
    
    this->_make_mask();

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
        auto yesno_window = ui::Scale9Sprite::create(RES_COMMON_DIR + "window.png", Rect(0, 0, 64, 64), Rect(10, 10, 44, 44));
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
    //std::copy(messages.begin(), messages.end(), std::back_inserter(this->messages));
    this->messages = messages;
    CCLOG("set message!!");
    
    auto cliping = this->getChildByTag(TAG_CLIPPING);
    
    for (int t = TAG_MESSAGE_WINDOW_TEXT_A_1, i = 0; t <= TAG_MESSAGE_WINDOW_TEXT_B_4; t++, i++) {
        auto label = (Label *)cliping->getChildByTag(t);
        label->setString("");
    }
    
    this->_set_touch_enabled(true);
    this->setVisible(true);
    
    this->message_total_count = 0;
    this->_proc_message(true);
}

//---------------------------------------------------------
// 後始末
//---------------------------------------------------------
void MessageLayer::_finalize(cocos2d::Node *sender) {
    
    this->is_end_line = false;
    this->is_disp_br_cursor = false;
    
    this->message_total_count = 0;
    
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
    vector<string> test = {
    "a",
    "b"};
    
    
    // 最初に文字セット
    
    
}

void MessageLayer::_disp_lines(std::vector<std::string> lines, bool is_init) {
    
    auto move_amount = (is_init) ? 0 : this->fixed_text_y_move_amount;
    CCLOG("amount %f", move_amount);
    
    auto cliping = (ClippingNode *)this->getChildByTag(TAG_CLIPPING);
    
    // 全ラインを送って、文字を入れ替える
    // 現状の文字を送り出す
    int start_tag = (this->ab_text == "a") ? TAG_MESSAGE_WINDOW_TEXT_A_1 : TAG_MESSAGE_WINDOW_TEXT_B_1;
    for (int tag = start_tag; tag < start_tag + MAX_LINE; tag++) {
        auto _message = cliping->getChildByTag(tag);
        // 上に移動する
        auto move_by = MoveBy::create(MOVE_TIME, Vec2(0.0f, move_amount));
        _message->runAction(move_by);
    }
    
    // 次に表示する文字
    int new_start_tag = (this->ab_text == "a") ? TAG_MESSAGE_WINDOW_TEXT_B_1 : TAG_MESSAGE_WINDOW_TEXT_A_1;
    for (int tag = new_start_tag, index = 0; tag < new_start_tag + MAX_LINE; tag++, index++) {
        auto _message = (Label *)cliping->getChildByTag(tag);
        // 新しい文字を設定
        string line = (index < lines.size()) ? lines[index] : "";
        _message->setString(line);
        // 見えない位置から移動してくる
        _message->setPositionY(this->fixed_line_y_positions[index] - move_amount);
        auto move_by = MoveBy::create(MOVE_TIME, Vec2(0.0f, move_amount));
        Sequence *seq;
        if (index == 0) {
            auto callback = CallFuncN::create( CC_CALLBACK_1(MessageLayer::_callback_event, this));
            seq = Sequence::create(move_by, callback, nullptr);
        }
        else {
            seq = Sequence::create(move_by, nullptr);
        }
        _message->runAction(seq);
    }
    
    // abを更新
    this->ab_text = (this->ab_text == "a") ? "b" : "a";
}

//---------------------------------------------------------
// テキスト内容の把握
//---------------------------------------------------------
void MessageLayer::_proc_message(bool is_init) {
    
    vector<string> lines;
    int now_count = 0;
    this->event_line = "";
    
    CCLOG("proc_message");
    
    while (now_count < MAX_LINE || this->message_total_count < this->messages.size()) {
        CCLOG("now %d", this->message_total_count);
        std::string line = this->messages[this->message_total_count];
        this->message_total_count++;
        // イベントメッセージ
        if (line.find(";") != std::string::npos) {
            CCLOG("event!!");
            this->event_line = line;
            break;
        }
        // セリフ
        else {
            lines.push_back(line);
            now_count++;
        }
    }
    // イベントがなかった場合
    if (this->event_line.empty()) {
        if (this->message_total_count >= this->messages.size()) {
            this->event_line = "end;";
        }
        else {
            this->event_line = "br;";
        }
    }
    // メッセージをセット、ない場合はイベント処理
    if (lines.size()) {
        this->_disp_lines(lines, is_init);
    }
    else {
        this->_callback_event(nullptr);
    }
}

void MessageLayer::_callback_event(Node *sender) {
    
    if (this->event_line.find("br;") != std::string::npos) {
        CCLOG("br!!");
        // カーソルを表示する
        this->_set_br();
    }
    // YES or NO
    else if (this->event_line.find("yes_no;") != std::string::npos) {
        CCLOG("find yes or now");
        this->_set_yesno(this->event_line);
    }
    // 指定の行数にとぶ
    else if (this->event_line.find("jump;") != std::string::npos) {
        CCLOG("jump");
        this->_set_jump(this->event_line);
    }
    // 途中だけど終了
    else if (this->event_line.find("end;") != std::string::npos) {
        this->_set_end();
    }
    else {
        assert(0);
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
    this->message_total_count = jump_line;
    this->_proc_message();
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
    this->message_total_count = this->yesno_line[yesno];
    this->_proc_message();
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
        this->_proc_message();
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


//---------------------------------------------------------
// メッセージを流す場所
//---------------------------------------------------------
void MessageLayer::_make_mask() {
    
    auto layer_size = this->getContentSize();
    
    auto base_position = Vec2(layer_size.width/2, layer_size.height/4);
    
    // クリッピングマスク用のテクスチャ作成
    // NOTE:addChildしないものはretainする
    auto render_tex = RenderTexture::create(layer_size.width, layer_size.height);
    render_tex->retain();
    {
        render_tex->begin();
        
        // 表示しない領域(透明画像)
        auto stencil = Sprite::create();
        stencil->setTextureRect(Rect(0.0f ,0.0f ,layer_size.width, 200 * 2));
        stencil->setColor(Color3B::GREEN);
        stencil->setOpacity(0);
        stencil->setPosition(layer_size.width/2, layer_size.height/2);
        stencil->retain();
        
        // 表示する領域(黒)
        auto disp = Sprite::create();
        disp->setTextureRect(Rect(0.0f ,0.0f ,layer_size.width - 50, 180));
        disp->setColor(Color3B::BLACK);
        disp->setPosition(layer_size.width/2, layer_size.height/2);
        disp->retain();
        
        // render_texに焼き付ける
        stencil->visit();
        disp->visit();
        
        render_tex->end();
    }
    // 生成したテクスチャでスプライト作成
    auto klone = Sprite::createWithTexture(render_tex->getSprite()->getTexture());
    klone->setPosition(base_position);
    
    // クリッピング処理
    auto clipping = ClippingNode::create();
    clipping->setAnchorPoint(Vec2(0, 0));
    clipping->setPosition(0, 0);
    
    // マスク領域
    clipping->setStencil(klone);
    clipping->setInverted(false); // stencilに設定した領域を表示する(黒い部分)
    clipping->setAlphaThreshold(0.01f);
    
    // クリッピングして表示したいもの
    auto x = this->getContentSize().width/2 - 180;
    float line_start_y_pos = base_position.y + (FONT_SIZE + SPACE) * 1 + SPACE;
    for (int t = TAG_MESSAGE_WINDOW_TEXT_A_1, i = 0; t <= TAG_MESSAGE_WINDOW_TEXT_B_4; t++, i++) {
        float pos = line_start_y_pos - i * (FONT_SIZE + SPACE);
        this->fixed_line_y_positions.push_back(pos);
        // ラベル
        auto label = Label::createWithTTF("", "fonts/misaki_gothic.ttf", FONT_SIZE);
        label->setTextColor(Color4B::WHITE);
        label->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
        label->setPosition(x, pos);
        clipping->addChild(label, ORDER_MESSAGE, t);
    }
    this->addChild(clipping, ORDER_MESSAGE, TAG_CLIPPING);
}

