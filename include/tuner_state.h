#pragma once
#include <atomic>
#include <deque>
#include <mutex>

enum class DisplayState { WAITING, LIVE, FROZEN };

struct TunerState {
  std::atomic<double> freq{0.0};
  std::atomic<double> db{-120.0};
  std::atomic<DisplayState> displayState{DisplayState::WAITING};
  std::atomic<int> activeStr{0};
  std::atomic<double> centsOff{0.0};
  std::atomic<bool> listening{false};
  std::atomic<int> lockedStr{-1};

  std::mutex histMu;
  std::deque<double> freqHistory;
  std::deque<double> stableBuffer;

  bool gateFreq(double f, bool crossValidated);
  double getStableMean();
  void pushFreq(double f);
  void clearHistory();
};
