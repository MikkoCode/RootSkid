# RootSkid

RootSkid is a lightweight native Windows desktop application written in modern C++ using the Win32 API. It has no Qt dependency and requires no additional runtime installation.

## Download

Download the latest ready-to-run Windows executable from the [GitHub Releases page](https://github.com/MikkoCode/RootSkid/releases/latest).

1. Download `RootSkid.exe`.
2. Run the executable on Windows.
3. Select **Run check** to verify the application is working.

## Features

- Native Win32 graphical interface
- Responsive window layout
- Unicode support
- Light application theme with a dark title bar
- No third-party GUI framework or runtime dependency
- CMake build configuration for CLion, MinGW, and compatible Windows toolchains

## Requirements

To run the prebuilt release:

- Windows 10 or Windows 11

To build from source:

- CMake 3.20 or newer
- A C++20 compiler
- MinGW-w64 or Microsoft Visual C++
- Ninja, Visual Studio, or another CMake-supported build system

## Build from source

### MinGW and Ninja

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The resulting executable is located at `build/RootSkid.exe`.

### Visual Studio

```powershell
cmake -S . -B build
cmake --build build --config Release
```

The resulting executable is normally located at `build/Release/RootSkid.exe`.

## Project structure

```text
RootSkid/
|-- CMakeLists.txt   CMake build configuration
|-- main.cpp         Win32 GUI application
`-- README.md        Project documentation
```

## Technology

- C++20
- Win32 API
- Windows Common Controls
- Desktop Window Manager API
- CMake

## Releases

Versioned binaries are published on the [Releases page](https://github.com/MikkoCode/RootSkid/releases). The current stable release is [v1.0.0](https://github.com/MikkoCode/RootSkid/releases/tag/v1.0.0).
