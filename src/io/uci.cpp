#include <iostream>

#include "uci.h"
#include "../version.h"
#include "../board/board.h"
#include "../utils.h"

namespace {
  void uci_ok() {
    std::cout << "id name " << version::PROGRAM_NAME << std::endl;
    std::cout << "id author John McFarland" << std::endl;

    // std::cout << std::endl;
    // std::cout << "option name OwnBook type check default true" << std::endl;
    std::cout << "option name playouts type spin default 800 min 1 max 100000" << std::endl;
    std::cout << "option name noise type check default false" << std::endl;

    std::cout << "uciok" << std::endl;
  }

  void parse_go(std::string& line, const board::Board& b, Agent* agent) {
    std::optional<int> move_time_ms;
    std::optional<int> time_left_ms;
    std::optional<int> inc_ms;

    auto words = utils::split_string(line, ' ');
    for (auto i=0; i<words.size(); ++i) {
      // std::cout << "parse_go: " << words[i] << "\n";
      if ((words[i] == "winc" && b.side == Color::white) ||
          (words[i] == "binc" && b.side == Color::black)) {
        inc_ms = std::stoi(words[i+1]);
      }
      else if ((words[i] == "wtime" && b.side == Color::white) ||
               (words[i] == "btime" && b.side == Color::black)) {
        time_left_ms = std::stoi(words[i+1]);
      }
      // else if (words[i] == "movestogo") {
      // }
      else if (words[i] == "movetime") {
        move_time_ms = std::stoi(words[i+1]);
      }
      // else if (words[i] == "depth") {
      // }
    }
    agent->set_search_time(move_time_ms, time_left_ms, inc_ms, b);
    auto mv = agent->select_move(b);
    std::cout << "bestmove " << mv << std::endl;
  }

  board::Board parse_pos(std::string_view line) {
    auto slice = line.substr(9);

    board::Board b = [&]() {
      if (slice.rfind("startpos", 0) == 0)
        return board::Board(); // NOLINT(bugprone-branch-clone)
      else if (slice.rfind("fen", 0) == 0) {
        slice = slice.substr(4);
        return board::Board(slice);
      }
      else
        // Unexpected input, but just assume startpos
        return board::Board(); // NOLINT(bugprone-branch-clone)
    }();

    auto i = slice.find("moves");
    if (i != slice.npos) {
      const std::string slice_str(slice.substr(i+6));
      auto words = utils::split_string(slice_str, ' ');
      for (auto& word : words) {
        auto mv = b.parse_move_string(word);
        if (mv)
          b.make_move(mv.value());
        else
          break;
      }
    }

    // std::cout << b;
    
    return b;
  }

  /// Parse command of form "setoption name <name> value <value>".
  void parse_setoption(const std::string& line, zero::ZeroAgent* agent) {
    auto words = utils::split_string(line, ' ');
    if (words.size() < 5)
      return;
    if (words[2] == "playouts") {
      try {
        agent->info.num_rounds = stoi(words[4]);
      } catch (const std::invalid_argument& e) {} // NOLINT(bugprone-empty-catch)
    }
    else if (words[2] == "noise") {
      if (words[4] == "true")
        agent->info.add_noise = true;
      else if (words[4] == "false")
        agent->info.add_noise = false;
    }
  }
};

namespace uci {

  void uci_loop(zero::ZeroAgent* agent) {
    auto b = board::Board();

    agent->info.game_mode = zero::GameMode::uci;

    uci_ok();

    std::string input;

    while (true) {
      std::cout << std::flush;

      std::getline(std::cin, input);

      if (input[0] == '\n')
        continue;
      else if (input.rfind("uci", 0) == 0)
        uci_ok();
      else if (input.rfind("isready", 0) == 0)
        std::cout << "readyok" << std::endl;
      else if (input.rfind("position", 0) == 0)
        b = parse_pos(input);
      else if (input.rfind("ucinewgame", 0) == 0)
        b = parse_pos("position startpos\n");
      else if (input.rfind("setoption", 0) == 0)
        parse_setoption(input, agent);
      else if (input.rfind("go", 0) == 0)
        parse_go(input, b, agent);
      else if (input.rfind("quit", 0) == 0)
        break;
    }
  };
  
};
