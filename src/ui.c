#include "miniaudio.h"
#include "player.h"
#include "ui.h"
#include <ncurses.h>
#include <string.h>

int is_helper_open = 0;

void init_terminal() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  curs_set(0);
}

void draw_playback_ui(char *title) {
  mvprintw(0, 0, "Now Playing: %s", title);
  move(1, 0);
  clrtoeol();

  if (is_mute) {
    mvprintw(1, 0, "Volume: mute");
  } else {
    mvprintw(1, 0, "Volume: %d%%", (int)(volume * 100.0f + 0.5f));
  }
  refresh();
}

void draw_helper_ui() {
  int rows, cols;
  getmaxyx(stdscr, rows, cols);

  const char *help_msgs[] = {"help: h",
                             "pause & resume: space",
                             "quit: q",
                             "volume up: arrow up",
                             "volume down: arrow down",
                             "mute toggle: m",
                             "seek -5 sec: arrow left",
                             "seek +5 sec: arrow right"};

  int num_msgs = sizeof(help_msgs) / sizeof(help_msgs[0]);

  if (is_helper_open) {
    for (int i = 0; i < num_msgs; i++) {
      mvprintw(rows - (num_msgs - i), cols - 32, "%s", help_msgs[i]);
    }
    refresh();
  } else {
    for (int i = 0; i < num_msgs; i++) {
      move(rows - (num_msgs - i), cols - 32);
      clrtoeol();
    }
    refresh();

    const char *msg = "help: h";
    mvprintw(rows - 1, cols - strlen(msg), "%s", msg);
    refresh();
  }
}

void format_time(ma_uint64 frames, ma_uint32 sample_rate, char *out,
                 size_t out_size) {
  ma_uint64 seconds = frames / sample_rate;
  snprintf(out, out_size, "%02llu:%02llu", seconds / 60, seconds % 60);
}

void draw_position() {
  ma_uint64 cursor_frame = 0;
  ma_uint64 total_frame = 0;
  ma_decoder_get_cursor_in_pcm_frames(&decoder, &cursor_frame);
  ma_decoder_get_length_in_pcm_frames(&decoder, &total_frame);

  int percent = 0;
  if (total_frame > 0) {
    percent = (int)((cursor_frame * 100) / total_frame);
  }

  char current_time[16];
  char total_time[16];

  format_time(cursor_frame, decoder.outputSampleRate, current_time,
              sizeof(current_time));
  format_time(total_frame, decoder.outputSampleRate, total_time,
              sizeof(total_time));

  mvprintw(3, 10, "%s / %s", current_time, total_time);

  int bar_width = 30;
  int filled = (percent * bar_width) / 100;

  mvprintw(4, 0, "[");
  for (int i = 0; i < bar_width; ++i) {
    if (i < filled) {
      addch('#');
    } else {
      addch('-');
    }
  }
  printw("] %d%%", percent);
}

void draw(char *title) {
  draw_helper_ui();

  int ch;
  while ((ch = getch()) != 'q') {
    if (ch == ' ') {
      mute_toggle();
    }

    if (ch == 'h') {
      if (is_helper_open) {
        is_helper_open = 0;
        draw_helper_ui();
      } else {
        is_helper_open = 1;
        draw_helper_ui();
      }
    }

    if (ch == KEY_UP) {
      increase_volume();
      draw_playback_ui(title);
    }

    if (ch == KEY_DOWN) {
      decrease_volume();
      draw_playback_ui(title);
    }

    if (ch == KEY_LEFT)
      seek(-5);

    if (ch == KEY_RIGHT)
      seek(5);

    if (ch == 'm') {
      if (is_mute) {
        is_mute = 0;
      } else {
        is_mute = 1;
      }
      draw_playback_ui(title);
    }

    draw_playback_ui(title);
    draw_position();
  }

  endwin();
}
