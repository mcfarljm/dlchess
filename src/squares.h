#ifndef SQUARES_H_
#define SQUARES_H_

namespace squares {

  using Square = int;
  using FileRank = Square;

  constexpr FileRank FILE_A = 0;
  constexpr FileRank FILE_H = 7;

  constexpr FileRank RANK_1 = 0;
  constexpr FileRank RANK_2 = 1;
  constexpr FileRank RANK_3 = 2;
  // constexpr FileRank RANK_4 = 3;
  // constexpr FileRank RANK_5 = 4;
  constexpr FileRank RANK_6 = 5;
  constexpr FileRank RANK_7 = 6;
  constexpr FileRank RANK_8 = 7;

  constexpr FileRank RANKS[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  constexpr FileRank RANKS_REVERSED[8] = {7, 6, 5, 4, 3, 2, 1, 0};
  constexpr FileRank FILES[8] = {0, 1, 2, 3, 4, 5, 6, 7};

  Square fr_to_sq(FileRank file, FileRank rank) {
    return file + rank * 8;
  }

};

#endif // SQUARES_H_
