#include <stdint.h>
#include <stdio.h>

#ifndef WAV_READER_H
#define WAV_READER_H

struct riff_header {
  char chunk_id[4];
  int chunk_size;
  char format[4];
};

struct fmt_subchunk {
  char subchunk1_id[4];
  int subchunk1_size;
  short audio_format;
  short num_channels;
  int sample_rate;
  int byte_rate;
  short block_align;
  short bits_per_sample;
};

struct data_subchunk {
  char subchunk2_id[4];
  int subchunk2_size;
};

struct wav_header {
  struct riff_header riff_header;
  struct fmt_subchunk fmt_subchunk;
  struct data_subchunk data_subchunk;
};

int check_file_format(FILE *fp);
struct riff_header read_riff_header(FILE *fp);
struct fmt_subchunk read_fmt_subchunk(FILE *fp);
struct data_subchunk read_data_subchunk(FILE *fp);
void print_wav_header_info(struct wav_header wh);

#endif
