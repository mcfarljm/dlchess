#ifndef TRANSFORM_BOARD_
#define TRANSFORM_BOARD_

#include <bitset>

#include "bitboard.h"

namespace chess {

  enum class TransformType {
    // Horizontal mirror, reverse_bits_in_bytes
    flip_transform,
    // Vertical mirror, reverse_bytes_in_bytes
    mirror_transform,
    // Diagonal tranpose A1 to H8, tranpose_bits_in_bytes
    tranpose_transform,
  };

  using Transform = std::bitset<3>;

  // Flip bitboard horizontally.
  inline uint64_t reverse_bits_in_bytes(uint64_t v) {
    v = ((v >> 1) & 0x5555555555555555ull) | ((v & 0x5555555555555555ull) << 1);
    v = ((v >> 2) & 0x3333333333333333ull) | ((v & 0x3333333333333333ull) << 2);
    v = ((v >> 4) & 0x0F0F0F0F0F0F0F0Full) | ((v & 0x0F0F0F0F0F0F0F0Full) << 4);
    return v;
  }

  // Flip bitboard vertically.
  inline uint64_t reverse_bytes_in_bytes(uint64_t v) {
    v = (v & 0x00000000FFFFFFFF) << 32 | (v & 0xFFFFFFFF00000000) >> 32;
    v = (v & 0x0000FFFF0000FFFF) << 16 | (v & 0xFFFF0000FFFF0000) >> 16;
    v = (v & 0x00FF00FF00FF00FF) << 8 | (v & 0xFF00FF00FF00FF00) >> 8;
    return v;
  }

  using Transform = std::bitset<3>;


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

};

#endif //TRANSFORM_BOARD_
