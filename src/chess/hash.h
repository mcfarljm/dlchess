#ifndef HASH_H_
#define HASH_H_

#include <iostream>
#include <random>
#include <cstdint>
#include <array>

#include "squares.h"
#include "pieces.h"


namespace chess {

  class Hasher {
  public:
    // Hashing also includes EMPTY pieces
    std::array<std::array<uint64_t, BOARD_SQ_NUM>, NUM_PIECE_TYPES_BOTH + 1> piece_keys;
    uint64_t side_key;
    std::array<uint64_t, 16> castle_keys;

    Hasher() {
      /* std::cout << "Setting hash\n"; */

      /* Note: Tried using a globally defined RNG but the executables hung;
         probably because of initialization order.  Could be reviewed, but for now
         will just use a separate one for this function. */
      std::default_random_engine engine;
      std::uniform_int_distribution<uint64_t> dist;

      side_key = dist(engine);

      for (auto& row : piece_keys)
        for (auto& piece_key : row)
          piece_key = dist(engine);

      for (auto& castle_key : castle_keys)
        castle_key = dist(engine);
    }
  };

};


#endif // HASH_H_
