#ifndef OBS_DIFF_H
#define OBS_DIFF_H

#include "squares.h"

namespace chess {

  squares::u64 get_line_attacks(squares::u64 occ, int direction, squares::Square sq);

};

#endif // OBS_DIFF_H
