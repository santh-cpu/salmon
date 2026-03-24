#pragma once
#include <termios.h>

class RawTerm {
public:
  RawTerm();
  ~RawTerm();
  int readKey();

private:
#if !defined(_WIN32)
  struct termios orig_{};
  bool active_ = false;
#endif
};
