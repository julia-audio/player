#ifndef UI_H
#define UI_H
#include "miniaudio.h"
extern int is_mute;
extern ma_device device;

void init_terminal();
void draw(char *title);
void draw_playback_ui(char *title);
void draw_helper_ui();

#endif
