salmo# salmon

A fast, accurate, no-nonsense guitar tuner for the terminal.

## Why salmon stands out

- Extremely stable pitch detection — combines YIN and HPS algorithms with cross-validation
- Almost never gives false readings, even with background noise
- Supports all common guitar, bass, and ukulele tunings
- One-key custom tuning support — add any tuning instantly
- Clean, low-latency UI with smooth real-time feedback
- Works perfectly on Linux, macOS, and Windows
- Single-purpose, lightweight, and actually fun to use

## Controls

- n / p — next / previous preset
- 0 — auto detect mode
- 1–8 — lock to specific string
- c — create custom tuning
- r — reset
- q — quit

Just plug in your guitar, run it, and tune.

## Quick Start

### Linux

```bash
g++ -O3 -std=c++20 src/*.cpp -o salmon -lportaudio -lfftw3 -lpthread -lm
./salmon
```

### macOS

```bash
brew install portaudio fftw
g++ -O3 -std=c++20 src/*.cpp -o salmon \
  -I/opt/homebrew/include -L/opt/homebrew/lib \
  -lportaudio -lfftw3 -lpthread -lm
./salmon
```

### Windows

Use vcpkg (portaudio + fftw3) + CMake, or MSYS2. Full Windows support is built-in.

## For developers

- Clean C++20 multi-file architecture
- Lock-free ring buffer for real-time audio
- Dual pitch detectors (YIN + HPS) with cross-validation
- Kalman smoothing and stability gating for jitter-free results
- Minimal dependencies (only PortAudio and FFTW3)

Open source. Feel free to fork, improve, or just use it to tune your guitar in peace.
