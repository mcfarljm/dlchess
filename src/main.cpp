#include <string>
#include <iostream>

#include <cxxopts.hpp>

#include "io/uci.h"
#include "zero/agent_zero.h"

int main(int argc, char* argv[]) {

  cxxopts::Options options("dlchess", "Run dlchess engine");

  options.add_options()
    ("network", "Path to network file", cxxopts::value<std::string>())
    ("r,rounds", "Number of rounds", cxxopts::value<int>()->default_value("800"))
    ("num-random-moves", "Number of randomized moves", cxxopts::value<int>()->default_value("0"))
    ("noise", "Include Dirichlet noise")
    // ("v,verbosity", "Verbosity level", cxxopts::value<int>()->default_value("0"))
    ("policy-softmax-temp", "Policy softmax temperature", cxxopts::value<float>()->default_value("1.0"))
    ("cpuct", "c_puct constant for UCT search", cxxopts::value<float>()->default_value("1.5"))
    ("t,num-threads", "Number of pytorch threads", cxxopts::value<int>())
    ("h,help", "Print usage")
    ;

  options.parse_positional({"network"});

  options.positional_help("<network>");

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

  auto num_rounds = args["rounds"].as<int>();
  auto num_randomized_moves = args["num-random-moves"].as<int>();
  auto noise = args["noise"].as<bool>();
  // auto verbosity = args["verbosity"].as<int>();
  auto policy_softmax_temp = args["policy-softmax-temp"].as<float>();
  auto cpuct = args["cpuct"].as<float>();

  if (args.count("num-threads")) {
    // std::cout << "setting " << args["num-threads"].as<int>() << " pytorch threads" << std::endl;
    at::set_num_threads(args["num-threads"].as<int>());
  }

  c10::InferenceMode guard;
  torch::jit::script::Module model;
  try {
    model = torch::jit::load(args["network"].as<std::string>());
  }
  catch (const c10::Error& e) {
    std::cerr << "Error loading the model: " << argv[1] << std::endl;
    return -1;
  }

  auto encoder = std::make_shared<zero::SimpleEncoder>();
  zero::SearchInfo info;
  info.num_rounds = num_rounds;
  info.num_randomized_moves = num_randomized_moves;
  info.add_noise = noise;
  info.policy_softmax_temp = policy_softmax_temp;
  info.cpuct = cpuct;
  auto agent = std::make_unique<zero::ZeroAgent>(model, encoder, info);

  std::string input;
  std::cin >> input;

  if (input.rfind("uci", 0) == 0)
    uci::uci_loop(agent.get());
}
