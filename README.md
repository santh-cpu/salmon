# salmon

A mic-assisted, accurate guitar tuner for the terminal
Completely offline and portable (as portable as your machine atleast)

Pluck the string and wait for isntruction to tune up or down based on whatever tuning you wish to achieve

Create a custom tuning to tune your guitar to for maximum creative control

## Why salmon stands out

- Extremely stable pitch detection — combines YIN and HPS algorithms with cross-validation
- Almost never gives false readings, even with background noise
- Supports all common guitar, bass, and ukulele tunings
- One-key custom tuning support — add any tuning instantly
- Clean, low-latency UI with smooth real-time feedback
- Works perfectly on Linux, macOS, and Windows
- Single-purpose, lightweight

## Controls

- n / p — next / previous preset
- 0 — auto detect mode
- 1–8 — lock to specific string
- c — create custom tuning
- r — reset
- q — quit

Build once and run the program for on-the-go offline mic assisted guitar tuning. 

Pluck the string and wait for feedback and instructions

## Quick Start

### Linux

```bash
sudo apt install portaudio19-dev libfftw3-dev
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

install vcpkg
```bash
vcpkg install portaudio fftw3
```

### CMake
Download above dependencies the same way 
```bash
mkdir build && cd build
cmake ..
cmake --build .
./salmon
```

## What i've actually done

- Lock-free ring buffer for real-time audio
- Dual pitch detectors (YIN + HPS) with cross-validation
- Kalman smoothing and stability gating for jitter-free results
- Minimal dependencies (only PortAudio and FFTW3)

Open source. Feel free to fork, improve, or just use it to tune your guitar in peace.

Credits: Claude wonderfully helped with all the tough math and signal processing. tnx babe \>w<
