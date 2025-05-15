cmake_minimum_required(VERSION 3.14)
project(rippope VERSION 0.1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTS "Build test programs" ON)
option(ENABLE_SANITIZERS "Enable address sanitizer in debug builds" OFF)

# Add debugging flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(ENABLE_SANITIZERS)
        if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
            set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined")
            set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined")
        endif()
    endif()
endif()

# Setup include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Handle raylib dependency
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0  # Using v5.0 release - adjust as needed
)
# Customize raylib build options
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

# Define source files
set(SOURCES
    src/main.c
    src/piece_table.c
    src/clay_utils/clay_renderer_raylib.c
)

# Main application
add_executable(clay_test ${SOURCES})
target_include_directories(clay_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(clay_test PRIVATE raylib)

# Conditional platform-specific settings
if(APPLE)
    target_link_libraries(clay_test PRIVATE "-framework Cocoa" "-framework IOKit" "-framework OpenGL")
elseif(WIN32)
    # Windows-specific libraries
    target_link_libraries(clay_test PRIVATE winmm)
elseif(UNIX AND NOT APPLE)
    # Linux-specific libraries
    find_package(X11 REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(Threads REQUIRED)
    target_link_libraries(clay_test PRIVATE X11 GL m pthread ${CMAKE_DL_LIBS})
endif()

# Tests
if(BUILD_TESTS)
    # Enable testing
    enable_testing()
    # Include tests directory
    add_subdirectory(src/tests)
endif()

# Installation
install(TARGETS clay_test
    RUNTIME DESTINATION bin
)

# Configure resource paths for cross-platform compatibility
target_compile_definitions(clay_test PRIVATE
    RESOURCE_PATH="${CMAKE_INSTALL_PREFIX}/share/rippope/"
)

# Copy any resource files to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/test.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/tests)

# Create resources directory and add font
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts)
endif()

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts/FiraCode-Regular.ttf)
    message(STATUS "Font file not found - downloading Fira Code font")
    file(DOWNLOAD
        https://github.com/tonsky/FiraCode/raw/master/distr/ttf/FiraCode-Regular.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts/FiraCode-Regular.ttf
        SHOW_PROGRESS
    )
endif()

# Install resources
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources
        DESTINATION share/rippope)

# Copy resources to build directory for testing
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})