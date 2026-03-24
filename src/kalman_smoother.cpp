#include "kalman_smoother.h"

double KalmanSmoother::update(double z) {
  if (!init_) {
    x_ = z;
    init_ = true;
    return z;
  }
  double pp = p_ + q_;
  double k = pp / (pp + r_);
  x_ += k * (z - x_);
  p_ = (1.0 - k) * pp;
  return x_;
}

void KalmanSmoother::reset() {
  init_ = false;
  p_ = 1000;
}
