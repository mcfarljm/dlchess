#include "piece_moves.h"
#include "../obs_diff.h"

using obs_diff::get_line_attacks;

namespace piece_moves {

  Bitboard get_rook_attacks(Square sq, Bitboard occ_bb) {
    auto occ = occ_bb.bits.to_ullong();
    return Bitboard(get_line_attacks(occ, 0, sq) |
                    get_line_attacks(occ, 1, sq));
  }

  Bitboard get_bishop_attacks(Square sq, Bitboard occ_bb) {
    auto occ = occ_bb.bits.to_ullong();
    return Bitboard(get_line_attacks(occ, 2, sq) |
                    get_line_attacks(occ, 3, sq));
  }

  Bitboard get_queen_attacks(Square sq, Bitboard occ_bb) {
    return get_rook_attacks(sq, occ_bb) | get_bishop_attacks(sq, occ_bb);
  }
  
};
