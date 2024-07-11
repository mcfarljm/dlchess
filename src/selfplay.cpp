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
#include "simulation.h"
#include "utils.h"

using namespace zero;
using namespace utils;


int main(int argc, const char* argv[]) {

  cxxopts::Options options("selfplay", "Run self play with zero agent");

  options.add_options()
    ("network", "Path to pytorch script file", cxxopts::value<std::string>())
    ("o,output-path", "Directory to store output", cxxopts::value<std::string>())
    ("l,label", "Label to use within experience directory", cxxopts::value<std::string>()->default_value(""))
    ("r,rounds", "Number of rounds (-1 no limit)", cxxopts::value<int>()->default_value("-1"))
    ("visits", "Number of visits (-1 no limit)", cxxopts::value<int>()->default_value("800"))
    ("g,num-games", "Number of games", cxxopts::value<int>()->default_value("1"))
    ("m,max-moves", "Maximum moves per game", cxxopts::value<int>()->default_value("10000"))
    // Careful, this only works with --noise=false, and "--noise false" will not
    // work and will not raise an error.
    ("noise", "Include Dirichlet noise", cxxopts::value<bool>()->default_value("true"))
    ("policy-softmax-temp", "Policy softmax temperature", cxxopts::value<float>()->default_value("1.0"))
    ("cpuct", "c_puct constant for UCT search", cxxopts::value<float>()->default_value("1.2"))
    ("e,save-every", "Interval at which to save experience", cxxopts::value<int>()->default_value("100"))
    // Todo: should be able to parse input shape from onnx model and determine this automatically.
    ("encoding-version", "Version of network input encoding", cxxopts::value<int>()->default_value("0"))
    ("v,verbosity", "Verbosity level", cxxopts::value<int>()->default_value("0"))
    ("d,debug", "Debug level", cxxopts::value<int>()->default_value("0"))
    ("t,num-threads", "Number of pytorch threads", cxxopts::value<int>())
    ("h,help", "Print usage")
    ;

  options.parse_positional({"network"});
  options.positional_help("<network_file>");

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

  if (! args.count("network")) {
    std::cout << options.help() << std::endl;
    exit(1);
  }

  std::string output_path;
  bool store_experience = false;

  auto num_rounds = args["rounds"].as<int>();
  auto num_visits = args["visits"].as<int>();
  auto num_games = args["num-games"].as<int>();
  auto max_moves = args["max-moves"].as<int>();
  auto save_interval = args["save-every"].as<int>();
  auto encoding_version = args["encoding-version"].as<int>();
  auto verbosity = args["verbosity"].as<int>();
  auto debug = args["debug"].as<int>();
  auto add_noise = args["noise"].as<bool>();
  auto policy_softmax_temp = args["policy-softmax-temp"].as<float>();
  auto cpuct = args["cpuct"].as<float>();
  if (num_rounds <= 0 && num_visits <= 0) {
    std::cerr << "Error, must specify number of rounds or visits" << std::endl;
    exit(1);
  }
  if (args.count("output-path")) {
    output_path = args["output-path"].as<std::string>();
    store_experience = true;
  }
  std::string experience_label(args["label"].as<std::string>());
  if (experience_label.size()) {
    if (! store_experience) {
      std::cerr << "Error, experience label passed without output path" << std::endl;
      exit(1);
    }
    experience_label.insert(0, "_");
  }

  if (store_experience) {
    if (std::filesystem::exists(output_path) && ! std::filesystem::is_directory(output_path)) {
      std::cerr << "output path exists and is not a directory: " + output_path << std::endl;
      exit(1);
    }
  }
    
  std::optional<int> num_threads_option;
  if (args.count("num-threads")) {
    std::cout << "setting " << args["num-threads"].as<int>() << " inference threads" << std::endl;
    num_threads_option = args["num-threads"].as<int>();
  }

  auto model = std::make_shared<InferenceModel>(args["network"].as<std::string>().c_str(), num_threads_option);

  std::cout << "Model loaded\n";

  zero::SearchInfo info;
  info.num_rounds = num_rounds;
  info.num_visits = num_visits;
  info.num_randomized_moves = 10000;
  info.add_noise = add_noise;
  info.policy_softmax_temp = policy_softmax_temp;
  info.cpuct = cpuct;
  info.fpu_value = 0.0;
  info.debug = debug;

  auto encoder = std::make_shared<SimpleEncoder>(encoding_version);

  auto collector = std::make_shared<ExperienceCollector>();

  auto agent = std::make_unique<ZeroAgent>(model, encoder, info);

  agent->set_collector(collector);

  int num_white_wins = 0;
  int num_black_wins = 0;
  int num_draws = 0;
  int save_counter = 0;
  int total_num_moves = 0;
  auto cumulative_timer = Timer();
  for (int game_num=0; game_num < num_games; ++game_num) {
    auto timer = Timer();
    auto [winner, num_moves] = simulate_game(agent.get(), agent.get(), verbosity, max_moves);
    auto duration = timer.elapsed();
    total_num_moves += num_moves;
    if (num_games <= 5) {
      std::cout << "Game: " << num_moves << " moves in " << duration;
      std:: cout << " s (" << num_moves / duration << " mv/s, " << duration / num_moves << " s/mv)" << std::endl;
    }

    if (winner == Color::white) ++num_white_wins;
    if (winner == Color::black) ++num_black_wins;
    if (winner == Color::both) ++num_draws;

    auto total_duration = cumulative_timer.elapsed();
    auto games_per_sec = (game_num + 1) / total_duration;
    auto remaining_sec = (num_games - game_num - 1) / games_per_sec;
    std::cout << game_num + 1 << "/" << num_games;
    std::cout << std::fixed << std::setprecision(1) << ": " << 100.0 * num_white_wins / (game_num + 1) << ", " << 100.0 * num_black_wins / (game_num + 1) << ", " << 100.0 * num_draws / (game_num + 1) << " (%WW,BW,D)";
    std::cout << ", " << total_num_moves / (game_num + 1) << " mpg";
    std::cout << std::defaultfloat << std::setprecision(4);
    std::cout << ", " << total_num_moves / total_duration << " mps";
    std::cout << "  [" << format_seconds(total_duration) << " < " << format_seconds(remaining_sec) << "]" << std::endl;

    auto white_reward = [=]() {
      if (winner == Color::white)
        return 1.0;
      else if (winner == Color::black)
        return -1.0;
      else
        return 0.0;
    }();

    collector->complete_episode(white_reward);

    if (store_experience && (game_num + 1) % save_interval == 0) {
      collector->serialize_binary(output_path, experience_label + "_" + std::to_string(save_counter));
      collector->reset();
      ++save_counter;
    }
  }

  std::cout << "Finished: " << total_num_moves << " moves at " << std::setprecision(2) << total_num_moves / cumulative_timer.elapsed() << " moves / second" << std::endl;

  if (store_experience) {
    collector->serialize_binary(output_path, experience_label);
  }
}
