#pragma once
#include <string>
#include <unordered_map>
#include <utility>

namespace Music {
double midiToFreq(int midi);
double freqToMidi(double f);
double centsDiff(double measured, double target);
std::string noteName(int midi);
double parseNote(const std::string &s);
std::pair<std::string, int> nearestNote(double freq);
} // namespace Music
