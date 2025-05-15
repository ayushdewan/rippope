# CMAKE Build System for Rippope

## Overview

This project uses CMake as its build system generator, allowing it to be built on multiple platforms including Linux, macOS, and Windows. The CMake configuration automatically handles dependencies, compiler flags, and platform-specific settings.

## Directory Structure

```
rippope/
├── CMakeLists.txt              # Main CMake configuration
├── include/                    # Header files
│   ├── clay_utils/             # Clay library headers
│   │   ├── clay.h
│   │   └── clay_renderer_raylib.h
│   └── piece_table.h           # Piece table implementation header
├── src/                        # Source files
│   ├── clay_utils/             # Clay library source
│   │   └── clay_renderer_raylib.c
│   ├── tests/                  # Test sources
│   │   ├── CMakeLists.txt      # Test-specific CMake config
│   │   ├── main.c              # Test program
│   │   └── test.txt            # Test data
│   ├── main.c                  # Main application source
│   └── piece_table.c           # Piece table implementation
├── resources/                  # Application resources
│   └── fonts/                  # Font files
└── build.sh                    # Build helper script
```

## Build Options

The CMake configuration provides the following options:

- `BUILD_TESTS` (ON/OFF): Controls whether test programs are built (default: ON)
- `ENABLE_SANITIZERS` (ON/OFF): Enables address and undefined behavior sanitizers in debug builds (default: OFF)

## Dependencies

### Raylib

The build system automatically downloads and compiles Raylib from GitHub during the build process. By default, it uses version 5.0, but this can be changed in the CMakeLists.txt file.

### Clay

Clay is included directly in the source tree in the `clay_utils/` directory, so no external dependency is required.

## Platform-Specific Configuration

The CMake setup handles platform-specific requirements:

### Linux
- Links against X11, OpenGL, Threads, and dynamic loading libraries
- Uses system fonts if available, falls back to bundled fonts

### macOS
- Links against Cocoa, IOKit, and OpenGL frameworks
- Uses bundled fonts

### Windows
- Links against WinMM for multimedia support
- Uses bundled fonts

## Building from the Command Line

### Linux/macOS

```bash
mkdir -p build
cd build
cmake ..
make
```

### Windows (with Visual Studio)

```cmd
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Using the build script

For convenience, a build script is provided:

```bash
./build.sh
```

### Using the build script

For convenience, a build script is provided:

```bash
./build.sh
```

## Running the Application

After building, the executable will be located in the build directory:

```bash
# From the build directory
./clay_test
```

## Running Tests

```bash
# From the build directory
./piece_table_test tests/test.txt
```

## Installing

```bash
# From the build directory
cmake --install .
```

This will install the executable and resources to the default system location (configurable with CMAKE_INSTALL_PREFIX).

## Customizing the Build

To customize your build, you can pass options to CMake:

```bash
cmake -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release ..
```

## Cross-Compiling

For cross-compilation, you'll need to provide a toolchain file:

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=path/to/toolchain.cmake ..
```

## Troubleshooting

1. **Missing dependencies**: If CMake cannot find required dependencies, install the development packages for X11, OpenGL, etc.

2. **Font issues**: If you experience problems with fonts, check that the fonts are properly located in the resources/fonts directory.

3. **Compilation errors**: Make sure your compiler supports C11.