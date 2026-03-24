# salmon

A mic-assisted, accurate guitar tuner for the terminal
Completely offline and portable (as portable as your machine atleast)

Pluck the string and wait for isntruction to tune up or down based on whatever tuning you wish to achieve

Create a custom tuning to tune your guitar to for maximum creative control

<img width="1582" height="959" alt="Screenshot 2026-03-24 at 5 28 31 pm" src="https://github.com/user-attachments/assets/95a3e72e-4785-42c1-a42b-b0b5b5568f18" />

## Why salmon stands out

- Extremely stable pitch detection — combines YIN and HPS algorithms with cross-validation
- Almost never gives false readings, even with background noise
- Supports all common guitar, bass, and ukulele tunings
- One-key custom tuning support — add any tuning instantly
- Clean, low-latency UI with smooth real-time feedback
- Works perfectly on Linux, macOS, and Windows
- Single-purpose, lightweight

## Custom Tuning

<img width="1582" height="959" alt="Screenshot 2026-03-24 at 5 29 21 pm" src="https://github.com/user-attachments/assets/7e1dfd30-46e5-49c3-8c80-efc194bc64cf" />

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
g++ -O3 -std=c++20 src/*.cpp -o salmon -Iinclude -lportaudio -lfftw3 -lpthread -lm
./salmon
```

### macOS

```bash
brew install portaudio fftw
g++ -O3 -std=c++20 src/*.cpp -o salmon -Iinclude -I/opt/homebrew/include -L/opt/homebrew/lib -lportaudio -lfftw3 -lpthread -lm
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

## Technical Highlights

- **Lock-free SPSC ring buffer** (ensures real-time audio processing without blocking, so input stays fast and consistent) 

- **YIN algorithm** (a well-known time-domain method for detecting the fundamental frequency. It’s good at locking onto the actual note, even when the signal is slightly off) 

- **Harmonic Product Spectrum (HPS)** (a frequency-domain technique that strengthens the true pitch by combining harmonic information. This helps in cases where overtones might confuse simpler detectors) 

- **Cross-validation (YIN + HPS)** (both detectors run in parallel and agree on the result. If they don’t match, the system avoids jumping to unstable readings) 

- **Kalman filter** (used to smooth the detected pitch over time, reducing jitter while still staying responsive to real changes) 

- **Overlap-add with Hann window** — standard DSP technique to reduce spectral artifacts and keep frequency analysis clean  

---

Credit: Claude wonderfully helped with all the tough math and signal processing \>w<
