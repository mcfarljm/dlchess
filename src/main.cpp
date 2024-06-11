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
    ("policy-softmax-temp", "Policy softmax temperature", cxxopts::value<float>()->default_value("1.359"))
    ("cpuct", "c_puct constant for UCT search", cxxopts::value<float>()->default_value("1.745"))
    ("cpuct-base", "c_puct base for growth", cxxopts::value<float>()->default_value("38739.0"))
    ("cpuct-factor", "c_puct multiplier for growth", cxxopts::value<float>()->default_value("3.894"))
    ("fpu-value", "First play urgency value", cxxopts::value<float>()->default_value("0.44"))
    ("fpu-absolute", "Use FPU absolute strategy")
    // Note: Disabling bool must be done via --opt=false
    ("time-manager", "Use time manager", cxxopts::value<bool>()->default_value("true"))
    ("t,num-threads", "Number of pytorch threads", cxxopts::value<int>())
    ("d,debug", "Debug level", cxxopts::value<int>()->default_value("0"))
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
  auto cpuct_base = args["cpuct-base"].as<float>();
  auto cpuct_factor = args["cpuct-factor"].as<float>();
  auto fpu_value = args["fpu-value"].as<float>();
  auto fpu_absolute = args["fpu-absolute"].as<bool>();
  auto time_manager = args["time-manager"].as<bool>();
  auto debug = args["debug"].as<int>();

  std::optional<int> num_threads_option;
  if (args.count("num-threads")) {
    std::cout << "setting " << args["num-threads"].as<int>() << " inference threads" << std::endl;
    num_threads_option = args["num-threads"].as<int>();
  }

  auto model = std::make_shared<zero::InferenceModel>(args["network"].as<std::string>().c_str(), num_threads_option);

  auto encoder = std::make_shared<zero::SimpleEncoder>();
  zero::SearchInfo info;
  info.num_rounds = num_rounds;
  info.num_randomized_moves = num_randomized_moves;
  info.add_noise = noise;
  info.policy_softmax_temp = policy_softmax_temp;
  info.cpuct = cpuct;
  info.cpuct_base = cpuct_base;
  info.cpuct_factor = cpuct_factor;
  info.fpu_value = fpu_value;
  info.fpu_absolute = fpu_absolute;
  info.debug = debug;
  if (time_manager) {
    // auto the_time_manager = std::make_shared<AlphaZeroTimeManager>();
    auto the_time_manager = std::make_shared<SimpleTimeManager>();
    info.time_manager = the_time_manager;
  }
  auto agent = std::make_unique<zero::ZeroAgent>(model, encoder, info);

  std::string input;
  std::cin >> input;

  if (input.rfind("uci", 0) == 0)
    uci::uci_loop(agent.get());
}
