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

  if (is_helper_open) {
    const char *msg1 = "| help         : h             |";
    const char *msg2 = "| stop         : space         |";
    const char *msg3 = "| quit         : q             |";
    const char *msg4 = "| change volume: arrow up/down |";
    const char *msg5 = "| mute         : m             |";

    mvprintw(rows - 5, cols - 32, "%s", msg1);
    mvprintw(rows - 4, cols - 32, "%s", msg2);
    mvprintw(rows - 3, cols - 32, "%s", msg3);
    mvprintw(rows - 2, cols - 32, "%s", msg4);
    mvprintw(rows - 1, cols - 32, "%s", msg5);
    refresh();
  } else {
    for (int i = 1; i < 6; i++) {
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

    if (ch == 'm') {
      if (is_mute) {
        is_mute = 0;
      } else {
        is_mute = 1;
      }
      draw_playback_ui(title);
    }

    draw_playback_ui(title);
  }

  endwin();
}
