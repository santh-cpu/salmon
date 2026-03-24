cmake_minimum_required(VERSION 3.20)
project(salmon_tuner LANGUAGES CXX)

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

target_compile_features(salmon PRIVATE cxx_std_20)
target_include_directories(salmon PRIVATE include)

if(WIN32)
    find_package(portaudio CONFIG QUIET)
    find_package(FFTW3 CONFIG QUIET)

    if(NOT portaudio_FOUND)
        find_path(PORTAUDIO_INCLUDE_DIR portaudio.h HINTS ${PORTAUDIO_DIR}/include)
        find_library(PORTAUDIO_LIBRARY NAMES portaudio portaudio_x64 portaudio_x86 HINTS ${PORTAUDIO_DIR}/lib)
        if(NOT PORTAUDIO_LIBRARY)
            message(FATAL_ERROR "PortAudio not found! Set PORTAUDIO_DIR to its install root.")
        endif()
        target_include_directories(salmon PRIVATE ${PORTAUDIO_INCLUDE_DIR})
        target_link_libraries(salmon PRIVATE ${PORTAUDIO_LIBRARY})
    else()
        target_link_libraries(salmon PRIVATE portaudio)
    endif()

    if(NOT FFTW3_FOUND)
        find_path(FFTW3_INCLUDE_DIR fftw3.h HINTS ${FFTW3_DIR}/include)
        find_library(FFTW3_LIBRARY NAMES fftw3 libfftw3-3 fftw3f HINTS ${FFTW3_DIR}/lib)
        if(NOT FFTW3_LIBRARY)
            message(FATAL_ERROR "FFTW3 not found! Set FFTW3_DIR to its install root.")
        endif()
        target_include_directories(salmon PRIVATE ${FFTW3_INCLUDE_DIR})
        target_link_libraries(salmon PRIVATE ${FFTW3_LIBRARY})
    else()
        if(TARGET FFTW3::fftw3)
            target_link_libraries(salmon PRIVATE FFTW3::fftw3)
        else()
        endif()
    endif()

else()
    find_package(PkgConfig REQUIRED)
    
    pkg_check_modules(PORTAUDIO REQUIRED IMPORTED_TARGET portaudio-2.0)
    pkg_check_modules(FFTW3 REQUIRED IMPORTED_TARGET fftw3)

    target_link_libraries(salmon PRIVATE 
        PkgConfig::PORTAUDIO 
        PkgConfig::FFTW3
    )
    
    if(UNIX AND NOT APPLE)
        target_link_libraries(salmon PRIVATE m)
    endif()
endif()
