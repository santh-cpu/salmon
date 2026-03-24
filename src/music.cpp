#include "music.h"
#include <algorithm>
#include <cmath>

namespace Music {
constexpr double A4_REF = 440.0;
constexpr double CENTS_OCTAVE = 1200.0;

double midiToFreq(int midi) {
  return A4_REF * std::pow(2.0, (midi - 69) / 12.0);
}

double freqToMidi(double f) { return 69.0 + 12.0 * std::log2(f / A4_REF); }

double centsDiff(double measured, double target) {
  return CENTS_OCTAVE * std::log2(measured / target);
}

std::string noteName(int midi) {
  static const char *N[] = {"C",  "C#", "D",  "D#", "E",  "F",
                            "F#", "G",  "G#", "A",  "A#", "B"};
  return std::string(N[midi % 12]) + std::to_string(midi / 12 - 1);
}

double parseNote(const std::string &s) {
  static const std::unordered_map<std::string, int> BASE = {
      {"C", 0},  {"C#", 1}, {"Db", 1},  {"D", 2},   {"D#", 3}, {"Eb", 3},
      {"E", 4},  {"F", 5},  {"F#", 6},  {"Gb", 6},  {"G", 7},  {"G#", 8},
      {"Ab", 8}, {"A", 9},  {"A#", 10}, {"Bb", 10}, {"B", 11}};
  for (int len : {2, 1}) {
    if ((int)s.size() <= len)
      continue;
    auto it = BASE.find(s.substr(0, len));
    if (it == BASE.end())
      continue;
    try {
      int oct = std::stoi(s.substr(len));
      int midi = (oct + 1) * 12 + it->second;
      return midiToFreq(midi);
    } catch (...) {
    }
  }
  throw std::invalid_argument("Cannot parse note: \"" + s + "\"");
}

std::pair<std::string, int> nearestNote(double freq) {
  int midi = std::clamp((int)std::round(freqToMidi(freq)), 0, 127);
  return {noteName(midi), midi};
}
} // namespace Music
