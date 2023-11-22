#include <string>
#include <iostream>

#include "io/uci.h"
#include "zero/agent_zero.h"

int main(int argc, char* argv[]) {

  constexpr int num_rounds = 800;

  if (argc < 2) {
    std::cerr << "Usage: dlchess <network>" << std::endl;
    return -1;
  }

  c10::InferenceMode guard;
  torch::jit::script::Module model;
  try {
    model = torch::jit::load(argv[1]);
  }
  catch (const c10::Error& e) {
    std::cerr << "Error loading the model: " << argv[1] << std::endl;
    return -1;
  }

  auto encoder = std::make_shared<zero::SimpleEncoder>();
  auto agent = std::make_unique<zero::ZeroAgent>(model, encoder, num_rounds, 0);

  std::string input;
  std::cin >> input;

  if (input.rfind("uci", 0) == 0)
    uci::uci_loop(agent.get());
}
