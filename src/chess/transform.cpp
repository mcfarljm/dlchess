#include "transform.h"

namespace chess {

  Bitboard transform_bitboard(Bitboard bb, Transform transform) {
    if (! transform.any() || ! bb.any())
      return bb;

    auto new_bb_ul = bb.to_ullong();
    if (transform[static_cast<int>(TransformType::flip_transform)])
      new_bb_ul = reverse_bits_in_bytes(new_bb_ul);
    if (transform[static_cast<int>(TransformType::mirror_transform)])
      new_bb_ul = reverse_bytes_in_bytes(new_bb_ul);

    return Bitboard {new_bb_ul};
  }

  Square transform_square(Square sq, Transform transform) {
    if (! transform.any())
      return sq;
    auto [rank, file] = sq_to_rf(sq);
    if (transform[static_cast<int>(TransformType::flip_transform)])
      // Reverse the file (RANKS_REVERSED=FILES_REVERSED)
      file = RANKS_REVERSED[file];
    if (transform[static_cast<int>(TransformType::mirror_transform)])
      rank = RANKS_REVERSED[rank];
    return fr_to_sq(file, rank);
  }

};
