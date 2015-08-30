//
//  GameManager.cpp
//
//  Created by shamaton.
//
//

#include "GameManager.h"

GameManager* GameManager::getInstance(void) {
	static GameManager _instance;
	return &_instance;
}

GameManager::GameManager() {
	
}

GameManager::~GameManager() {
	
}
//---------------------------------------------------------
// サウンド管理
//---------------------------------------------------------
void GameManager::set_se_type(E_SE_TYPE se_type) {
	this->se_type = se_type;
}

E_SE_TYPE GameManager::get_se_type() {
	return this->se_type;
}
