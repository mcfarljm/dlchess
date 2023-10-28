#include "piece_moves.h"
#include "../obs_diff.h"

using obs_diff::get_line_attacks;

namespace piece_moves {

  Bitboard get_rook_attacks(Square sq, Bitboard occ_bb) {
    auto occ = occ_bb.to_ullong();
    return Bitboard(get_line_attacks(occ, 0, sq) |
                    get_line_attacks(occ, 1, sq));
  }

  Bitboard get_bishop_attacks(Square sq, Bitboard occ_bb) {
    auto occ = occ_bb.to_ullong();
    return Bitboard(get_line_attacks(occ, 2, sq) |
                    get_line_attacks(occ, 3, sq));
  }

  Bitboard get_queen_attacks(Square sq, Bitboard occ_bb) {
    return get_rook_attacks(sq, occ_bb) | get_bishop_attacks(sq, occ_bb);
  }

  std::array<Bitboard,64> get_king_moves() {
    std::array<Bitboard,64> bitboards;

    for (Square sq=0; sq<64; ++sq) {
      auto rank = sq / 8;
      auto file = sq % 8;

      // N
      if (rank<7)
        bitboards[sq].set_bit(sq+8);
      // NE
      if (rank<7 && file<7)
        bitboards[sq].set_bit(sq+9);
      // E
      if (file<7)
        bitboards[sq].set_bit(sq+1);
      // SE
      if (file<7 && rank>0)
        bitboards[sq].set_bit(sq-7);
      // S
      if (rank>0)
        bitboards[sq].set_bit(sq-8);
      // SW
      if (rank>0 && file>0)
        bitboards[sq].set_bit(sq-9);
      // W
      if (file>0)
        bitboards[sq].set_bit(sq-1);
      // NW
      if (file>0 && rank<7)
        bitboards[sq].set_bit(sq+7);
    }

    return bitboards;
  }

  std::array<Bitboard,64> king_moves = get_king_moves();

  std::array<Bitboard,64> get_knight_moves() {
    std::array<Bitboard,64> bitboards;

    for (Square sq=0; sq<64; ++sq) {
      auto rank = sq / 8;
      auto file = sq % 8;

      // NNE
      if (rank<6 && file<7)
        bitboards[sq].set_bit(sq+17);
      // ENE
      if (rank<7 && file<6)
        bitboards[sq].set_bit(sq+10);
      // ESE
      if (rank>0 && file<6)
        bitboards[sq].set_bit(sq-6);
      // SSE
      if (rank>1 && file<7)
        bitboards[sq].set_bit(sq-15);
      // SSW
      if (rank>1 && file>0)
          bitboards[sq].set_bit(sq-17);
      // WSW
      if (rank>0 && file>1)
        bitboards[sq].set_bit(sq-10);
      // WNW
      if (rank<7 && file>1)
        bitboards[sq].set_bit(sq+6);
      // NNW
      if (rank<6 && file>0)
        bitboards[sq].set_bit(sq+15);
    }

    return bitboards;
  }

  std::array<Bitboard,64> knight_moves = get_knight_moves();

  std::array<Bitboard,64> get_white_pawn_attacks() {
    std::array<Bitboard,64> bitboards;
    for (Square sq=0; sq<64; ++sq) {
      // Left captures
      bitboards[sq].set_bit(sq);
      bitboards[sq] = Bitboard((bitboards[sq] & ~ bitboard::BB_FILE_A) << 7);

      // Right captures
      bitboards[sq].set_bit(sq);
      bitboards[sq] = Bitboard((bitboards[sq] & ~ bitboard::BB_FILE_H) << 9);
    }
    return bitboards;
  }

  std::array<Bitboard,64> get_black_pawn_attacks() {
    std::array<Bitboard,64> bitboards;
    for (Square sq=0; sq<64; ++sq) {
      // Left captures
      bitboards[sq].set_bit(sq);
      bitboards[sq] = Bitboard((bitboards[sq] & ~ bitboard::BB_FILE_A) >> 9);

      // Right captures
      bitboards[sq].set_bit(sq);
      bitboards[sq] = Bitboard((bitboards[sq] & ~ bitboard::BB_FILE_H) >> 7);
    }
    return bitboards;
  }

  std::array<Bitboard,64> white_pawn_attacks = get_white_pawn_attacks();
  std::array<Bitboard,64> black_pawn_attacks = get_black_pawn_attacks();
  
};
