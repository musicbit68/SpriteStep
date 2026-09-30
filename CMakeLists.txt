cmake_minimum_required(VERSION 3.16)
project(SpriteStep LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

add_executable(SpriteStep src/main.cpp)
target_link_libraries(SpriteStep PRIVATE SpriteStepCore)

add_executable(SpriteStepTests
    tests/test_engine.cpp
)
target_link_libraries(SpriteStepTests PRIVATE SpriteStepCore)

enable_testing()
add_test(NAME SpriteStepTests COMMAND SpriteStepTests)
