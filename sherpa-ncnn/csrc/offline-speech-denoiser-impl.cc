// sherpa-ncnn/csrc/offline-speech-denoiser-impl.cc
//
// Copyright (c)  2025  Xiaomi Corporation
#include "sherpa-ncnn/csrc/offline-speech-denoiser-impl.h"

#include <memory>

#if __ANDROID_API__ >= 9
#include "android/asset_manager.h"
#include "android/asset_manager_jni.h"
#endif

#if __OHOS__
#include "rawfile/raw_file_manager.h"
#endif

#include "sherpa-ncnn/csrc/macros.h"
#include "sherpa-ncnn/csrc/offline-speech-denoiser-gtcrn-impl.h"

namespace sherpa_ncnn {

std::unique_ptr<OfflineSpeechDenoiserImpl> OfflineSpeechDenoiserImpl::Create(
    const OfflineSpeechDenoiserConfig &config) {
  if (!config.model.gtcrn.param.empty()) {
    return std::make_unique<OfflineSpeechDenoiserGtcrnImpl>(config);
  }
  NCNN_LOGE("Please provide a speech denoising model.");
  return nullptr;
}

template <typename Manager>
std::unique_ptr<OfflineSpeechDenoiserImpl> OfflineSpeechDenoiserImpl::Create(
    Manager *mgr, const OfflineSpeechDenoiserConfig &config) {
  if (!config.model.gtcrn.param.empty()) {
    return std::make_unique<OfflineSpeechDenoiserGtcrnImpl>(mgr, config);
  }
  NCNN_LOGE("Please provide a speech denoising model.");
  return nullptr;
}

#if __ANDROID_API__ >= 9
template std::unique_ptr<OfflineSpeechDenoiserImpl>
OfflineSpeechDenoiserImpl::Create(AAssetManager *mgr,
                                  const OfflineSpeechDenoiserConfig &config);
#endif

#if __OHOS__
template std::unique_ptr<OfflineSpeechDenoiserImpl>
OfflineSpeechDenoiserImpl::Create(NativeResourceManager *mgr,
                                  const OfflineSpeechDenoiserConfig &config);
#endif

}  // namespace sherpa_ncnn
