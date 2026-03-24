#include "tuning_registry.h"
#include <limits>

TuningRegistry::TuningRegistry() {
  auto N = [](const char *name) { return Note{name, Music::parseNote(name)}; };

  presets.push_back({"Standard E (EADGBE)",
                     {N("E2"), N("A2"), N("D3"), N("G3"), N("B3"), N("E4")}});
  presets.push_back({"Drop D (DADGBE)",
                     {N("D2"), N("A2"), N("D3"), N("G3"), N("B3"), N("E4")}});
  presets.push_back({"Open G (DGDGBD)",
                     {N("D2"), N("G2"), N("D3"), N("G3"), N("B3"), N("D4")}});
  presets.push_back({"Open D (DADFsAD)",
                     {N("D2"), N("A2"), N("D3"), N("F#3"), N("A3"), N("D4")}});
  presets.push_back({"Open E (EBEGsBE)",
                     {N("E2"), N("B2"), N("E3"), N("G#3"), N("B3"), N("E4")}});
  presets.push_back(
      {"DADGAD", {N("D2"), N("A2"), N("D3"), N("G3"), N("A3"), N("D4")}});
  presets.push_back(
      {"Half Step Down (Eb)",
       {N("Eb2"), N("Ab2"), N("Db3"), N("Gb3"), N("Bb3"), N("Eb4")}});
  presets.push_back({"Full Step Down (D)",
                     {N("D2"), N("G2"), N("C3"), N("F3"), N("A3"), N("D4")}});
  presets.push_back(
      {"Bass Standard (EADG)", {N("E1"), N("A1"), N("D2"), N("G2")}});
  presets.push_back({"Ukulele (GCEA)", {N("G4"), N("C4"), N("E4"), N("A4")}});
}

TuningPreset &TuningRegistry::active() { return presets[activeIdx]; }

void TuningRegistry::next() {
  activeIdx = (activeIdx + 1) % (int)presets.size();
}

void TuningRegistry::prev() {
  activeIdx = ((activeIdx - 1) + (int)presets.size()) % (int)presets.size();
}

const std::vector<TuningPreset> &TuningRegistry::getPresets() const {
  return presets;
}

int TuningRegistry::getActiveIdx() const { return activeIdx; }

void TuningRegistry::addPreset(const TuningPreset &preset) {
  presets.push_back(preset);
}

void TuningRegistry::setActiveIdx(int idx) {
  if (idx >= 0 && idx < (int)presets.size()) {
    activeIdx = idx;
  }
}

std::pair<int, double> TuningRegistry::closestString(double freq) const {
  int best = 0;
  double bestC = std::numeric_limits<double>::max();
  for (int i = 0; i < (int)presets[activeIdx].strings.size(); ++i) {
    double c =
        std::abs(Music::centsDiff(freq, presets[activeIdx].strings[i].freq));
    if (c < bestC) {
      bestC = c;
      best = i;
    }
  }
  return {best, Music::centsDiff(freq, presets[activeIdx].strings[best].freq)};
}
