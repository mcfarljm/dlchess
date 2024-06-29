#include <cassert>

#include "board.h"

namespace board {
  std::optional<game_moves::Move> Board::parse_move_string(std::string_view str) {
    if (str.size() < 4)
      return std::nullopt;
    if (str[0] > 'h' || str[0] < 'a')
      return std::nullopt;
    if (str[2] > 'h' || str[2] < 'a')
      return std::nullopt;
    if (str[1] > '8' || str[1] < '1')
      return std::nullopt;
    if (str[3] > '8' || str[3] < '1')
      return std::nullopt;

    auto from = squares::fr_to_sq(str[0] - 'a', str[1] - '1');
    auto to = squares::fr_to_sq(str[2] - 'a', str[3] - '1');
    assert(squares::square_on_board(from));
    assert(squares::square_on_board(to));

    auto move_list = generate_all_moves();

    for (auto& mv : move_list.moves) {
      if (mv.from == from && mv.to == to) {
        if (mv.promote.exists()) {
          // Return nullopt if the input is missing a promotion character
          if (str.size() < 5)
            return std::nullopt;
          if ((mv.promote.is_queen() && str[4] == 'q') ||
              (mv.promote.is_rook() && str[4] == 'r') ||
              (mv.promote.is_bishop() && str[4] == 'b') ||
              (mv.promote.is_knight() && str[4] == 'n'))
            return mv;
        }
        else
          // No promotion
          return mv;
      }
    }

    return std::nullopt;
  }
};
