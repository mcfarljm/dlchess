#include <catch2/catch_test_macros.hpp>
// #include <catch2/benchmark/catch_benchmark.hpp>

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

#include "bitboard.h"
#include "board/board.h"
#include "board/piece_moves.h"
#include "board/game_moves.h"


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

TEST_CASE( "Init board", "[board]" ) {
  auto b = board::Board::from_fen(board::START_FEN);

  REQUIRE( b.king_sq[0] == 4 );
  REQUIRE( b.king_sq[1] == 60 );

  std::stringstream ss;
  ss << std::endl << b;

  auto s = R"(
8     r  n  b  q  k  b  n  r  
7     p  p  p  p  p  p  p  p  
6     .  .  .  .  .  .  .  .  
5     .  .  .  .  .  .  .  .  
4     .  .  .  .  .  .  .  .  
3     .  .  .  .  .  .  .  .  
2     P  P  P  P  P  P  P  P  
1     R  N  B  Q  K  B  N  R  

      a  b  c  d  e  f  g  h  
side: w
enPas: 64
castle: KQkq
)";

  REQUIRE( ss.str() == s );
  REQUIRE( b.check() );

}


TEST_CASE( "Rook attacks", "[attacks]" ) {
  Bitboard occ;
  auto attacks = piece_moves::get_rook_attacks(0, occ);
  auto vec = std::vector<Square>(attacks.begin(), attacks.end());
  std::vector<Square> expected = {1, 2, 3, 4, 5, 6, 7, 8, 16, 24, 32, 40, 48, 56};
  REQUIRE( vec == expected );

  occ.set_bit(4);
  occ.set_bit(32);
  attacks = piece_moves::get_rook_attacks(0, occ);
  vec = std::vector<Square>(attacks.begin(), attacks.end());
  expected = {1, 2, 3, 4, 8, 16, 24, 32};
  REQUIRE( vec == expected );
}

TEST_CASE( "King moves", "[attacks]" ) {
  auto moves = std::vector<Square>(piece_moves::king_moves[9].begin(),
                                   piece_moves::king_moves[9].end());
  std::vector<Square> expected = {0, 1, 2, 8, 10, 16, 17, 18};
  REQUIRE( moves == expected );

  moves = std::vector<Square>(piece_moves::king_moves[0].begin(),
                              piece_moves::king_moves[0].end());
  expected = {1, 8, 9};
  REQUIRE( moves == expected );

  moves = std::vector<Square>(piece_moves::king_moves[63].begin(),
                              piece_moves::king_moves[63].end());
  expected = {54, 55, 62};
  REQUIRE( moves == expected );
}

TEST_CASE( "Move string", "[moves]" ) {
  auto mv = game_moves::Move(squares::Position::C1,
                             squares::Position::C3,
                             Piece::none, Piece::WR,
                             game_moves::MoveFlag::none);
  std::stringstream ss;
  ss << mv;
  REQUIRE( ss.str() == "c1c3r" );
}

/// Utility function to check move count for a given FEN string.
void check_move_count(std::string_view fen, long num_moves) {
  auto b = board::Board::from_fen(fen);

  auto ml = b.generate_all_moves();
  REQUIRE( ml.moves.size() == num_moves );
}

TEST_CASE( "White pawn start", "[movegen]" ) {
  // These were originally used in VICE to check pawn moves, of which there are
  // 26.
  const auto pawn_moves_w = "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
  check_move_count(pawn_moves_w, 42);
}

TEST_CASE( "Black pawn start", "[movegen]" ) {
  const auto pawn_moves_b = "rnbqkbnr/p1p1p3/3p3p/1p1p4/2P1Pp2/8/PP1P1PpP/RNBQKB1R b KQkq e3 0 1";
  check_move_count(pawn_moves_b, 42);
}

TEST_CASE( "Castling", "[movegen]" ) {
  const auto castle_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
  check_move_count(castle_fen, 48);
}
