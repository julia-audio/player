#include <stdint.h>
#include <stdio.h>
#include "wav_reader.h"


int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <.wav file>\n", argv[0]);
    return 1;
  }

	FILE *file = fopen(argv[1], "rb");
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

	print_wav_header_info(wh);

	fclose(file);

	return 0;
}
