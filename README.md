#  salmon

Guitar Tuner that lives in your Terminal and actually works

Mic-assisted and accurate Tuner for the Terminal
Completely offline and portable (as portable as your machine atleast)

pluck a string. it listens and tells you what to do. that's it, go tune your guitar

<img width="1582" height="959" alt="salmon tuner in action" src="https://github.com/user-attachments/assets/95a3e72e-4785-42c1-a42b-b0b5b5568f18" />

-----

## Why salmon

salmon runs **two Pitch Detection algorithms at the same time (YIN and HPS)**. YIN algorithm excels in detecting pitch of low frequency signals and fumbles in the higher range. HPS, however excels in the higher frequency range as per observation. the final reading is only displayed when both algorithms agree (within 25 cents), proving to be more accurate than either of them individually.

on top of that, a **Kalman Filter** smooths the output over time so the display doesn't flicker around when detecting small disturbances.

the Audio Thread runs on a **Lock-free Ring Buffer** so the mic capture and the pitch processing never wait on each other. most naive implementations use a mutex here, which means the audio thread can stall mid-capture while processing catches up. that stall is a dropped sample. enough dropped samples and the pitch reading gets inaccurate. the Ring Buffer lets both threads run independently at full speed.

-----

## Quick Start

### macOS

#### App 
(recommended, fullscreen for optimal tuning experience)

```bash
brew install --cask santh-cpu/salmon/salmon
```
\n
(as a CLI tool, then call `salmon` anywhere)

```bash
brew install santh-cpu/salmon/salmon
```

### Linux

```bash
git clone https://github.com/santh-cpu/salmon.git
cd salmon
sudo apt install g++ portaudio19-dev libfftw3-dev
g++ -O3 -std=c++20 src/*.cpp -o salmon -Iinclude -lportaudio -lfftw3 -lpthread -lm
./salmon
```

### Windows (CMake)

```bash
vcpkg install portaudio fftw3

git clone https://github.com/santh-cpu/salmon.git
cd salmon && mkdir build && cd build
cmake .. && cmake --build .
./salmon
```

-----

## Controls

|key      |does                     |
|---------|-------------------------|
|`n` / `p`|next / previous preset   |
|`0`      |auto-detect mode         |
|`1`–`8`  |lock to a specific string|
|`c`      |create a Custom Tuning   |
|`r`      |reset                    |
|`q`      |quit                     |

-----

## Custom Tunings

<img width="1582" height="959" alt="custom tuning input" src="https://github.com/user-attachments/assets/7e1dfd30-46e5-49c3-8c80-efc194bc64cf" />

press `c` and type a name followed by the notes you want, like:

```
skinnylove C2 G2 E3 G3 C4 C4
```

-----

## Technical Highlights

- **YIN**: known for its excellent accuracy in Pitch Detection, with error rates significantly lower than many competing methods. performance can drop in high-noise environments
- **HPS (Harmonic Product Spectrum)**: generally effective at identifying fault components and distinguishing the Fundamental Frequency from noise. generally not suitable for identifying pitch in very low-frequency ranges (e.g., below 50 Hz)
- **Cross-validation**: both algorithms run in parallel. if they land within 25 cents of each other, the reading is trusted. if not, salmon stays quiet rather than guessing
- **Kalman Filter**: smooths pitch over time. responsive to real changes, resistant to jitter
- **Hann Window + Overlap-add**: standard DSP trick to keep spectral analysis clean at frame boundaries
- **Lock-free SPSC Ring Buffer**: the Audio Capture thread and the processing thread never block each other

-----

credit: got a little help with the difficult [DSP](https://publications.hnu.de/3642/1/Bachelorthesis.pdf) math \>w<
