#include "miniaudio.h"
#include "player.h"
#include "ui.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <taglib/tag_c.h>

int is_mute = 0;
int is_paused = 0;
float volume = 1.0f;

ma_decoder decoder;
ma_device device;

void print_usage() {
  printf("Usage: player [OPTIONS] <file>\n");
  printf("Options:\n");
  printf("  -h, --help     show this help message\n");
}

void audio_clean() {
  ma_device_uninit(&device);
  ma_decoder_uninit(&decoder);
}

int main(int argc, char *argv[]) {
  char *filename = NULL;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      print_usage();
      return 0;
    } else if (argv[i][0] == '-') {
      print_usage();
      return 1;
    } else {
      filename = argv[i];
    }
  }

  if (!filename) {
    fprintf(stderr, "ERROR, No File\n");
    print_usage();
    return 1;
  }

  TagLib_File *file = taglib_file_new(filename);
  const TagLib_Tag *tag = taglib_file_tag(file);
  char *title;

  if (tag == NULL) {
    title = filename;
    taglib_file_free(file);
  }

  title = taglib_tag_title(tag);
  if (strcmp(title, "") == 0) {
    title = filename;
  }

  init_terminal();
  play(filename);
  draw(title);

  audio_clean();
  taglib_file_free(file);

  return 0;
}
