#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <string>
#include <array>
#include <filesystem>

#include <cxxopts.hpp>

#include "board/board.h"
#include "zero/agent_zero.h"
#include "utils.h"
#include "simulation.h"


using namespace zero;
using namespace utils;


std::unique_ptr<Agent> load_zero_agent(const std::string network_path,
                                       std::optional<int> num_threads_option,
                                       SearchInfo info) {

  auto model = std::make_shared<InferenceModel>(network_path.c_str(), num_threads_option);

  std::cout << "Loaded: " << network_path << std::endl;

  auto encoder = std::make_shared<SimpleEncoder>();
  auto agent = std::make_unique<ZeroAgent>(model, encoder, info);
  return agent;
}


int main(int argc, const char* argv[]) {

  cxxopts::Options options("evaluate", "Pair two agents against each other");

  options.add_options()
    ("agent1", "Netowrk path or 'random'", cxxopts::value<std::string>())
    ("agent2", "Netowrk path or 'random'", cxxopts::value<std::string>())
    ("r,rounds", "Number of rounds", cxxopts::value<int>()->default_value("800"))
    ("g,num-games", "Number of games", cxxopts::value<int>()->default_value("1"))
    ("m,max-moves", "Maximum moves per game", cxxopts::value<int>()->default_value("10000"))
    ("num-random-moves", "Number of randomized moves", cxxopts::value<int>()->default_value("16"))
    ("v,verbosity", "Verbosity level", cxxopts::value<int>()->default_value("0"))
    ("t,num-threads", "Number of pytorch threads", cxxopts::value<int>())
    ("h,help", "Print usage")
    ;

  options.parse_positional({"agent1", "agent2"});
  options.positional_help("<agent1> <agent2>");

  cxxopts::ParseResult args;
  try {
    args = options.parse(argc, argv);
  }
  catch (const cxxopts::exceptions::exception& e) {
    std::cout << options.help() << std::endl;
    exit(1);
  }

  if (args.count("help")) {
    std::cout << options.help() << std::endl;
    exit(0);
  }

  if (! args.count("agent1") || ! args.count("agent2")) {
    std::cout << options.help() << std::endl;
    exit(1);
  }

  auto num_rounds = args["rounds"].as<int>();
  auto num_randomized_moves = args["num-random-moves"].as<int>();
  auto num_games = args["num-games"].as<int>();
  auto max_moves = args["max-moves"].as<int>();
  auto verbosity = args["verbosity"].as<int>();

  std::optional<int> num_threads_option;
  if (args.count("num-threads")) {
    std::cout << "setting " << args["num-threads"].as<int>() << " inference threads" << std::endl;
    num_threads_option = args["num-threads"].as<int>();
  }

  zero::SearchInfo info;
  info.num_rounds = num_rounds;
  info.num_randomized_moves = num_randomized_moves;
  info.add_noise = true;

  auto agent1 = load_zero_agent(args["agent1"].as<std::string>(), num_threads_option, info);
  auto agent2 = load_zero_agent(args["agent1"].as<std::string>(), num_threads_option, info);

  if (! agent1 || ! agent2)
    return -1;

  Agent* white_agent;
  Agent* black_agent;

  int agent1_wins = 0;
  int draws = 0;
  int agent1_wins_as_black = 0;
  int agent1_draws_as_black = 0;
  int agent1_num_black_games = 0;
  int total_num_moves = 0;
  auto cumulative_timer = Timer();
  for (int game_num=0; game_num < num_games; ++game_num) {
    if (game_num % 2 == 0) {
      // Agent 1 plays white on even games
      white_agent = agent1.get();
      black_agent = agent2.get();
    } else {
      white_agent = agent2.get();
      black_agent = agent1.get();
      ++agent1_num_black_games;
    }
    auto timer = Timer();
    auto [winner, num_moves] = simulate_game(white_agent, black_agent, verbosity, max_moves);
    auto duration = timer.elapsed();
    total_num_moves += num_moves;
    if (num_games <= 5) {
      std::cout << "Game: " << num_moves << " moves in " << duration;
      std:: cout << " s (" << num_moves / duration << " mv/s, " << duration / num_moves << " s/mv)" << std::endl;
    }

    if (winner == Color::both)
      ++draws;

    if (game_num % 2 == 0) {
      // Agent 1 plays white
      if (winner == Color::white) {
        ++agent1_wins;
      }
    }
    else {
      // Agent 1 plays black
      if (winner == Color::black) {
        ++agent1_wins;
        ++agent1_wins_as_black;
      } else if (winner == Color::both) {
        ++agent1_draws_as_black;
      }
    }

    auto agent1_losses = game_num + 1 - agent1_wins - draws;
    auto total_duration = cumulative_timer.elapsed();
    auto games_per_sec = (game_num + 1) / total_duration;
    auto remaining_sec = (num_games - game_num - 1) / games_per_sec;
    std::cout << game_num + 1 << "/" << num_games << "g";
    std::cout << ", " << agent1_wins << "/" << draws << "/" << agent1_losses << " (w/d/l):";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "  " << 100.0 * agent1_wins / (game_num + 1) << "% W";
    std::cout << ", " << 100.0 * draws / (game_num + 1) << "% D";
    std::cout << ", " << 100.0 * agent1_wins_as_black / agent1_num_black_games << "% BW";
    std::cout << ", " << 100.0 * agent1_draws_as_black / agent1_num_black_games << "% BD";
    std::cout << ";  " << total_num_moves / (game_num + 1) << " mpg";
    std::cout << std::defaultfloat << std::setprecision(4);
    std::cout << ", " << total_num_moves / total_duration << " mps";
    std::cout << "  [" << format_seconds(total_duration) << " < " << format_seconds(remaining_sec) << "]" << std::endl;

  }

}
