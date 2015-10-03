#include "MusicFade.h"
#include "SimpleAudioEngine.h"

using namespace cocos2d;
using namespace CocosDenshion;

MusicFade::MusicFade()
{
	m_initialVal = 0;
	m_targetVal = 0;
}

MusicFade* MusicFade::create(float duration, float volume, bool pauseOnComplete)
{
	MusicFade *pAction = new MusicFade();
	pAction->initWithDuration(duration, volume, pauseOnComplete);
	pAction->autorelease();
	
	return pAction;
}

bool MusicFade::initWithDuration(float duration, float volume, bool pauseOnComplete)
{
	if (CCActionInterval::initWithDuration(duration))
	{
		m_targetVal = volume;
		m_bPauseOnComplete = pauseOnComplete;
		return true;
	}
	
	return false;
}

void MusicFade::update(float time)
{
	float vol = m_initialVal + time*(m_targetVal - m_initialVal);
	SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(vol);
}

MusicFade* MusicFade::clone(void) const
{
	// no copy constructor
	auto a = new MusicFade();
	a->initWithDuration(_duration, m_targetVal, m_bPauseOnComplete);
	a->autorelease();
	return a;
}

MusicFade* MusicFade::reverse() const
{
	return MusicFade::create(_duration, m_targetVal);
}

void MusicFade::startWithTarget(Node *pTarget)
{
	CCActionInterval::startWithTarget( pTarget );
	m_initialVal = SimpleAudioEngine::getInstance()->getBackgroundMusicVolume();
}

void MusicFade::stop(void)
{
	if(m_bPauseOnComplete) SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
	CCActionInterval::stop();
}