#ifndef SQUARES_H_
#define SQUARES_H_

#include <cstdint>
#include <string>
#include <array>

namespace squares {

  constexpr int BOARD_SQ_NUM = 64;
  constexpr int GRID_SIZE = 8;

  // Todo: There might be a better place to define this.  For now putting it
  // here for accessibility by other modules.
  using u64 = uint64_t;

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

  Square fr_to_sq(FileRank file, FileRank rank);
  std::array<int, 2> sq_to_rf(Square);

  namespace Position {

    // Using unscoped enum inside a namespace provides a namsepace scope but
    // without the need to cast to int.
    enum Position {
      A1, B1, C1, D1, E1, F1, G1, H1,
      A2, B2, C2, D2, E2, F2, G2, H2,
      A3, B3, C3, D3, E3, F3, G3, H3,
      A4, B4, C4, D4, E4, F4, G4, H4,
      A5, B5, C5, D5, E5, F5, G5, H5,
      A6, B6, C6, D6, E6, F6, G6, H6,
      A7, B7, C7, D7, E7, F7, G7, H7,
      A8, B8, C8, D8, E8, F8, G8, H8,
      none,
    };

  };

  std::string square_string(Square sq);

};

#endif // SQUARES_H_
