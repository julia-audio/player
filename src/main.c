#define _POSIX_C_SOURCE 200809L

#include "wav_parser.h"
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <string.h>

void print_usage() {
  printf("\033[1mUsage: player\033[0m \033[32m[OPTIONS]\033[0m "
         "\033[33m<file>\033[0m\n");
  printf("\033[32mOPTIONS:\033[0m\n");
  printf("\033[32m  -h, --help\033[0m\033[1m: show this help message\033[0m\n");
}

struct wav_header decode(FILE *fp) {
  rewind(fp);

  struct wav_header wh;
  wh.riff_header = read_riff_header(fp);
  wh.fmt_subchunk = read_fmt_subchunk(fp);
  wh.data_subchunk = read_data_subchunk(fp);

  print_wav_header_info(wh);

  return wh;
}

int16_t float_to_pcm16(float sample) {
  if (sample > 1.0f)
    sample = 1.0f;
  if (sample < -1.0f)
    sample = -1.0f;
  return (int16_t)(sample * 32767.0f);
}

void play(FILE *fp) {
  struct wav_header wh = decode(fp);

  snd_pcm_t *pcm_handle;
  snd_pcm_hw_params_t *params;
  unsigned int sample_rate = wh.fmt_subchunk.sample_rate;
  int channels = wh.fmt_subchunk.num_channels;
  int frames = 1024;

  if (snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
    perror("Cannot open ALSA device!");
    fclose(fp);
    return;
  }

  snd_pcm_hw_params_malloc(&params);
  snd_pcm_hw_params_any(pcm_handle, params);
  snd_pcm_hw_params_set_access(pcm_handle, params,
                               SND_PCM_ACCESS_RW_INTERLEAVED);
  snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
  snd_pcm_hw_params_set_channels(pcm_handle, params, channels);
  snd_pcm_hw_params_set_rate_near(pcm_handle, params, &sample_rate, 0);
  snd_pcm_hw_params(pcm_handle, params);

  int16_t *pcm_buffer = malloc(frames * channels * sizeof(int16_t));

  if (wh.fmt_subchunk.audio_format != 1) {
    float *float_buffer = malloc(frames * channels * sizeof(float));

    while (fread(float_buffer, sizeof(float), frames * channels, fp) > 0) {
      for (int i = 0; i < frames * channels; i++) {
        pcm_buffer[i] = float_to_pcm16(float_buffer[i]);
      }

      if (snd_pcm_writei(pcm_handle, pcm_buffer, frames) == -EPIPE) {
        snd_pcm_prepare(pcm_handle);
      }
    }

    free(float_buffer);
  } else {
    while (fread(pcm_buffer, sizeof(int16_t), frames * channels, fp) > 0) {
      if (snd_pcm_writei(pcm_handle, pcm_buffer, frames) == -EPIPE) {
        snd_pcm_prepare(pcm_handle);
      }
    }
  }

  free(pcm_buffer);
  snd_pcm_hw_params_free(params);
  snd_pcm_drain(pcm_handle);
  snd_pcm_close(pcm_handle);
}

int main(int argc, char *argv[]) {
  char *filename = NULL;

  if (argc < 2) {
    print_usage();
    return 0;
  }

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      print_usage();
      return 0;
    } else {
      filename = argv[i];
    }
  }

  FILE *file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "cannot open file: %s", filename);
    return 1;
  }

  play(file);

  fclose(file);
  return 0;
}
