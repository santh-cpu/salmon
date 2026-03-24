#include "raw_term.h"
#include <iostream>

#if defined(_WIN32)
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

RawTerm::RawTerm() {
#if defined(_WIN32)
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD mode = 0;
  GetConsoleMode(h, &mode);
  SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#else
  if (!isatty(STDIN_FILENO))
    return;
  tcgetattr(STDIN_FILENO, &orig_);
  struct termios raw = orig_;
  raw.c_iflag &= ~(unsigned)(ICRNL | IXON);
  raw.c_lflag &= ~(unsigned)(ECHO | ICANON | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
  active_ = true;
#endif
}

RawTerm::~RawTerm() {
#if !defined(_WIN32)
  if (active_)
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_);
#endif
}

int RawTerm::readKey() {
#if defined(_WIN32)
  return _kbhit() ? _getch() : -1;
#else
  unsigned char c = 0;
  return (::read(STDIN_FILENO, &c, 1) == 1) ? (int)c : -1;
#endif
}
