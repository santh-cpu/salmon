#pragma once
#include "hann_window.h"
#include "kalman_smoother.h"
#include <fftw3.h>
#include <vector>

struct YINResult {
  double freq, confidence, db;
  bool valid;
};

struct HPSResult {
  double freq, db;
  bool valid;
};

struct CrossResult {
  double freq, confidence, db;
  bool valid, crossValidated;
};

class YINDetector {
  int N_, maxLag_, minLag_;
  std::vector<double> diff_, cmnd_;
  KalmanSmoother kalman_;

public:
  explicit YINDetector(int N);
  YINResult detect(const std::vector<double> &frame);
  void reset();
};

class HPSDetector {
  int N_;
  fftw_plan plan_;
  double *in_;
  fftw_complex *out_;
  HannWindow win_;
  KalmanSmoother kalman_;

public:
  explicit HPSDetector(int N);
  ~HPSDetector();
  HPSResult detect(const std::vector<double> &frame);
  void reset();
};

class CrossValidator {
  YINDetector yin_;
  HPSDetector hps_;

public:
  explicit CrossValidator(int N);
  CrossResult detect(const std::vector<double> &frame);
  void reset();
};
