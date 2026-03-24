#pragma once

class KalmanSmoother {
  double x_ = 0, p_ = 1000, q_ = 0.1, r_ = 20.0;
  bool init_ = false;

public:
  double update(double z);
  void reset();
};
