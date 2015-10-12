//
//  MessageWindow.cpp
//  Choukyu
//
//  Created by shamaton
//
//

#include "MessageWindow.h"

#include "Common.h"

USING_NS_CC;

enum {
    TAG_WINDOW,
    TAG_WINDOW_BG,
    TAG_MESSAGE
};

//---------------------------------------------------------
// 初期化
//---------------------------------------------------------
MessageWindow* MessageWindow::create() {
    
    auto mw = new MessageWindow();
    if (mw)
    {
        mw->_init();
        mw->autorelease();
        mw->retain();
        return mw;
    }
    
    CC_SAFE_DELETE(mw);
    return NULL;
}

void MessageWindow::_init() {
	
	// window画像
	auto img_window    = Sprite::create(RES_BATTLE_DIR + "window_waku.png");
	auto img_window_bg = Sprite::create(RES_BATTLE_DIR + "window_waku_bg.png");
	
	auto font_size = 20;
	TTFConfig ttfConfig("fonts/misaki_gothic.ttf",
						font_size,
						GlyphCollection::DYNAMIC);
	
	// message
	auto message_text = Label::createWithTTF(ttfConfig, "");
	
	this->addChild(img_window_bg, 1, TAG_WINDOW_BG);
	this->addChild(img_window,    2, TAG_WINDOW);
	this->addChild(message_text,  3, TAG_MESSAGE);
}

//---------------------------------------------------------
// メッセージ設定
//---------------------------------------------------------
void MessageWindow::set_message(std::string message) {
	auto message_label = (Label *)this->getChildByTag(TAG_MESSAGE);
	message_label->setString(message);
}

//---------------------------------------------------------
// タイマーで設定した秒数表示する
//---------------------------------------------------------
void MessageWindow::set_disp_timer(float time) {
	setVisible(true);
	// 何も動かない
	auto disp = MoveTo::create(time, getPosition());
	
	// 直列実行
    auto seq = Sequence::create(disp, Hide::create(), nullptr);
	runAction(seq);
}

//---------------------------------------------------------
// ウインドウサイズ取得
//---------------------------------------------------------
Size MessageWindow::get_img_size() {
	auto img = (Sprite *)this->getChildByTag(TAG_WINDOW);
	return img->getContentSize();
}