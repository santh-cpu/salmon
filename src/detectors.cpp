#include "detectors.h"
#include "music.h"
#include <algorithm>
#include <cmath>

constexpr double NOISE_FLOOR_DB = -50.0;
constexpr double YIN_THRESHOLD = 0.12;
constexpr int HPS_HARMONICS = 3;
constexpr double CROSSVAL_CENTS = 25.0;
constexpr double MIN_FREQ = 30.0;
constexpr double MAX_FREQ = 1500.0;
constexpr int SAMPLE_RATE = 44100;
constexpr int FFT_SIZE = 8192;

YINDetector::YINDetector(int N)
    : N_(N), maxLag_((int)(SAMPLE_RATE / MIN_FREQ) + 1),
      minLag_((int)(SAMPLE_RATE / MAX_FREQ)), diff_(maxLag_, 0.0),
      cmnd_(maxLag_, 0.0) {}

YINResult YINDetector::detect(const std::vector<double> &frame) {
  double rms = 0.0;
  for (double s : frame)
    rms += s * s;
  rms = std::sqrt(rms / frame.size());
  double db = (rms > 1e-10) ? 20.0 * std::log10(rms) : -120.0;
  if (db < NOISE_FLOOR_DB)
    return {0.0, 1.0, db, false};

  std::fill(diff_.begin(), diff_.end(), 0.0);
  int limit = (int)frame.size() - maxLag_;
  if (limit <= 0)
    return {0.0, 1.0, db, false};

  for (int tau = 1; tau < maxLag_; ++tau) {
    for (int i = 0; i < limit; ++i) {
      double delta = frame[i] - frame[i + tau];
      diff_[tau] += delta * delta;
    }
  }

  cmnd_[0] = 1.0;
  double running = 0.0;
  for (int tau = 1; tau < maxLag_; ++tau) {
    running += diff_[tau];
    cmnd_[tau] = (running > 0.0) ? diff_[tau] * tau / running : 1.0;
  }

  int tau = -1;
  for (int t = minLag_; t < maxLag_; ++t) {
    if (cmnd_[t] < YIN_THRESHOLD) {
      while (t + 1 < maxLag_ && cmnd_[t + 1] < cmnd_[t])
        ++t;
      tau = t;
      break;
    }
  }
  if (tau < 0)
    return {0.0, 1.0, db, false};

  double refined = tau;
  if (tau > 1 && tau < maxLag_ - 1) {
    double a = cmnd_[tau - 1], b = cmnd_[tau], c = cmnd_[tau + 1];
    double denom = a - 2.0 * b + c;
    if (std::abs(denom) > 1e-12)
      refined += std::clamp(0.5 * (a - c) / denom, -1.0, 1.0);
  }

  double freq = (double)SAMPLE_RATE / refined;
  if (freq < MIN_FREQ || freq > MAX_FREQ)
    return {0.0, 1.0, db, false};

  return {kalman_.update(freq), cmnd_[tau], db, true};
}

void YINDetector::reset() { kalman_.reset(); }

HPSDetector::HPSDetector(int N) : N_(N), win_(N) {
  in_ = fftw_alloc_real(N);
  out_ = fftw_alloc_complex(N / 2 + 1);
  std::fill(in_, in_ + N, 0.0);
  plan_ = fftw_plan_dft_r2c_1d(N, in_, out_, FFTW_ESTIMATE);
}

HPSDetector::~HPSDetector() {
  fftw_destroy_plan(plan_);
  fftw_free(in_);
  fftw_free(out_);
}

HPSResult HPSDetector::detect(const std::vector<double> &frame) {
  int n = std::min((int)frame.size(), N_);
  std::copy(frame.begin(), frame.begin() + n, in_);
  std::fill(in_ + n, in_ + N_, 0.0);

  double rms = 0.0;
  for (int i = 0; i < N_; ++i)
    rms += in_[i] * in_[i];
  rms = std::sqrt(rms / N_);
  double db = (rms > 1e-10) ? 20.0 * std::log10(rms) : -120.0;
  if (db < NOISE_FLOOR_DB)
    return {0.0, db, false};

  win_.apply(in_, N_);
  fftw_execute(plan_);

  int bins = N_ / 2 + 1;
  std::vector<double> mag(bins);
  for (int k = 0; k < bins; ++k) {
    double re = out_[k][0], im = out_[k][1];
    mag[k] = std::sqrt(re * re + im * im);
  }

  int lo = std::max(2, (int)(MIN_FREQ * N_ / SAMPLE_RATE));
  int hi = std::min(bins - 1, (int)(MAX_FREQ * N_ / SAMPLE_RATE));

  std::vector<double> hps(bins, 0.0);
  for (int k = lo; k <= hi; ++k)
    hps[k] = mag[k];

  for (int h = 2; h <= HPS_HARMONICS; ++h) {
    int loH = lo;
    int hiH = std::min(hi, (bins - 1) / h);
    if (loH > hiH)
      continue;
    for (int k = loH; k <= hiH; ++k)
      hps[k] *= mag[k * h];
    for (int k = hiH + 1; k <= hi; ++k)
      hps[k] = 0.0;
  }

  int peak = lo;
  double best = 0.0;
  for (int k = lo; k <= hi; ++k) {
    if (hps[k] > best) {
      best = hps[k];
      peak = k;
    }
  }

  double refined = peak;
  if (peak > 1 && peak < (int)hps.size() - 1) {
    double a = hps[peak - 1], b = hps[peak], c = hps[peak + 1];
    double denom = a - 2.0 * b + c;
    if (std::abs(denom) > 1e-12)
      refined += std::clamp(0.5 * (a - c) / denom, -1.0, 1.0);
  }

  double freq = refined * (double)SAMPLE_RATE / (double)N_;
  if (freq < MIN_FREQ || freq > MAX_FREQ)
    return {0.0, db, false};

  return {kalman_.update(freq), db, true};
}

void HPSDetector::reset() { kalman_.reset(); }

CrossValidator::CrossValidator(int N) : yin_(N), hps_(N) {}

CrossResult CrossValidator::detect(const std::vector<double> &frame) {
  auto yr = yin_.detect(frame);
  auto hr = hps_.detect(frame);
  double db = yr.db;

  if (!yr.valid && !hr.valid)
    return {0.0, 1.0, db, false, false};
  if (yr.valid && !hr.valid)
    return {yr.freq, yr.confidence, db, true, false};
  if (!yr.valid && hr.valid)
    return {hr.freq, 1.0, db, true, false};

  if (std::abs(Music::centsDiff(yr.freq, hr.freq)) <= CROSSVAL_CENTS) {
    return {yr.freq, yr.confidence, db, true, true};
  }
  return {0.0, 1.0, db, false, false};
}

void CrossValidator::reset() {
  yin_.reset();
  hps_.reset();
};
