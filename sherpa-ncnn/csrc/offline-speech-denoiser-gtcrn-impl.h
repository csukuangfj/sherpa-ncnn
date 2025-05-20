// sherpa-ncnn/csrc/offline-speech-denoiser-gtcrn-impl.h
//
// Copyright (c)  2025  Xiaomi Corporation

#ifndef SHERPA_NCNN_CSRC_OFFLINE_SPEECH_DENOISER_GTCRN_IMPL_H_
#define SHERPA_NCNN_CSRC_OFFLINE_SPEECH_DENOISER_GTCRN_IMPL_H_

#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>
#include <vector>

#include "kaldi-native-fbank/csrc/feature-window.h"
#include "kaldi-native-fbank/csrc/istft.h"
#include "kaldi-native-fbank/csrc/stft.h"
#include "sherpa-ncnn/csrc/offline-speech-denoiser-gtcrn-model.h"
#include "sherpa-ncnn/csrc/offline-speech-denoiser-impl.h"
#include "sherpa-ncnn/csrc/offline-speech-denoiser.h"
#include "sherpa-ncnn/csrc/resample.h"

namespace sherpa_ncnn {

float MySum(ncnn::Mat m) {
  const float *p = m;
  int32_t n = m.w * m.h * m.c;
  float sum = 0;
  for (int32_t i = 0; i < n; i++) {
    sum += p[i];
  }
  return sum;
}

class OfflineSpeechDenoiserGtcrnImpl : public OfflineSpeechDenoiserImpl {
 public:
  explicit OfflineSpeechDenoiserGtcrnImpl(
      const OfflineSpeechDenoiserConfig &config)
      : model_(config.model) {}

  template <typename Manager>
  OfflineSpeechDenoiserGtcrnImpl(Manager *mgr,
                                 const OfflineSpeechDenoiserConfig &config)
      : model_(mgr, config.model) {}

  DenoisedAudio Run(const float *samples, int32_t n,
                    int32_t sample_rate) const override {
    const auto &meta = model_.GetMetaData();

    std::vector<float> tmp;
    auto p = samples;

    if (sample_rate != meta.sample_rate) {
      NCNN_LOGE(
          "Creating a resampler:\n"
          "   in_sample_rate: %d\n"
          "   output_sample_rate: %d\n",
          sample_rate, meta.sample_rate);

      float min_freq = std::min<int32_t>(sample_rate, meta.sample_rate);
      float lowpass_cutoff = 0.99 * 0.5 * min_freq;

      int32_t lowpass_filter_width = 6;
      auto resampler = std::make_unique<LinearResample>(
          sample_rate, meta.sample_rate, lowpass_cutoff, lowpass_filter_width);
      resampler->Resample(samples, n, true, &tmp);
      p = tmp.data();
      n = tmp.size();
    }

    knf::StftConfig stft_config;
    stft_config.n_fft = meta.n_fft;
    stft_config.hop_length = meta.hop_length;
    stft_config.win_length = meta.window_length;
    stft_config.window_type = meta.window_type;
    if (stft_config.window_type == "hann_sqrt") {
      auto window = knf::GetWindow("hann", stft_config.win_length);
      for (auto &w : window) {
        w = std::sqrt(w);
      }
      stft_config.window = std::move(window);
    }

    knf::Stft stft(stft_config);
    knf::StftResult stft_result = stft.Compute(p, n);
    NCNN_LOGE("stft result.real size: %d", (int)stft_result.real.size());
    NCNN_LOGE("stft result.imag size: %d", (int)stft_result.imag.size());

    FILE *fp = fopen("./x.bin", "rb");
    fseek(fp, 0, SEEK_END);
    int32_t nn = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    printf("n: %d\n", nn);

    int32_t num_frames = nn / 4 / 514;
    printf("num_frames: %d\n", num_frames);

    stft_result.real.resize(num_frames * 257);
    stft_result.imag.resize(num_frames * 257);

    std::vector<float> buf(nn / sizeof(float));
    fread(buf.data(), sizeof(float), nn, fp);
    fclose(fp);

    const float *p_buf = buf.data();
    for (int32_t i = 0; i < num_frames; ++i) {
      float *p_real = stft_result.real.data() + i * 257;
      float *p_imag = stft_result.imag.data() + i * 257;
      std::copy(p_buf, p_buf + 257, p_real);
      p_buf += 257;
      std::copy(p_buf, p_buf + 257, p_imag);
    }

    auto states = model_.GetInitStates();
    OfflineSpeechDenoiserGtcrnModel::States next_states;

    knf::StftResult enhanced_stft_result;
    enhanced_stft_result.num_frames = stft_result.num_frames;
    for (int32_t i = 0; i < stft_result.num_frames; ++i) {
      auto p = Process(stft_result, i, states, &next_states);
      states = next_states;

      enhanced_stft_result.real.insert(enhanced_stft_result.real.end(),
                                       p.first.begin(), p.first.end());
      enhanced_stft_result.imag.insert(enhanced_stft_result.imag.end(),
                                       p.second.begin(), p.second.end());
      // exit(0);
    }

    knf::IStft istft(stft_config);

    DenoisedAudio denoised_audio;
    denoised_audio.sample_rate = meta.sample_rate;
    denoised_audio.samples = istft.Compute(enhanced_stft_result);
    return denoised_audio;
  }

  int32_t GetSampleRate() const override {
    return model_.GetMetaData().sample_rate;
  }

 private:
  std::pair<std::vector<float>, std::vector<float>> Process(
      const knf::StftResult &stft_result, int32_t frame_index,
      const OfflineSpeechDenoiserGtcrnModel::States &states,
      OfflineSpeechDenoiserGtcrnModel::States *next_states) const {
    const auto &meta = model_.GetMetaData();
    int32_t n_fft = meta.n_fft;

    ncnn::Mat x(2, 1, n_fft / 2 + 1);
    float *px = x;

    const float *p_real =
        stft_result.real.data() + frame_index * (n_fft / 2 + 1);
    const float *p_imag =
        stft_result.imag.data() + frame_index * (n_fft / 2 + 1);

    int32_t k = 0;
    for (int32_t i = 0; i < n_fft / 2 + 1; ++i) {
      x.row(i)[0] = p_real[i];
      x.row(i)[1] = p_imag[i];
      // px[2 * i] = p_real[i];
      // px[2 * i + 1] = p_imag[i];
      k += 2;
    }
    NCNN_LOGE("k: %d, %d, %d\n", k, x.w * x.h * x.c, x.dims);

    ncnn::Mat output;
    std::tie(output, *next_states) = model_.Run(x, states);

    std::vector<float> real(n_fft / 2 + 1);
    std::vector<float> imag(n_fft / 2 + 1);
    const float *p0 = output;
    const float *p1 = output.row(1);

    std::copy(p0, p1, real.begin());
    std::copy(p1, p1 + real.size(), imag.begin());

    return {std::move(real), std::move(imag)};
  }

 private:
  OfflineSpeechDenoiserGtcrnModel model_;
};

}  // namespace sherpa_ncnn

#endif  // SHERPA_NCNN_CSRC_OFFLINE_SPEECH_DENOISER_GTCRN_IMPL_H_
