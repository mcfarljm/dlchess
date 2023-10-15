#include "squares.h"

namespace squares {

  Square fr_to_sq(FileRank file, FileRank rank) {
    return file + rank * 8;
  }

};
