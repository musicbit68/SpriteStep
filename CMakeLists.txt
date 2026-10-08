cmake_minimum_required(VERSION 3.16)
project(SpriteStep LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Current SpriteStep architecture: the new core remains the authoritative development/test library.
add_library(SpriteStepCore
    src/core/Parameter.cpp
    src/project/Pattern.cpp
    src/project/Track.cpp
    src/project/Project.cpp
    src/project/Instrument.cpp
    src/sequencer/Transport.cpp
    src/sequencer/TrackPlayer.cpp
    src/sequencer/PatternLauncher.cpp
    src/sequencer/Sequencer.cpp
)
target_include_directories(SpriteStepCore PUBLIC src)

# Keep the small console harness available for core development, but do not use it as the
# handheld application. The graphical application is built from the restored SDL shell below.
add_executable(SpriteStepCoreDemo src/main.cpp)
target_link_libraries(SpriteStepCoreDemo PRIVATE SpriteStepCore)

add_executable(SpriteStepTests
    tests/test_engine.cpp
)
target_link_libraries(SpriteStepTests PRIVATE SpriteStepCore)

enable_testing()
add_test(NAME SpriteStepTests COMMAND SpriteStepTests)

# Graphical desktop/handheld shell.
#
# The pre-2.0 shell is deliberately restored as an application layer rather than as a wholesale
# project revert. Its native engine/UI are kept intact for this first reconnection step; the new
# SpriteStepCore library above remains present and tested independently while the model adapter is
# introduced at the application seam.
add_subdirectory(shell)
