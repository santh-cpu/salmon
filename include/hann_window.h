#pragma once
#include <vector>

class HannWindow {
  std::vector<double> w_;

public:
  explicit HannWindow(int n);
  void apply(double *d, int n) const;
};
