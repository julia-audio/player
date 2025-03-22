#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <taglib/tag_c.h>
#include "miniaudio.h"

int is_paused = 0;
ma_decoder decoder;
ma_device device;


void print_usage() {
	printf("Usage: player [OPTIONS] <file>\n");
	printf("Options:\n");
	printf("  -h, --help     show this help message\n");
}

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
	ma_decoder* pDecoder = (ma_decoder *)pDevice->pUserData;
	if (pDecoder == NULL) {
		return;
	}

	ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

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

	deviceConfig.playback.format   = decoder.outputFormat;
	deviceConfig.playback.channels = decoder.outputChannels;
	deviceConfig.sampleRate        = decoder.outputSampleRate;
	deviceConfig.dataCallback      = data_callback;
	deviceConfig.pUserData         = &decoder;

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

void print_playback_ui(char *title) {
	mvprintw(0, 0, "Now Playing: %s", title);
	refresh();
}

void draw(char *title) {
	int ch;
	while((ch = getch()) != 'q') {
		if (ch == ' ') {
			if (is_paused) {
				ma_device_start(&device);
				is_paused = 0;
			} else {
				ma_device_stop(&device);
				is_paused = 1;
			}
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
