// sherpa-ncnn/csrc/offline-speech-denoiser-gtcrn-model.cc
//
// Copyright (c)  2025  Xiaomi Corporation

#include "sherpa-ncnn/csrc/offline-speech-denoiser-gtcrn-model.h"

#include <memory>
#include <regex>  // NOLINT
#include <string>
#include <utility>
#include <vector>

#include "sherpa-ncnn/csrc/model.h"

namespace sherpa_ncnn {

float MySum(ncnn::Mat m);

class OfflineSpeechDenoiserGtcrnModel::Impl {
 public:
  explicit Impl(const OfflineSpeechDenoiserModelConfig &config)
      : config_(config) {
    Init();

    Model::InitNet(model_, config_.gtcrn.param, config_.gtcrn.bin);

    InitInputOutputIndexes();
  }

  template <typename Manager>
  Impl(Manager *mgr, const OfflineSpeechDenoiserModelConfig &config)
      : config_(config) {
    Init();

    Model::InitNet(mgr, model_, config_.gtcrn.param, config_.gtcrn.bin);

    InitInputOutputIndexes();
  }

  const OfflineSpeechDenoiserGtcrnModelMetaData &GetMetaData() const {
    return meta_;
  }

  States GetInitStates() const {
    ncnn::Mat conv_cache(meta_.conv_cache_shape[2], meta_.conv_cache_shape[1],
                         meta_.conv_cache_shape[0]);

    ncnn::Mat tra_cache(meta_.tra_cache_shape[1], meta_.tra_cache_shape[0]);

    ncnn::Mat inter_cache(meta_.inter_cache_shape[2],
                          meta_.inter_cache_shape[1],
                          meta_.inter_cache_shape[0]);

    conv_cache.fill(0);
    tra_cache.fill(0);
    inter_cache.fill(0);

    States ans(3);
    ans[0] = conv_cache;
    ans[1] = tra_cache;
    ans[2] = inter_cache;

    return ans;
  }

  std::pair<ncnn::Mat, States> Run(const ncnn::Mat &x,
                                   const States &states) const {
    ncnn::Extractor ex = model_.create_extractor();

    // Note: We ignore error check there
    // ex.input(input_indexes_[0], x);
    // for (int32_t i = 1; i != input_indexes_.size(); ++i) {
    //   ex.input(input_indexes_[i], states[i - 1]);
    // }
    ex.input("in0", x);
    ex.input("in1", states[0]);
    ex.input("in2", states[1]);
    ex.input("in3", states[2]);

    NCNN_LOGE("in: w %d, h %d, c %d", x.w, x.h, x.c);

    NCNN_LOGE("in1: w %d, h %d, c %d", states[0].w, states[0].h, states[0].c);
    NCNN_LOGE("in2: w %d, h %d, c %d", states[1].w, states[1].h, states[1].c);
    NCNN_LOGE("in3: w %d, h %d, c %d", states[2].w, states[2].h, states[2].c);
    NCNN_LOGE("x sum: %.6f, in1 sum: %.6f, in2 sum: %.6f in3 sum: %.6f",
              MySum(x), MySum(states[0]), MySum(states[1]), MySum(states[2]));

    ncnn::Mat out;
    // ex.extract(output_indexes_[0], out);
    ex.extract("out0", out);

    States next_states(3);
    ex.extract("out1", next_states[0]);
    ex.extract("out2", next_states[1]);
    ex.extract("out3", next_states[2]);
    // for (int32_t i = 1; i != output_indexes_.size(); ++i) {
    //   ex.extract(output_indexes_[i], next_states[i - 1]);
    // }

    NCNN_LOGE("out: w %d, h %d, c %d", out.w, out.h, out.c);
    NCNN_LOGE("out1: w %d, h %d, c %d", next_states[0].w, next_states[0].h,
              next_states[0].c);
    NCNN_LOGE("out2: w %d, h %d, c %d", next_states[1].w, next_states[1].h,
              next_states[1].c);
    NCNN_LOGE("out3: w %d, h %d, c %d", next_states[2].w, next_states[2].h,
              next_states[2].c);

    NCNN_LOGE("out sum: %.3f, out1 sum: %.3f, out2 sum: %.3f out3 sum: %.3f",
              MySum(out), MySum(next_states[0]), MySum(next_states[1]),
              MySum(next_states[2]));

    return {out, next_states};
  }

 private:
  void Init() {
    model_.opt.num_threads = config_.gtcrn.num_threads;

    bool has_gpu = false;
#if NCNN_VULKAN
    has_gpu = ncnn::get_gpu_count() > 0;
#endif

    if (has_gpu && config_.use_vulkan_compute) {
      model_.opt.use_vulkan_compute = true;
      NCNN_LOGE("Use GPU");
    } else {
      // NCNN_LOGE("Don't Use GPU. has_gpu: %d, config_.use_vulkan_compute: %d",
      //           static_cast<int32_t>(has_gpu),
      //           static_cast<int32_t>(config_.use_vulkan_compute));
    }
  }

  void InitInputOutputIndexes() {
    // input indexes map
    // [0] -> in0, samples
    // [1] -> in1, conv_cache
    // [2] -> in2, tra_cache
    // [3] -> in3, inter_cache
    // ... ...
    input_indexes_.resize(4);

    // output indexes map
    // [0] -> out0, model_out
    //
    // [1] -> out1, conv_cache
    // [2] -> out2, tra_cache
    // [3] -> out3, inter_cache

    output_indexes_.resize(4);
    const auto &blobs = model_.blobs();

    std::regex in_regex("in(\\d+)");
    std::regex out_regex("out(\\d+)");

    std::smatch match;
    for (int32_t i = 0; i != blobs.size(); ++i) {
      const auto &b = blobs[i];
      if (std::regex_match(b.name, match, in_regex)) {
        auto index = std::atoi(match[1].str().c_str());
        input_indexes_[index] = i;
      } else if (std::regex_match(b.name, match, out_regex)) {
        auto index = std::atoi(match[1].str().c_str());
        output_indexes_[index] = i;
      }
    }
  }

 private:
  OfflineSpeechDenoiserModelConfig config_;
  OfflineSpeechDenoiserGtcrnModelMetaData meta_;

  ncnn::Net model_;

  std::vector<int32_t> input_indexes_;
  std::vector<int32_t> output_indexes_;
};

OfflineSpeechDenoiserGtcrnModel::~OfflineSpeechDenoiserGtcrnModel() = default;

OfflineSpeechDenoiserGtcrnModel::OfflineSpeechDenoiserGtcrnModel(
    const OfflineSpeechDenoiserModelConfig &config)
    : impl_(std::make_unique<Impl>(config)) {}

template <typename Manager>
OfflineSpeechDenoiserGtcrnModel::OfflineSpeechDenoiserGtcrnModel(
    Manager *mgr, const OfflineSpeechDenoiserModelConfig &config)
    : impl_(std::make_unique<Impl>(mgr, config)) {}

OfflineSpeechDenoiserGtcrnModel::States
OfflineSpeechDenoiserGtcrnModel::GetInitStates() const {
  return impl_->GetInitStates();
}

std::pair<ncnn::Mat, OfflineSpeechDenoiserGtcrnModel::States>
OfflineSpeechDenoiserGtcrnModel::Run(const ncnn::Mat &x,
                                     const States &states) const {
  return impl_->Run(x, states);
}

const OfflineSpeechDenoiserGtcrnModelMetaData &
OfflineSpeechDenoiserGtcrnModel::GetMetaData() const {
  return impl_->GetMetaData();
}

#if __ANDROID_API__ >= 9
template OfflineSpeechDenoiserGtcrnModel::OfflineSpeechDenoiserGtcrnModel(
    AAssetManager *mgr, const OfflineSpeechDenoiserModelConfig &config);
#endif

#if __OHOS__
template OfflineSpeechDenoiserGtcrnModel::OfflineSpeechDenoiserGtcrnModel(
    NativeResourceManager *mgr, const OfflineSpeechDenoiserModelConfig &config);
#endif

}  // namespace sherpa_ncnn
