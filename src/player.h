#ifndef PLYAER_H
#define PLYAER_H
#include "miniaudio.h"
extern int is_mute;
extern int is_paused;
extern float volume;
extern ma_decoder decoder;
extern ma_device device;

void data_callback(ma_device *pDevice, void *pOutput, const void *pInput,
                   ma_uint32 frameCount);
void play(char *filename);
void increase_volume();
void decrease_volume();
void mute_toggle();

#endif
