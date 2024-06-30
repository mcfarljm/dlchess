#include "squares.h"

namespace squares {

  Square fr_to_sq(FileRank file, FileRank rank) {
    return file + rank * 8;
  }

  std::array<int, 2> sq_to_rf(Square sq) {
    return {sq / 8, sq % 8};
  }

  std::string square_string(Square sq) {
    std::string s;
    s += static_cast<char>('a' + sq%8);
    s += static_cast<char>('1' + sq/8);
    return s;
  }

};
