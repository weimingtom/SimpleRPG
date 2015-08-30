#include "AppDelegate.h"
#include "Map/MapScene.h"

#include "GameManager.h"

USING_NS_CC;

AppDelegate::AppDelegate() {

}

AppDelegate::~AppDelegate() 
{
}

bool AppDelegate::applicationDidFinishLaunching() {
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
        glview = GLView::create("My Game");
        director->setOpenGLView(glview);
    }
    
    // チラツキ対応
    director->getInstance()->setProjection(Director::Projection::_2D);
    director->getInstance()->setDepthTest(false);
    
    
    glview->setDesignResolutionSize(GAME_RESOLUTION.width, GAME_RESOLUTION.height, ResolutionPolicy::SHOW_ALL);
    
    //glview->setDesignResolutionSize(800, 320, ResolutionPolicy::SHOW_ALL);
    //director->setContentScaleFactor(2.0f);

    // turn on display FPS
    director->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0 / 60);
    
    // とりあえずの初期化
    auto gm = GameManager::getInstance();
    gm->set_load_map_name("test");
    gm->set_map_init_position(9, 9);

    // create a scene. it's an autorelease object
    auto scene = MapScene::createScene();

    // run
    director->runWithScene(scene);

    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
