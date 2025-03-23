#ifndef UI_H
#define UI_H
#include "miniaudio.h"
extern int is_mute;
extern ma_device device;
extern ma_decoder decoder;

void init_terminal();
void draw(char *title);

#endif
