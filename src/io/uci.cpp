#include <iostream>

#include "uci.h"
#include "../version.h"
#include "../board/board.h"
#include "../utils.h"

namespace {
  void uci_ok() {
    std::cout << "id name " << version::PROGRAM_NAME << std::endl;
    std::cout << "id author John McFarland" << std::endl;
    std::cout << "uciok" << std::endl;
  }

  void parse_go(std::string_view line, const board::Board& b, Agent* agent) {
    auto mv = agent->select_move(b);
    std::cout << "bestmove " << mv << std::endl;
  }

  board::Board parse_pos(std::string_view line) {
    auto slice = line.substr(9);

    board::Board b = [&]() {
      if (slice.rfind("startpos", 0) == 0)
        return board::Board();
      else if (slice.rfind("fen", 0) == 0) {
        slice = slice.substr(4);
        return board::Board(slice);
      }
      else
        // Unexpected input, but just assume startpos
        return board::Board();
    }();

    if (slice.find("moves") != slice.npos) {
      std::string slice_str(slice);
      auto words = utils::split_string(slice_str, ' ' );
      for (auto& word : words) {
        // Todo
      }
    }

    std::cout << b;
    
    return b;
  }
};

namespace uci {

  void uci_loop(Agent* agent) {
    auto b = board::Board();

    uci_ok();

    std::string input;

    while (true) {
      std::cout << std::flush;

      std::getline(std::cin, input);

      if (input[0] == '\n')
        continue;
      else if (input.rfind("isready", 0) == 0)
        std::cout << "readyok" << std::endl;
      else if (input.rfind("position", 0) == 0)
        b = parse_pos(input);
      else if (input.rfind("ucinewgame", 0) == 0)
        b = parse_pos("position startpos\n");
      else if (input.rfind("go", 0) == 0)
        parse_go(input, b, agent);
      else if (input.rfind("uci", 0) == 0)
        uci_ok();
      else if (input.rfind("quit", 0) == 0)
        break;
    }
  };
  
};
