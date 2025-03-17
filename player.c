#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char chunk_id[4];
  uint32_t chunk_size;
  char format[4];
} RiffHeader;

typedef struct {
  char subchunk_id[4];
  uint32_t subchunk_size;
} ChunkHeader;

typedef struct {
  char subchunk1_id[4];
  uint32_t subchunk1_size;
  uint16_t audio_format;
  uint16_t num_channels;
  uint32_t sample_rate;
  uint32_t byte_rate;
  uint16_t block_align;
  uint16_t bits_per_sample;
} WavFmt;

typedef struct {
  char subchunk2_id[4];
  uint32_t subchunk2_size;
  uint8_t *data;
} WavData;

typedef struct {
  RiffHeader riff;
  WavFmt fmt;
  WavData data;
} WavFile;

WavFile *wav_open(const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    perror("failed open file");
    return NULL;
  }

  WavFile *wav = (WavFile *)malloc(sizeof(WavFile));
  if (!wav) {
    fprintf(stderr, "Memory allocation failed!\n");
    fclose(file);
    return NULL;
  }

  // RIFF Header
  fread(&wav->riff, sizeof(RiffHeader), 1, file);
  if (strncmp(wav->riff.chunk_id, "RIFF", 4) ||
      strncmp(wav->riff.format, "WAVE", 4)) {
    fprintf(stderr, "Not a WAV file!\n");
    fclose(file);
    free(wav);
    return NULL;
  }

  ChunkHeader chunk;

  // FMT
  while (fread(&chunk, sizeof(ChunkHeader), 1, file)) {
    if (strncmp(chunk.subchunk_id, "fmt ", 4) == 0) {
      memcpy(wav->fmt.subchunk1_id, chunk.subchunk_id, 4);
      wav->fmt.subchunk1_size = chunk.subchunk_size;

      fread(&wav->fmt.audio_format, chunk.subchunk_size, 1, file);
      break;
    } else {
      fseek(file, chunk.subchunk_size, SEEK_CUR);
    }
  }

  // DATA
  while (fread(&chunk, sizeof(ChunkHeader), 1, file)) {
    if (strncmp(chunk.subchunk_id, "data", 4) == 0) {
      memcpy(wav->data.subchunk2_id, chunk.subchunk_id, 4);
      wav->data.subchunk2_size = chunk.subchunk_size;

      wav->data.data = (uint8_t *)malloc(chunk.subchunk_size);

      if (!wav->data.data) {
        fprintf(stderr, "Failed to allocate memory for audio data\n");
        fclose(file);
        free(wav);
        return NULL;
      }

      fread(wav->data.data, chunk.subchunk_size, 1, file);
      break;
    } else {
      fseek(file, chunk.subchunk_size, SEEK_CUR);
    }
  }

  return wav;
}

void wav_print_info(WavFile *wav) {
  printf("===== WAV FILE INFO =====\n");
  printf("Chunk ID      : %.4s\n", wav->riff.chunk_id);
  printf("Chunk Size    : %u bytes\n", wav->riff.chunk_size);
  printf("Format        : %.4s\n", wav->riff.format);

  printf("\n=== Format Chunk ===\n");
  printf("Subchunk1 ID  : %.4s\n", wav->fmt.subchunk1_id);
  printf("Subchunk1 Size: %u\n", wav->fmt.subchunk1_size);
  printf("Audio Format  : %u (%s)\n", wav->fmt.audio_format,
         (wav->fmt.audio_format == 1) ? "PCM" : "Non-PCM");
  printf("Channels      : %u\n", wav->fmt.num_channels);
  printf("Sample Rate   : %u Hz\n", wav->fmt.sample_rate);
  printf("Byte Rate     : %u bytes/sec\n", wav->fmt.byte_rate);
  printf("Block Align   : %u bytes\n", wav->fmt.block_align);
  printf("Bits Per Sample: %u bits\n", wav->fmt.bits_per_sample);

  printf("\n=== Data Chunk ===\n");
  printf("Subchunk2 ID  : %.4s\n", wav->data.subchunk2_id);
  printf("Subchunk2 Size: %u bytes (Audio Data)\n", wav->data.subchunk2_size);
}

void wav_close(WavFile *wav) {
  if (wav) {
    free(wav->data.data);
    free(wav);
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <.wav file>\n", argv[0]);
    return 1;
  }

  WavFile *wav = wav_open(argv[1]);

  if (!wav) {
    fprintf(stderr, "WAV file load failed\n");
    return 1;
  }

  wav_print_info(wav);

  wav_close(wav);

  return 0;
}
