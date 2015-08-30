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
// 
//---------------------------------------------------------
void GameManager::set_load_map_name(std::string name) {
    this->load_map_str = name;
}

std::string GameManager::get_load_map_name() {
    return this->load_map_str;
}