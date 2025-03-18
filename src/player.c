#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "wav_reader.h"

void print_usage() {
	printf("Usage: player [OPTIONS] <.wav file>\n");
	printf("Options:\n");
	printf("  -v, --verbose  show wav header information\n");
	printf("  -h, --help     show this help message\n");
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

	if (check_file_format(file) != 0) {
		fprintf(stderr, "failed file format check \n");
		fclose(file);
		return 1;
	}

	rewind(file);

	struct wav_header wh;
	wh.riff_header = read_riff_header(file);
	wh.fmt_subchunk = read_fmt_subchunk(file);
	wh.data_subchunk = read_data_subchunk(file);

	if (verbose)
		print_wav_header_info(wh);

	fclose(file);

	return 0;
}
