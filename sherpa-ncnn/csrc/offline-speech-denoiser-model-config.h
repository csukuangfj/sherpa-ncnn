// sherpa-ncnn/csrc/offline-speech-denoiser-model-config.h
//
// Copyright (c)  2025  Xiaomi Corporation
#ifndef SHERPA_NCNN_CSRC_OFFLINE_SPEECH_DENOISER_MODEL_CONFIG_H_
#define SHERPA_NCNN_CSRC_OFFLINE_SPEECH_DENOISER_MODEL_CONFIG_H_

#include <string>

#include "sherpa-ncnn/csrc/offline-speech-denoiser-gtcrn-model-config.h"
#include "sherpa-ncnn/csrc/parse-options.h"

namespace sherpa_ncnn {

struct OfflineSpeechDenoiserModelConfig {
  OfflineSpeechDenoiserGtcrnModelConfig gtcrn;

  bool debug = false;

  bool use_vulkan_compute = true;

  OfflineSpeechDenoiserModelConfig() = default;

  OfflineSpeechDenoiserModelConfig(OfflineSpeechDenoiserGtcrnModelConfig gtcrn,
                                   int32_t num_threads, bool debug)
      : gtcrn(gtcrn), debug(debug) {}

  void Register(ParseOptions *po);
  bool Validate() const;

  std::string ToString() const;
};

}  // namespace sherpa_ncnn

#endif  // SHERPA_NCNN_CSRC_OFFLINE_SPEECH_DENOISER_MODEL_CONFIG_H_
