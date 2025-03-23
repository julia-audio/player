#include "player.h"
#include <stdio.h>
#include <stdlib.h>

void data_callback(ma_device *pDevice, void *pOutput, const void *pInput,
                   ma_uint32 frameCount) {
  ma_decoder *pDecoder = (ma_decoder *)pDevice->pUserData;
  if (pDecoder == NULL) {
    return;
  }

  ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

  float *buffer = (float *)pOutput;
  ma_uint32 channels = pDevice->playback.channels;

  for (ma_uint32 i = 0; i < frameCount * channels; ++i) {
    if (is_mute) {
      buffer[i] *= 0.0;
    } else {
      buffer[i] *= volume;
    }
  }

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

  deviceConfig.playback.format = decoder.outputFormat;
  deviceConfig.playback.channels = decoder.outputChannels;
  deviceConfig.sampleRate = decoder.outputSampleRate;
  deviceConfig.dataCallback = data_callback;
  deviceConfig.pUserData = &decoder;

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

void increase_volume() {
  volume += 0.05f;
  if (volume > 1.0f)
    volume = 1.0f;
}

void decrease_volume() {
  volume -= 0.05f;
  if (volume < 0.0f)
    volume = 0.0f;
}

void mute_toggle() {
  if (is_paused) {
    ma_device_start(&device);
    is_paused = 0;
  } else {
    ma_device_stop(&device);
    is_paused = 1;
  }
}

void seek(int seconds) {
  ma_uint64 current_frame;
  ma_uint64 target_frame;
  ma_uint64 total_frames;
  ma_result result;

  result = ma_decoder_get_cursor_in_pcm_frames(&decoder, &current_frame);
  if (result != MA_SUCCESS)
    return;

  ma_decoder_get_length_in_pcm_frames(&decoder, &total_frames);

  ma_uint64 offset = abs(seconds) * decoder.outputSampleRate;

  if (seconds >= 0) {
    target_frame = current_frame + offset;

    if (target_frame > total_frames)
      target_frame = total_frames;
  } else {
    if (offset > current_frame) {
      target_frame = 0;
    } else {
      target_frame = current_frame - offset;
    }
  }

  ma_decoder_seek_to_pcm_frame(&decoder, target_frame);
}
