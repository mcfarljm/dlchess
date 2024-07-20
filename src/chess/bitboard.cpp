#include <iostream>

#include "bitboard.h"


namespace chess {

  const Bitboard BB_FILE_A {0x0101010101010101};
  const Bitboard BB_FILE_H {0x8080808080808080};
  const Bitboard BB_RANK_4 {0x00000000FF000000};
  const Bitboard BB_RANK_5 {0x000000FF00000000};

  std::ostream& operator<<(std::ostream& os, const Bitboard& b) {
    for (auto rank : RANKS_REVERSED) {
      for (auto file : FILES) {
        auto sq = fr_to_sq(file, rank);
        os << (b[sq] ? "x" : "-");
      }
      os << std::endl;
    }
    return os;
  }

};
