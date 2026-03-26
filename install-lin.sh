#!/usr/bin/env bash
set -e

# salmon installer for Linux
# Usage: curl -fsSL https://raw.githubusercontent.com/santh-cpu/salmon/main/install.sh | bash

REPO="https://github.com/santh-cpu/salmon.git"
INSTALL_DIR="$HOME/.local/bin"
BUILD_DIR="$(mktemp -d)"

RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
BOLD='\033[1m'
RESET='\033[0m'

echo -e "${CYAN}${BOLD}"
echo "  🐟 salmon installer"
echo -e "${RESET}"

# ── Detect package manager ──────────────────────────────────────────────────
install_deps() {
  echo -e "${BOLD}Installing dependencies...${RESET}"
  if command -v apt-get &>/dev/null; then
    sudo apt-get install -y -q g++ portaudio19-dev libfftw3-dev git
  elif command -v dnf &>/dev/null; then
    sudo dnf install -y gcc-c++ portaudio-devel fftw-devel git
  elif command -v pacman &>/dev/null; then
    sudo pacman -S --noconfirm gcc portaudio fftw git
  elif command -v zypper &>/dev/null; then
    sudo zypper install -y gcc-c++ portaudio-devel fftw3-devel git
  else
    echo -e "${RED}Could not detect a supported package manager (apt/dnf/pacman/zypper).${RESET}"
    echo "Please manually install: g++, portaudio dev, fftw3 dev, git"
    exit 1
  fi
}

# ── Check deps, install if missing ─────────────────────────────────────────
MISSING=0
command -v g++    &>/dev/null || MISSING=1
pkg-config --exists portaudio-2.0 2>/dev/null || MISSING=1
pkg-config --exists fftw3        2>/dev/null || MISSING=1

if [ "$MISSING" -eq 1 ]; then
  install_deps
fi

# ── Clone & build ───────────────────────────────────────────────────────────
echo -e "${BOLD}Cloning salmon...${RESET}"
git clone --depth=1 "$REPO" "$BUILD_DIR/salmon" -q

echo -e "${BOLD}Building...${RESET}"
cd "$BUILD_DIR/salmon"
g++ -O3 -std=c++20 src/*.cpp -o salmon -Iinclude -lportaudio -lfftw3 -lpthread -lm

# ── Install binary ──────────────────────────────────────────────────────────
mkdir -p "$INSTALL_DIR"
cp salmon "$INSTALL_DIR/salmon"
chmod +x "$INSTALL_DIR/salmon"

# ── PATH check ──────────────────────────────────────────────────────────────
if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
  echo ""
  echo -e "${CYAN}Adding $INSTALL_DIR to your PATH...${RESET}"
  SHELL_RC="$HOME/.bashrc"
  [[ "$SHELL" == */zsh ]] && SHELL_RC="$HOME/.zshrc"
  [[ "$SHELL" == */fish ]] && SHELL_RC="$HOME/.config/fish/config.fish"

  if [[ "$SHELL" == */fish ]]; then
    echo "fish_add_path $INSTALL_DIR" >> "$SHELL_RC"
  else
    echo "export PATH=\"\$PATH:$INSTALL_DIR\"" >> "$SHELL_RC"
  fi
  echo -e "  Added to ${SHELL_RC}. Run: ${BOLD}source $SHELL_RC${RESET}"
fi

# ── Cleanup ──────────────────────────────────────────────────────────────────
rm -rf "$BUILD_DIR"

echo ""
echo -e "${GREEN}${BOLD}✓ salmon installed!${RESET}"
echo -e "  Run ${BOLD}salmon${RESET} anywhere to open the tuner."
echo ""
