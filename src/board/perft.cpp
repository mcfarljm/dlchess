#include <cassert>
#include <iostream>
#include "board.h"

namespace board {
  long Board::perft(int depth) {
    assert(check());

    if (depth == 0)
      return 1;

    auto move_list = generate_all_moves();

    long count = 0;
    long new_count;
    for (auto& mv : move_list.moves) {
      // std::cout << "trying " << mv << ": ";
      auto legal = make_move(mv);
      // std::cout << legal << std::endl;
      if (! legal)
        continue;
      new_count = perft(depth - 1);
      // if (depth == 2)
      //   std::cout << "depth(" << depth << "), move " << mv << ": " << new_count << std::endl;
      count += new_count;
      undo_move();
    }
    
    return count;
  }
};
