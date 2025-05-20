// sherpa-ncnn/csrc/offline-speech-denoiser-gtcrn-model.h
//
// Copyright (c)  2025  Xiaomi Corporation
#ifndef SHERPA_NCNN_CSRC_OFFLINE_SPEECH_DENOISER_GTCRN_MODEL_H_
#define SHERPA_NCNN_CSRC_OFFLINE_SPEECH_DENOISER_GTCRN_MODEL_H_
#include <memory>
#include <utility>
#include <vector>

#include "net.h"  // NOLINT
#include "sherpa-ncnn/csrc/offline-speech-denoiser-gtcrn-model-meta-data.h"
#include "sherpa-ncnn/csrc/offline-speech-denoiser-model-config.h"
#include "sherpa-ncnn/csrc/offline-speech-denoiser.h"

namespace sherpa_ncnn {

class OfflineSpeechDenoiserGtcrnModel {
 public:
  ~OfflineSpeechDenoiserGtcrnModel();
  explicit OfflineSpeechDenoiserGtcrnModel(
      const OfflineSpeechDenoiserModelConfig &config);

  template <typename Manager>
  OfflineSpeechDenoiserGtcrnModel(
      Manager *mgr, const OfflineSpeechDenoiserModelConfig &config);

  using States = std::vector<ncnn::Mat>;

  States GetInitStates() const;

  std::pair<ncnn::Mat, States> Run(const ncnn::Mat &x,
                                   const States &states) const;

  const OfflineSpeechDenoiserGtcrnModelMetaData &GetMetaData() const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace sherpa_ncnn

#endif  // SHERPA_ONNX_CSRC_OFFLINE_SPEECH_DENOISER_GTCRN_MODEL_H_
