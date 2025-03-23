#ifndef PLYAER_H
#define PLYAER_H
#include "miniaudio.h"
extern int is_mute;
extern int is_paused;
extern float volume;
extern ma_decoder decoder;
extern ma_device device;

void play(char *filename);
void increase_volume();
void decrease_volume();
void mute_toggle();
void seek(int seconds);

#endif
