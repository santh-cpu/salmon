#include "tuner_state.h"
#include "music.h"
#include <numeric>

constexpr int STABLE_FRAMES = 3;
constexpr double STABLE_CENTS = 25.0;
constexpr int UNVALIDATED_EXTRA_FRAMES = 3;
constexpr int HIST = 10;

bool TunerState::gateFreq(double f, bool crossValidated) {
  int required =
      crossValidated ? STABLE_FRAMES : STABLE_FRAMES + UNVALIDATED_EXTRA_FRAMES;
  if (stableBuffer.empty()) {
    stableBuffer.push_back(f);
    return false;
  }
  double currentMean =
      std::accumulate(stableBuffer.begin(), stableBuffer.end(), 0.0) /
      stableBuffer.size();
  if (std::abs(Music::centsDiff(f, currentMean)) <= STABLE_CENTS) {
    stableBuffer.push_back(f);
    if ((int)stableBuffer.size() > required + 2)
      stableBuffer.pop_front();
    return (int)stableBuffer.size() >= required;
  } else {
    stableBuffer.clear();
    stableBuffer.push_back(f);
    return false;
  }
}

double TunerState::getStableMean() {
  if (stableBuffer.empty())
    return 0.0;
  return std::accumulate(stableBuffer.begin(), stableBuffer.end(), 0.0) /
         stableBuffer.size();
}

void TunerState::pushFreq(double f) {
  std::lock_guard<std::mutex> g(histMu);
  freqHistory.push_back(f);
  if ((int)freqHistory.size() > HIST)
    freqHistory.pop_front();
}

void TunerState::clearHistory() {
  std::lock_guard<std::mutex> g(histMu);
  freqHistory.clear();
  stableBuffer.clear();
}
