//
//  Sound.h
//  
//
//  Created by shamaton.
//
//

#ifndef __Choukyu__Sound__
#define __Choukyu__Sound__

#include <iostream>

#define BGM_VOLUME_MAX     1.0f
#define BGM_VOLUME_DEFAULT 0.5f
#define BGM_VOLUME_MIN     0.0f

// se
void preload_se(std::string filename);
void play_se(std::string filename);

void set_se_volume(float volume);

// bgm
void preload_bgm(std::string filename);
void play_bgm(std::string filename);
void pause_bgm();
void resume_bgm();
void rewind_bgm();
bool is_playing_bgm();

void set_bgm_volume(float volume);


#endif /* defined(__Choukyu__Sound__) */
