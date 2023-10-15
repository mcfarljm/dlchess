#ifndef HASH_H_
#define HASH_H_

#include <iostream>
#include <random>
#include <cstdint>

#include "../pieces.h"


namespace board {

  class Hasher {
  public:
    // Hashing also includes EMPTY pieces
    uint64_t piece_keys[pieces::NUM_PIECE_TYPES_BOTH + 1][BOARD_SQ_NUM];
    uint64_t side_key;
    uint64_t castle_keys[16];

    Hasher() {
      /* std::cout << "Setting hash\n"; */

      /* Note: Tried using a globally defined RNG but the executables hung;
         probably because of initialization order.  Could be reviewed, but for now
         will just use a separate one for this function. */
      std::default_random_engine engine;
      std::uniform_int_distribution<uint64_t> dist;

      side_key = dist(engine);

      for (int i=0; i<pieces::NUM_PIECE_TYPES_BOTH + 1; ++i) {
        for (int j=0; j<BOARD_SQ_NUM; ++j)
          piece_keys[i][j] = dist(engine);
      }

      for (int i=0; i<16; ++i)
        castle_keys[i] = dist(engine);
    }
  };

};


#endif // HASH_H_
