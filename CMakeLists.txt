cmake_minimum_required(VERSION 3.20)
project(salmon_tuner LANGUAGES CXX)

find_path(PORTAUDIO_INCLUDE_DIR portaudio.h PATHS /opt/homebrew/include)
find_library(PORTAUDIO_LIBRARY portaudio PATHS /opt/homebrew/lib)

find_path(FFTW3_INCLUDE_DIR fftw3.h PATHS /opt/homebrew/include)
find_library(FFTW3_LIBRARY fftw3 PATHS /opt/homebrew/lib)

add_executable(salmon
    src/main.cpp
    src/music.cpp
    src/raw_term.cpp
    src/kalman_smoother.cpp
    src/hann_window.cpp
    src/detectors.cpp
    src/tuner_state.cpp
    src/tuning_registry.cpp
    src/audio_capture.cpp
)

target_include_directories(salmon PRIVATE
    include
    ${PORTAUDIO_INCLUDE_DIR}
    ${FFTW3_INCLUDE_DIR}
)

target_link_libraries(salmon PRIVATE
    ${PORTAUDIO_LIBRARY}
    ${FFTW3_LIBRARY}
)

target_compile_features(salmon PRIVATE cxx_std_20)
