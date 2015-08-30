//
//  GameManager.h
//
//  Created by shamaton.
//
//

#ifndef __GameManager__
#define __GameManager__

#include "Common.h"

class GameManager {
private:
	E_SE_TYPE se_type;
	
	// 単独での生成やコピーをさせないため、コンストラクタをprivateへ
	GameManager();
	GameManager(const GameManager& obj);
	GameManager& operator=(const GameManager& obj);
	
	~GameManager();
	
public:
	// singleton
	static GameManager* getInstance(void);
	
	void set_se_type(E_SE_TYPE se_type);
	E_SE_TYPE get_se_type();
	
};

#endif /* defined(__GameManager__) */
