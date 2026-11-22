# 📼 Lo-Fi Tape Simulator VST3

[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS-blue.svg)]()

A vintage-style audio effect plugin that simulates the warm, imperfect sound of analog cassette tapes. Built with **JUCE** and **C++**.

![Lo-Fi Tape Simulator UI](assets/lofi-tape-simulator-ui.png)

## ✨ Features

- 🎚️ **Drive**: Analog-style saturation using hyperbolic tangent clipping
- 🎛️ **Filter**: Low-pass filter to mimic limited tape frequency response
- 🌀 **Wow & Flutter**: Realistic pitch modulation using LFO delay lines
- 📻 **Noise**: Tape hiss simulation
- 🎨 **Mix**: Dry/wet balance control
- 🎬 **Animated UI**: Rotating reel-to-reel visualization

## 🎵 Perfect For

- Lo-fi hip hop
- Bedroom pop
- Indie music
- Vintage sound design
- Adding warmth to digital recordings

## 🚀 Installation

### Building from Source

#### Prerequisites

| Tool | Version |
|------|---------|
| CMake | **3.22+** |
| C++ Compiler | C++17 (MSVC 2022 / Xcode / GCC) |
| JUCE | 7.0+ |

> ⚠️ Папка `JUCE` **не включена** в репозиторий (см. `.gitignore`). Её нужно скачать отдельно перед сборкой.

#### Steps

```bash
# 1. Клонируем проект
git clone https://github.com/Hunterice348/Lo-Fi-Tape-Simulator.git
cd Lo-Fi-Tape-Simulator

# 2. Скачиваем JUCE рядом с CMakeLists.txt
git clone https://github.com/juce-framework/JUCE.git

# 3. Сборка (Windows)
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release

# 3. Сборка (macOS)
cmake -B build -G Xcode
cmake --build build --config Release
