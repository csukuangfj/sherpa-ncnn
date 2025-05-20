// sherpa-ncnn/csrc/offline-speech-denoiser-model-config.cc
//
// Copyright (c)  2025  Xiaomi Corporation

#include "sherpa-ncnn/csrc/offline-speech-denoiser-model-config.h"

#include <string>

namespace sherpa_ncnn {

void OfflineSpeechDenoiserModelConfig::Register(ParseOptions *po) {
  gtcrn.Register(po);

  po->Register("debug", &debug,
               "true to print model information while loading it.");
}

bool OfflineSpeechDenoiserModelConfig::Validate() const {
  return gtcrn.Validate();
}

std::string OfflineSpeechDenoiserModelConfig::ToString() const {
  std::ostringstream os;

  os << "OfflineSpeechDenoiserModelConfig(";
  os << "gtcrn=" << gtcrn.ToString() << ", ";
  os << "debug=" << (debug ? "True" : "False") << ")";

  return os.str();
}

}  // namespace sherpa_ncnn
