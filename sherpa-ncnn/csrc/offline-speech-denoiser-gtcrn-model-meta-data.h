// sherpa-ncnn/csrc/offline-speech-denoiser-gtcrn-model-meta-data.h
//
// Copyright (c)  2025  Xiaomi Corporation

#ifndef SHERPA_NCNN_CSRC_OFFLINE_SPEECH_DENOISER_GTCRN_MODEL_META_DATA_H_
#define SHERPA_NCNN_CSRC_OFFLINE_SPEECH_DENOISER_GTCRN_MODEL_META_DATA_H_

#include <cstdint>
#include <string>
#include <vector>

namespace sherpa_ncnn {

// please refer to
// https://github.com/wxqwinner/gtcrn-ncnn/blob/main/gtcrn.py
struct OfflineSpeechDenoiserGtcrnModelMetaData {
  int32_t sample_rate = 16000;
  int32_t version = 1;
  int32_t n_fft = 512;
  int32_t hop_length = 256;
  int32_t window_length = 512;
  std::string window_type = "hann_sqrt";

  // std::vector<int32_t> conv_cache_shape = {2, 1, 16, 16, 33};
  // std::vector<int32_t> tra_cache_shape = {2, 3, 1, 1, 16};
  // std::vector<int32_t> inter_cache_shape = {2, 1, 33, 16};

  std::vector<int32_t> conv_cache_shape = {16, 32, 33};
  std::vector<int32_t> tra_cache_shape = {6, 16};
  std::vector<int32_t> inter_cache_shape = {1, 33, 32};
};

}  // namespace sherpa_ncnn

#endif  // SHERPA_NCNN_CSRC_OFFLINE_SPEECH_DENOISER_GTCRN_MODEL_META_DATA_H_
