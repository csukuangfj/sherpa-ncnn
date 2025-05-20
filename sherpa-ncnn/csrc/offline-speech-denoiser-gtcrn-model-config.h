// sherpa-ncnn/csrc/offline-speech-denoiser-gtcrn-model-config.h
//
// Copyright (c)  2025  Xiaomi Corporation
#ifndef SHERPA_NCNN_CSRC_OFFLINE_SPEECH_DENOISER_GTCRN_MODEL_CONFIG_H_
#define SHERPA_NCNN_CSRC_OFFLINE_SPEECH_DENOISER_GTCRN_MODEL_CONFIG_H_

#include <string>

#include "sherpa-ncnn/csrc/parse-options.h"

namespace sherpa_ncnn {

struct OfflineSpeechDenoiserGtcrnModelConfig {
  std::string param;  // path to gtcrn.ncnn.param
  std::string bin;    // path to gtcrn.ncnn.bin
  int32_t num_threads = 1;
  OfflineSpeechDenoiserGtcrnModelConfig() = default;

  OfflineSpeechDenoiserGtcrnModelConfig(const std::string &param,
                                        const std::string &bin,
                                        int32_t num_threads)
      : param(param), bin(bin), num_threads(num_threads) {}

  void Register(ParseOptions *po);
  bool Validate() const;

  std::string ToString() const;
};

}  // namespace sherpa_ncnn

#endif  // SHERPA_NCNN_CSRC_OFFLINE_SPEECH_DENOISER_GTCRN_MODEL_CONFIG_H_
