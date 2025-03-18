#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "wav_reader.h"

int check_file_format(FILE *fp) {
	char chunk_id[4];
	fread(chunk_id, 4, 1, fp);

	return strncmp(chunk_id, "RIFF", 4);
}

struct riff_header read_riff_header(FILE *fp) {
	struct riff_header rh;

	fread(rh.chunk_id, 4, 1, fp);
	fread(&rh.chunk_size, 4, 1, fp);
	fread(rh.format, 4, 1, fp);

	return rh;
}

struct fmt_subchunk read_fmt_subchunk(FILE *fp) {
	struct fmt_subchunk fs;
	
	while (1) {
		if (fread(fs.subchunk1_id, 4, 1, fp) != 1) {
			fprintf(stderr, "EOF while searching for 'fmt ' subchunk\n");
			exit(EXIT_FAILURE);
		}

		if (strncmp(fs.subchunk1_id, "fmt ", 4) == 0) {
			break;
		}
	}

	while (1) {
		if (fread(&fs.subchunk1_size, 4, 1, fp) != 1) {
			fprintf(stderr, "EOF while searching for subchunk size\n");
			exit(EXIT_FAILURE);
		}

		if (fs.subchunk1_size == 16) {
			break;
		}
	}

	fread(&fs.audio_format, 2, 1, fp);
	fread(&fs.num_channels, 2, 1, fp);
	fread(&fs.sample_rate, 4, 1, fp);
	fread(&fs.byte_rate, 4, 1, fp);
	fread(&fs.block_align, 2, 1, fp);
	fread(&fs.bits_per_sample, 2, 1, fp);

	if (fs.subchunk1_size > 16) {
		fseek(fp, fs.subchunk1_size - 16, SEEK_CUR);
	}

	return fs;
}

struct data_subchunk read_data_subchunk(FILE *fp) {
	struct data_subchunk ds;

	fread(ds.subchunk2_id, 4, 1, fp);
	fread(&ds.subchunk2_size, 4, 1, fp);

	return ds;
}

void print_wav_header_info(struct wav_header wh) {
		printf("SubChunk ID     : %.4s\n", wh.riff_header.chunk_id);
		printf("SubChunk Size   : %u\n", wh.riff_header.chunk_size);
		printf("Audio Format    : %s\n", wh.riff_header.format);

		printf("SubChunk1 ID    : %.4s\n", wh.fmt_subchunk.subchunk1_id);
		printf("SubChunk1 Size  : %u\n", wh.fmt_subchunk.subchunk1_size);
		printf("Audio Format    : %u (%s)\n", wh.fmt_subchunk.audio_format, (wh.fmt_subchunk.audio_format == 1) ? "PCM": "Non-PCM");
		printf("Channels        : %u\n", wh.fmt_subchunk.num_channels);
		printf("Sample Rate     : %u HZ\n", wh.fmt_subchunk.sample_rate);
		printf("Byte Rate       : %u bytes/sec\n", wh.fmt_subchunk.byte_rate);
		printf("Block Align     : %u bytes\n", wh.fmt_subchunk.block_align);
		printf("Bits Per Sample : %u bits\n", wh.fmt_subchunk.bits_per_sample);

		printf("Subchunk2 ID    : %.4s\n", wh.data_subchunk.subchunk2_id);
		printf("Subchunk2 Size  : %u bytes (Audio Data)\n", wh.data_subchunk.subchunk2_size);

}
