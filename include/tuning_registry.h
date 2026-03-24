#pragma once
#include "music.h"
#include <string>
#include <vector>

struct Note {
  std::string name;
  double freq;
};

struct TuningPreset {
  std::string name;
  std::vector<Note> strings;
};

class TuningRegistry {
  std::vector<TuningPreset> presets;
  int activeIdx = 0;

public:
  TuningRegistry();
  TuningPreset &active();
  void next();
  void prev();
  std::pair<int, double> closestString(double freq) const;
  const std::vector<TuningPreset> &getPresets() const;
  int getActiveIdx() const;
  void addPreset(const TuningPreset &preset);
  void setActiveIdx(int idx);
};
