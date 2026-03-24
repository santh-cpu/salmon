#pragma once
#include "ring_buffer.h"
#include <portaudio.h>
#include <vector>

using AudioRing = RingBuffer<float, 131072>;

struct PaUD {
  AudioRing *ring;
  int channels = 1;
};

class AudioCapture {
  PaStream *stream_ = nullptr;
  PaUD ud_;
  AudioRing ring_;

public:
  AudioCapture();
  ~AudioCapture();
  bool read(std::vector<double> &out);
};
