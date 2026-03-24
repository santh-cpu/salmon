#include "audio_capture.h"
#include <stdexcept>

constexpr int SAMPLE_RATE = 44100;
constexpr int HOP_SIZE = 1024;

static int paCallback(const void *in, void *, unsigned long frames,
                      const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags,
                      void *ud_ptr) {
  auto *src = static_cast<const float *>(in);
  auto *ud = static_cast<PaUD *>(ud_ptr);
  if (!src)
    return paContinue;

  if (ud->channels == 1) {
    ud->ring->push(src, frames);
  } else {
    static float mono[8192];
    unsigned long n = std::min<unsigned long>(frames, 8192);
    for (unsigned long i = 0; i < n; ++i)
      mono[i] = src[i * ud->channels];
    ud->ring->push(mono, n);
  }
  return paContinue;
}

AudioCapture::AudioCapture() : ud_{&ring_} {
  Pa_Initialize();
  PaStreamParameters p{};
  p.device = Pa_GetDefaultInputDevice();
  if (p.device == paNoDevice)
    throw std::runtime_error("No audio input device found.");
  p.channelCount = 1;
  p.sampleFormat = paFloat32;
  p.suggestedLatency = Pa_GetDeviceInfo(p.device)->defaultLowInputLatency;

  PaError e = Pa_OpenStream(&stream_, &p, nullptr, SAMPLE_RATE, HOP_SIZE,
                            paClipOff, paCallback, &ud_);
  if (e == paInvalidChannelCount) {
    p.channelCount = 2;
    ud_.channels = 2;
    e = Pa_OpenStream(&stream_, &p, nullptr, SAMPLE_RATE, HOP_SIZE, paClipOff,
                      paCallback, &ud_);
  }
  if (e != paNoError)
    throw std::runtime_error("PortAudio open stream failed.");
  Pa_StartStream(stream_);
}

AudioCapture::~AudioCapture() {
  if (stream_) {
    Pa_StopStream(stream_);
    Pa_CloseStream(stream_);
  }
  Pa_Terminate();
}

bool AudioCapture::read(std::vector<double> &out) {
  if (ring_.available() < (size_t)HOP_SIZE)
    return false;
  static std::vector<float> tmp(HOP_SIZE);
  ring_.pop(tmp.data(), HOP_SIZE);
  out.resize(HOP_SIZE);
  for (int i = 0; i < HOP_SIZE; ++i)
    out[i] = (double)tmp[i];
  return true;
}
