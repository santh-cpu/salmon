#include "hann_window.h"
#include <cmath>

HannWindow::HannWindow(int n) : w_(n) {
  for (int i = 0; i < n; ++i)
    w_[i] = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (n - 1)));
}

void HannWindow::apply(double *d, int n) const {
  for (int i = 0; i < n; ++i)
    d[i] *= w_[i];
}
