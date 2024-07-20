cmake_minimum_required(VERSION 3.12)

project(dlchess LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS "ON" CACHE INTERNAL "")


option(ONNXRUNTIME_ROOTDIR "onnxruntime root dir")

include_directories("${ONNXRUNTIME_ROOTDIR}/include"                           # Pre-built package
                    "${ONNXRUNTIME_ROOTDIR}/include/onnxruntime"               # Linux local install to /usr/local
                    "${ONNXRUNTIME_ROOTDIR}/include/onnxruntime/core/session") # Windows local install

link_directories("${ONNXRUNTIME_ROOTDIR}/lib")


Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)

FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts
  GIT_TAG        v3.1.1
)

FetchContent_MakeAvailable(cxxopts)


# To use:
# - only works with static linking
# - compile in new dir: cmake .. -DCMAKE_BUILD_TYPE=PROFILE
# - run executable as normal
# - run gprof -P <exename> > prof.out
# The -P is optional but produces only the call graph, which is much easier to use.
set( CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_DEBUG} -pg" )

set(sources
  src/utils.cpp
  src/version.cpp
  src/myrand.cpp
  src/chess/squares.cpp
  src/chess/bitboard.cpp
  src/chess/pieces.cpp
  src/chess/board.cpp
  src/chess/obs_diff.cpp
  src/chess/piece_moves.cpp
  src/chess/game_moves.cpp
  src/chess/movegen.cpp
  src/chess/makemove.cpp
  src/chess/perft.cpp
  src/chess/io.cpp

  src/agent_random.cpp
  src/simulation.cpp

  src/zero/encoder.cpp
  src/zero/agent_zero.cpp
  src/zero/experience.cpp
  src/zero/cached_inference.cpp

  src/io/uci.cpp
)


# Absolute paths needed for clang-tidy, probably good practice in general.
list(TRANSFORM sources PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)


add_library(dlchesslib SHARED ${sources})

# set_target_properties(dlchess PROPERTIES CXX_STANDARD 20)

target_link_libraries(dlchesslib onnxruntime)

add_executable(tests src/test.cpp)
target_link_libraries(tests PRIVATE dlchesslib Catch2::Catch2WithMain)

add_executable(selfplay src/selfplay.cpp)
target_link_libraries(selfplay PRIVATE dlchesslib cxxopts)

add_executable(dlchess src/main.cpp)
target_link_libraries(dlchess PRIVATE dlchesslib cxxopts)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)


add_custom_target(
  check clang-tidy -p ${CMAKE_BINARY_DIR}/compile_commands.json
  --header-filter=${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${sources}
  DEPENDS ${sources}
)
