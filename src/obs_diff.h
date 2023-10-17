#ifndef OBS_DIFF_H
#define OBS_DIFF_H

#include <cstdint>

#include "squares.h"

namespace obs_diff {

  using u64 = uint64_t;

  u64 get_line_attacks(u64 occ, int direction, squares::Square sq);

};

#endif // OBS_DIFF_H
