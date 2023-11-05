cmake_minimum_required(VERSION 3.12)

project(dlchess LANGUAGES CXX)

find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

set(CMAKE_CXX_STANDARD 20)

# find_package(Torch REQUIRED)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)

FetchContent_MakeAvailable(Catch2)

# FetchContent_Declare(
#   cxxopts
#   GIT_REPOSITORY https://github.com/jarro2783/cxxopts
#   GIT_TAG        v3.1.1
# )

# FetchContent_MakeAvailable(cxxopts)


# To use:
# - only works with static linking
# - compile in new dir: cmake .. -DCMAKE_BUILD_TYPE=PROFILE
# - run executable as normal
# - run gprof -P <exename> > prof.out
# The -P is optional but produces only the call graph, which is much easier to use.
set( CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_DEBUG} -pg" )

add_library(dlchess SHARED
  src/utils.cpp
  src/myrand.cpp
  src/squares.cpp
  src/bitboard.cpp
  src/pieces.cpp
  src/board/board.cpp
  src/obs_diff.cpp
  src/board/piece_moves.cpp
  src/board/game_moves.cpp
  src/board/movegen.cpp
  src/board/makemove.cpp
  src/board/perft.cpp

  src/agent_random.cpp
  src/simulation.cpp

  src/zero/encoder.cpp
)

# set_target_properties(dlchess PROPERTIES CXX_STANDARD 20)

target_link_libraries(dlchess "${TORCH_LIBRARIES}")

add_executable(tests src/test.cpp)
target_link_libraries(tests PRIVATE dlchess Catch2::Catch2WithMain)

add_executable(matchup src/matchup.cpp)
target_link_libraries(matchup PRIVATE dlchess)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)
