#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <taglib/tag_c.h>
#include "miniaudio.h"

void print_usage() {
	printf("Usage: player [OPTIONS] <.wav file>\n");
	printf("Options:\n");
	printf("  -v, --verbose  show wav header information\n");
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

int main(int argc, char *argv[]) {
	int verbose = 0;
	char *filename = NULL;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
			verbose = 1;
		} else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
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

	FILE *file = fopen(filename, "rb");
	if (!file) {
		perror("failed open file");
		return 1;
	}

	// play sound
	ma_result result;
	ma_decoder decoder;
	ma_device_config deviceConfig;
	ma_device device;

	result = ma_decoder_init_file(filename, NULL, &decoder);
	if (result != MA_SUCCESS) {
		return -2;
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
		return -3;
	}

	if (ma_device_start(&device) != MA_SUCCESS) {
		printf("Failed to start playback device.\n");
		
		ma_device_uninit(&device);
		ma_decoder_uninit(&decoder);
		return -4;
	}

	// information print
	TagLib_File *tag_file = taglib_file_new(filename);
	const TagLib_Tag *tag = taglib_file_tag(tag_file);

	if (tag == NULL) {
		fprintf(stderr, "No tag found in file. \n");
		taglib_file_free(tag_file);
	}

	if (verbose) {
		printf("Title: %s\n", taglib_tag_title(tag));
		printf("Artist: %s\n", taglib_tag_artist(tag));
		printf("Album: %s\n", taglib_tag_album(tag));
		printf("Year: %d\n", taglib_tag_year(tag));
		printf("Track: %d\n", taglib_tag_track(tag));
		printf("Genre: %s\n", taglib_tag_genre(tag));

		printf("channels: %i\n", decoder.outputChannels);
		printf("sample rate: %i\n", decoder.outputSampleRate);
	}

	printf("Press Enter to quit...");
	getchar();

	// cleanup
	ma_device_uninit(&device);
	ma_decoder_uninit(&decoder);
	taglib_file_free(tag_file);
	
	fclose(file);

	return 0;
}
