#include <catch2/catch_test_macros.hpp>
// #include <catch2/benchmark/catch_benchmark.hpp>

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

#include "bitboard.h"


TEST_CASE( "Test bb string empty", "[bitboard]" ) {
  auto bb = bitboard::Bitboard();

  std::stringstream ss;
  ss << std::endl << bb;

  auto s = R"(
--------
--------
--------
--------
--------
--------
--------
--------
)";

  REQUIRE( ss.str() == s );
}


TEST_CASE( "Test bb string 9", "[bitboard]" ) {
  auto bb = bitboard::Bitboard();
  bb.set_bit(9);

  std::stringstream ss;
  ss << std::endl << bb;

  auto s = R"(
--------
--------
--------
--------
--------
--------
-x------
--------
)";

  REQUIRE( ss.str() == s );
}

TEST_CASE( "Test bb string 9, 44", "[bitboard]" ) {
  auto bb = bitboard::Bitboard();
  bb.set_bit(9);
  bb.set_bit(44);

  std::stringstream ss;
  ss << std::endl << bb;

  auto s = R"(
--------
--------
----x---
--------
--------
--------
-x------
--------
)";

  REQUIRE( ss.str() == s );
}


TEST_CASE( "Test bb count", "[bitboard]" ) {
  auto bb = bitboard::Bitboard();
  bb.set_bit(9);
  bb.set_bit(44);
  REQUIRE( bb.count() == 2 );
}


TEST_CASE( "Empty bb iter", "[bitboard]" ) {
  auto bb = bitboard::Bitboard();
  auto vec = std::vector<Square>(bb.begin(), bb.end());
  REQUIRE( vec.size() == 0 );
}


TEST_CASE( "bb iter", "[bitboard]" ) {
  auto expected = std::vector<Square>({9, 25, 44});
  auto bb = bitboard::Bitboard();
  for (auto i : expected)
    bb.set_bit(i);
  auto vec = std::vector<Square>(bb.begin(), bb.end());
  std::sort(vec.begin(), vec.end());
  REQUIRE( vec == expected );
}
