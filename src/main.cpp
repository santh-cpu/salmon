#include "audio_capture.h"
#include "detectors.h"
#include "music.h"
#include "raw_term.h"
#include "tuner_state.h"
#include "tuning_registry.h"
#include <chrono>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace Ansi {
const char *CLEAR = "\033[2J\033[H";
const char *HIDE_CURSOR = "\033[?25l";
const char *SHOW_CURSOR = "\033[?25h";
inline std::string move(int row, int col) {
  return "\033[" + std::to_string(row) + ";" + std::to_string(col) + "H";
}
} // namespace Ansi

constexpr int DISPLAY_FPS = 20;
static std::atomic<bool> g_running{true};

void sigHandler(int) { g_running = false; }

int getTerminalWidth() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w.ws_col;
}

const std::vector<std::string> ART = {
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣴⣦⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⣤⣤⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⣾⣿⣿⢿⣷⠀⠀⢠⣾⣿⣿⣷⣦⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⢀⣄⡀⠀⢸⣿⣿⣿⣿⣦⡀⠀⠀⠀⠀⠀⢀⣴⣿⣿⣿⣿⡿⢈⣿⡆⢠⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⣶⣿⣿⣿⣦⠀⣿⡏⢿⣿⣿⣿⣦⣤⣤⣤⣤⣿⣿⣿⣿⣿⣿⡇⣸⣿⡇⢸⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⢻⣿⣿⣿⣿⣷⣿⣧⠈⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣿⣿⡇⣿⣿⣿⣿⣿⣿⣿⠏⠀⠀⠀⣀⡀⣠⠀⡤⢰⠄⠀",
    "⠻⣿⣿⣿⣿⣿⣿⣿⣾⣿⢛⣍⡻⣿⣿⣿⡟⣱⣦⡙⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠏⠀⠀⠀⠀⣽⡗⢺⡶⡷⠿⢻⡆",
    "⠀⠙⢿⣿⣿⣿⣟⣿⣿⡇⣸⣿⣧⢸⣿⣿⡁⣿⣿⡇⢹⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠏⠀⠀⢀⣤⠾⠛⠃⠈⠁⣠⡴⠞⠀",
    "⠀⠀⠈⠻⣿⣿⣿⣿⣿⣧⢹⣿⡏⣼⣿⣿⣧⣻⡿⣣⣿⣿⣿⣿⣿⡿⣻⣿⣿⣿⣿⠃⢀⣠⠾⠋⠙⣆⢀⣀⣠⡾⠋⠀⠀⠀",
    "⠀⠀⠀⠀⠙⣿⣿⣿⣿⣿⣷⣭⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⢡⣿⣿⣿⣿⣧⡾⠛⠹⣄⢀⣠⡾⠛⠁⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠈⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⢁⣾⣿⣿⠿⠛⢻⡄⢀⣠⠿⠛⢁⣤⣾⣿⣷⠀⣀⣤⣶⡄",
    "⠀⠀⠀⠀⠀⠀⠈⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⢀⣾⡿⠛⢧⡀⢀⣠⡿⢛⣁⠀⣠⣾⣿⣿⣿⣿⣿⣿⣿⣿⠃",
    "⠀⠀⠀⠀⠀⠀⠀⠘⣿⣿⣿⣿⣿⣿⠟⠁⠀⠙⣿⣿⣿⣿⡴⠛⣇⠀⢀⣨⡿⠛⠁⣰⣿⣿⣷⣿⣿⣿⣿⣿⣿⣿⣿⣿⣁⡀",
    "⠀⠀⠀⠀⠀⠀⠀⣀⣸⣿⣿⣿⡿⠋⢀⣀⣀⣀⣸⡿⠟⢹⡄⢀⣸⣷⣿⣿⣇⠀⣰⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡗",
    "⠀⠀⠀⠀⢀⣴⠿⠛⠛⠛⠉⠉⣴⠞⠉⢡⡽⠛⠉⢧⣀⣠⣿⣿⣿⣿⣿⣿⣿⢠⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⠋⠀",
    "⠀⠀⠀⣰⡿⠁⠀⠀⠀⠀⠀⢸⣅⠀⠀⠈⠳⣄⣠⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⠀⠀⠀⠀",
    "⠀⠀⢀⣿⢁⣀⠀⠀⠀⣠⠖⠉⠈⢷⡀⠀⠀⠈⠁⠈⠛⠛⣛⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠿⠿⠟⠉⠀⠀⠀⠀",
    "⠀⠀⢸⣿⠈⠉⠀⠀⠘⢷⡀⠀⠀⣠⡗⠀⠀⠀⠀⠀⢀⣈⣋⣤⣿⣿⣿⣿⣿⣿⣿⣿⣿⠿⠛⠋⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠸⣿⡄⠀⠀⠀⠀⠈⢻⡴⠛⢁⣹⣶⠀⠀⣠⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠟⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⢻⣷⠀⠀⠀⠀⠀⠀⣀⣴⡿⠛⠙⣧⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠈⢻⣷⡀⠀⠀⢠⣾⠟⠉⠀⣠⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠙⠿⣶⣤⣀⣀⣠⣴⣾⣿⣿⣿⡟⠉⠉⠉⠙⣿⣿⣿⣿⣿⣿⣿⣿⣿⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠀⠈⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⠀⠀⠈⢿⣿⣿⣿⣿⣿⣿⣿⣿⣆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠀⣠⣿⣿⣿⣿⣿⣿⣿⣿⠃⠀⠀⠀⠀⠀⠀⠀⠻⣿⣿⣿⣿⣿⣿⣿⣿⠆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠸⣿⣿⣿⣿⣿⣿⣿⣿⡏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠻⠿⠿⠿⠿⠟⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠀⠙⠿⣿⣿⣿⣿⣿⠟⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀"};

void renderTUI(TunerState &st, TuningRegistry &reg, const std::string &status,
               bool enteringCustom, const std::string &customBuffer) {
  std::ostringstream ss;
  ss << Ansi::CLEAR;
  if (enteringCustom) {
    ss << "Salmon - Custom Tuning\n\n";
    ss << "Example: Custom E2 A2 D3 G3 B3 E4\n\n";
    ss << "Input: " << customBuffer << "\n\n";
    ss << "ENTER = add\nESC = cancel\nq = quit\n";
  } else {
    const auto &preset = reg.active();
    int locked = st.lockedStr.load();
    int active = st.activeStr.load();
    bool isLock = (locked >= 0 && locked < (int)preset.strings.size());
    double freq = st.freq.load();
    double db = st.db.load();
    double cents = st.centsOff.load();
    DisplayState ds = st.displayState.load();

    ss << "Salmon\n\n";
    ss << "Preset: " << preset.name << "\n";
    ss << "String: "
       << (isLock ? "Locked string " + std::to_string(locked + 1) : "Auto")
       << " (0 = auto, 1-" << preset.strings.size() << " = lock)\n";
    ss << "\n\nStrings:";
    ss << " |";
    for (int i = 0; i < (int)preset.strings.size(); ++i) {
      char mark = (isLock && i == locked) ? '*' : (i == active ? '>' : ' ');
      ss << " " << mark << " " << (i + 1) << ":" << preset.strings[i].name
         << "   |";
    }
    ss << "\n\n\n";
    if (ds == DisplayState::LIVE || ds == DisplayState::FROZEN) {
      auto [noteName, midi] = Music::nearestNote(freq);
      const auto &tgt = preset.strings[active];
      ss << "Detected: " << std::fixed << std::setprecision(2) << freq << " Hz "
         << noteName << "\n";
      ss << "Target  : " << std::fixed << std::setprecision(2) << tgt.freq
         << " Hz" << " " << tgt.name << "\n";
      ss << "Cents: " << std::showpos << std::fixed << std::setprecision(1)
         << cents << std::noshowpos;
      if (std::abs(cents) <= 5.0)
        ss << "\n\n\t\t(IN TUNE)\t\t\n\n";
      else if (cents < 0)
        ss << "\n\n\t\t(TUNE UP " << std::abs(cents)
           << " cents - flat)\t\t\n\n";
      else
        ss << "\n\n\t\t(TUNE DOWN " << std::abs(cents)
           << " cents - sharp)\t\t\n\n";
      ss << "\n";
      if (ds == DisplayState::FROZEN)
        ss << "[FROZEN - pluck again to update]\n";
    } else {
      ss << "Pluck a string to begin...\n\n";
    }
    ss << "Level: " << std::fixed << std::setprecision(1) << db << " dB";
    if (ds == DisplayState::FROZEN)
      ss << " [frozen]";
    else if (st.listening.load())
      ss << " [live]";
    ss << "\n\n";
    ss << "Controls:    n/p = preset        r = reset       c = custom      q "
          "= quit\n";
    if (!status.empty())
      ss << "Status: " << status << "\n";
    ss << "\nPresets:\n";
    const auto &presets = reg.getPresets();
    int activeIdx = reg.getActiveIdx();

    for (int i = 0; i < (int)presets.size(); ++i) {
      ss << (i == activeIdx ? " > " : " ") << presets[i].name << "\n";
    }
  }

  std::cout << ss.str();

  int termWidth = getTerminalWidth();
  int artWidth = 80;
  int startCol = termWidth - artWidth - 2;
  int startRow = 2;

  for (int i = 0; i < (int)ART.size(); ++i) {
    std::cout << Ansi::move(startRow + i, startCol) << ART[i];
  }

  std::cout << std::flush;
}

void audioPipeline(TunerState &st, TuningRegistry &reg) {
  CrossValidator cv(8192);
  std::vector<double> overlap(8192, 0.0);
  std::vector<double> hop;
  double envelopeDb = -120.0;
  try {
    auto cap = std::make_unique<AudioCapture>();
    while (g_running) {
      if (!cap->read(hop)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        continue;
      }
      std::rotate(overlap.begin(), overlap.begin() + 1024, overlap.end());
      std::copy(hop.begin(), hop.end(), overlap.end() - 1024);

      auto res = cv.detect(overlap);
      st.db.store(res.db);

      if (res.db < -50.0) {
        if (st.displayState.load() == DisplayState::LIVE)
          st.displayState.store(DisplayState::FROZEN);
        st.listening.store(false);
        st.stableBuffer.clear();
        cv.reset();
        envelopeDb = -120.0;
        continue;
      }

      st.listening.store(true);
      if (res.db > envelopeDb + 10.0) {
        st.stableBuffer.clear();
        cv.reset();
      }
      envelopeDb = std::max(res.db, envelopeDb - 0.5);

      if (res.valid) {
        if (st.gateFreq(res.freq, res.crossValidated)) {
          double gatedFreq = st.getStableMean();
          st.freq.store(gatedFreq);
          st.pushFreq(gatedFreq);

          int lk = st.lockedStr.load();
          const auto &strs = reg.active().strings;
          int idx;
          double centsOff;
          if (lk >= 0 && lk < (int)strs.size()) {
            idx = lk;
            centsOff = Music::centsDiff(gatedFreq, strs[lk].freq);
          } else {
            auto [ai, ac] = reg.closestString(gatedFreq);
            idx = ai;
            centsOff = ac;
          }
          st.activeStr.store(idx);
          st.centsOff.store(centsOff);
          st.displayState.store(DisplayState::LIVE);
        }
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "\nAudio error: " << e.what() << "\n";
    g_running = false;
  }
}

int main() {
  std::signal(SIGINT, sigHandler);
  std::signal(SIGTERM, sigHandler);
  std::cout << Ansi::HIDE_CURSOR << Ansi::CLEAR << std::flush;

  TunerState st;
  TuningRegistry reg;
  RawTerm term;
  std::string status;
  bool enteringCustom = false;
  std::string customBuffer;

  std::thread audioThread([&]() { audioPipeline(st, reg); });

  auto lastDraw = std::chrono::steady_clock::now();
  auto drawInterval = std::chrono::milliseconds(1000 / DISPLAY_FPS);

  while (g_running) {
    int key = term.readKey();
    if (key != -1) {
      if (enteringCustom) {
        if (key == 10 || key == 13) {
          enteringCustom = false;
          std::istringstream iss(customBuffer);
          std::vector<std::string> tokens;
          std::string token;
          while (iss >> token)
            tokens.push_back(token);

          if (!tokens.empty()) {
            std::string cname = tokens[0];
            std::vector<Note> cstrings;
            bool ok = true;
            for (size_t i = 1; i < tokens.size(); ++i) {
              try {
                double f = Music::parseNote(tokens[i]);
                cstrings.push_back({tokens[i], f});
              } catch (...) {
                ok = false;
                status = "Invalid note: " + tokens[i];
                break;
              }
            }
            if (ok && !cstrings.empty()) {
              reg.addPreset({cname, cstrings});
              reg.setActiveIdx((int)reg.getPresets().size() - 1);
              st.lockedStr.store(-1);
              st.clearHistory();
              st.displayState.store(DisplayState::WAITING);
              status = "Custom added: " + cname;
            }
          }
        } else if (key == 27) {
          enteringCustom = false;
          status = "Custom input canceled";
        } else if (key == 8 || key == 127) {
          if (!customBuffer.empty())
            customBuffer.pop_back();
        } else if (key >= 32 && key <= 126) {
          customBuffer += static_cast<char>(key);
        } else if (key == 'q' || key == 'Q' || key == 3) {
          g_running = false;
        }
      } else {
        switch (key) {
        case 'q':
        case 'Q':
        case 3:
          g_running = false;
          break;
        case 'n':
        case 'N':
          reg.next();
          st.lockedStr.store(-1);
          st.clearHistory();
          st.displayState.store(DisplayState::WAITING);
          status = "-> " + reg.active().name;
          break;
        case 'p':
        case 'P':
          reg.prev();
          st.lockedStr.store(-1);
          st.clearHistory();
          st.displayState.store(DisplayState::WAITING);
          status = "<- " + reg.active().name;
          break;
        case '0':
          st.lockedStr.store(-1);
          status = "Auto-detect mode";
          break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8': {
          int want = key - '1';
          if (want < (int)reg.active().strings.size()) {
            st.lockedStr.store(want);
            status = "Locked: String " + std::to_string(want + 1);
          }
          break;
        }
        case 'r':
        case 'R':
          st.clearHistory();
          st.displayState.store(DisplayState::WAITING);
          status = "Reset.";
          break;
        case 'c':
        case 'C':
          enteringCustom = true;
          customBuffer.clear();
          status = "Custom Tuning: Name Note1 Note2 ...";
          break;
        }
      }
    }

    auto now = std::chrono::steady_clock::now();
    if (now - lastDraw >= drawInterval) {
      renderTUI(st, reg, status, enteringCustom, customBuffer);
      lastDraw = now;
      if (!status.empty())
        status.clear();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  g_running = false;
  audioThread.join();
  std::cout << Ansi::SHOW_CURSOR << Ansi::CLEAR << "exit\n";
  return 0;
}
