// sherpa-ncnn/csrc/offline-speech-denoiser-gtcrn-model-config.cc
//
// Copyright (c)  2025  Xiaomi Corporation

#include "sherpa-ncnn/csrc/offline-speech-denoiser-gtcrn-model-config.h"

#include <string>

#include "sherpa-ncnn/csrc/file-utils.h"
#include "sherpa-ncnn/csrc/macros.h"

namespace sherpa_ncnn {

void OfflineSpeechDenoiserGtcrnModelConfig::Register(ParseOptions *po) {
  po->Register("speech-denoiser-gtcrn-param", &param,
               "Path to the gtcrn.ncnn.param for speech denoising");

  po->Register("speech-denoiser-gtcrn-bin", &bin,
               "Path to the gtcrn.ncnn.bin for speech denoising");

  po->Register("speech-denoiser-gtcrn-num-threads", &num_threads,
               "Number of threads used to run the gtcrn model");
}

bool OfflineSpeechDenoiserGtcrnModelConfig::Validate() const {
  if (param.empty()) {
    NCNN_LOGE("Please provide --speech-denoiser-gtcrn-param");
    return false;
  }

  if (!FileExists(param)) {
    NCNN_LOGE("gtcrn param file '%s' does not exist", param.c_str());
    return false;
  }

  if (bin.empty()) {
    NCNN_LOGE("Please provide --speech-denoiser-gtcrn-bin");
    return false;
  }

  if (!FileExists(bin)) {
    NCNN_LOGE("gtcrn bin file '%s' does not exist", bin.c_str());
    return false;
  }
  if (num_threads < 1) {
    NCNN_LOGE("Please use a positive number of threads. Given: %d",
              num_threads);
    return false;
  }

  return true;
}

std::string OfflineSpeechDenoiserGtcrnModelConfig::ToString() const {
  std::ostringstream os;

  os << "OfflineSpeechDenoiserGtcrnModelConfig(";
  os << "param=\"" << param << "\", ";
  os << "bin=\"" << bin << "\", ";
  os << "num_threads=" << num_threads << ")";

  return os.str();
}

}  // namespace sherpa_ncnn
