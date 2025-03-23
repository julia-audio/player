#include "miniaudio.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <taglib/tag_c.h>

float volume = 1.0f;
int is_paused = 0;
int is_helper_open = 0;
ma_decoder decoder;
ma_device device;

void print_usage() {
  printf("Usage: player [OPTIONS] <file>\n");
  printf("Options:\n");
  printf("  -h, --help     show this help message\n");
}

void data_callback(ma_device *pDevice, void *pOutput, const void *pInput,
                   ma_uint32 frameCount) {
  ma_decoder *pDecoder = (ma_decoder *)pDevice->pUserData;
  if (pDecoder == NULL) {
    return;
  }

  ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

  float *buffer = (float *)pOutput;
  ma_uint32 channels = pDevice->playback.channels;

  for (ma_uint32 i = 0; i < frameCount * channels; ++i) {
    buffer[i] *= volume;
  }

  (void)pInput;
}

void play(char *filename) {
  ma_result result;
  ma_device_config deviceConfig;

  result = ma_decoder_init_file(filename, NULL, &decoder);
  if (result != MA_SUCCESS) {
    exit(EXIT_FAILURE);
  }

  deviceConfig = ma_device_config_init(ma_device_type_playback);

  deviceConfig.playback.format = decoder.outputFormat;
  deviceConfig.playback.channels = decoder.outputChannels;
  deviceConfig.sampleRate = decoder.outputSampleRate;
  deviceConfig.dataCallback = data_callback;
  deviceConfig.pUserData = &decoder;

  if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
    printf("Failed to open playback device.\n");
    ma_decoder_uninit(&decoder);
    exit(EXIT_FAILURE);
  }

  if (ma_device_start(&device) != MA_SUCCESS) {
    printf("Failed to start playback device.\n");

    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);
    exit(EXIT_FAILURE);
  }
}

void audio_clean() {
  ma_device_uninit(&device);
  ma_decoder_uninit(&decoder);
}

void init_terminal() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  curs_set(0);
}

void increase_volume() {
  volume += 0.05f;
  if (volume > 1.0f)
    volume = 1.0f;
}

void decrease_volume() {
  volume -= 0.05f;
  if (volume < 0.0f)
    volume = 0.0f;
}

void print_playback_ui(char *title) {
  mvprintw(0, 0, "Now Playing: %s", title);
  move(1, 0);
  clrtoeol();
  mvprintw(1, 0, "Volume: %d%%", (int)(volume * 100.0f + 0.5f));
  refresh();
}

void print_helper_ui() {
  int rows, cols;
  getmaxyx(stdscr, rows, cols);

  if (is_helper_open) {
    const char *msg1 = "| help         : h             |";
    const char *msg2 = "| stop         : space         |";
    const char *msg3 = "| quit         : q             |";
    const char *msg4 = "| change volume: arrow up/down |";

    mvprintw(rows - 4, cols - 32, "%s", msg1);
    mvprintw(rows - 3, cols - 32, "%s", msg2);
    mvprintw(rows - 2, cols - 32, "%s", msg3);
    mvprintw(rows - 1, cols - 32, "%s", msg4);
    refresh();
  } else {
    for (int i = 1; i < 5; i++) {
      move(rows - i, cols - 32);
      clrtoeol();
    }
    refresh();

    const char *msg = "help: h";
    mvprintw(rows - 1, cols - strlen(msg), "%s", msg);
    refresh();
  }
}

void draw(char *title) {
  print_helper_ui();

  int ch;
  while ((ch = getch()) != 'q') {
    if (ch == ' ') {
      if (is_paused) {
        ma_device_start(&device);
        is_paused = 0;
      } else {
        ma_device_stop(&device);
        is_paused = 1;
      }
    }

    if (ch == 'h') {
      if (is_helper_open) {
        is_helper_open = 0;
        print_helper_ui();
      } else {
        is_helper_open = 1;
        print_helper_ui();
      }
    }

    if (ch == KEY_UP) {
      increase_volume();
      print_playback_ui(title);
    }

    if (ch == KEY_DOWN) {
      decrease_volume();
      print_playback_ui(title);
    }

    print_playback_ui(title);
  }

  endwin();
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
