#include "squares.h"

namespace squares {

  Square fr_to_sq(FileRank file, FileRank rank) {
    return file + rank * 8;
  }

  std::string square_string(Square sq) {
    std::string s;
    s += ('a' + sq%8);
    s += ('1' + sq/8);
    return s;
  }

};
