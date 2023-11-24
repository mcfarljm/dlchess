#ifndef AGENT_ZERO_H
#define AGENT_ZERO_H

#include <memory>
#include <optional>
#include <unordered_map>
#include <torch/script.h> // One-stop header.

#include "encoder.h"
#include "experience.h"
#include "../agent_base.h"

namespace zero {

  using namespace game_moves;

  class Branch {
  public:
    float prior;
    int visit_count = 0;
    float total_value = 0.0;

  public:
    Branch(float prior) : prior(prior) {}
  };


  class ZeroNode {

    // Concentration parameter for dirichlet noise:
    constexpr static double DIRICHLET_CONCENTRATION = 0.03;
    constexpr static float DIRICHLET_WEIGHT = 0.25;

  public:
    board::Board game_board;

    std::weak_ptr<ZeroNode> parent;
    std::optional<Move> last_move;
    std::unordered_map<Move, std::shared_ptr<ZeroNode>, MoveHash> children;
    std::unordered_map<Move, Branch, MoveHash> branches;
    float value;
    int total_visit_count = 1;
    bool terminal;

    ZeroNode(const board::Board& game_board, float value,
             std::unordered_map<Move, float, MoveHash> priors,
             std::weak_ptr<ZeroNode> parent,
             std::optional<Move> last_move,
             bool add_noise);

    void add_child(Move move, std::shared_ptr<ZeroNode> child) {
      children.emplace(move, child);
    }

    void record_visit(Move m, float val);

    float expected_value(Move m) const;

    float prior(Move m) const {
      return branches.find(m)->second.prior;
    }

    int visit_count(Move m) const {
      auto it = branches.find(m);
      if (it != branches.end())
        return it->second.visit_count;
      return 0;
    }
  };


  class ZeroAgent : public Agent {
    torch::jit::script::Module model;
    std::shared_ptr<Encoder> encoder;
    int num_rounds;
    float c_uct;

    std::shared_ptr<ExperienceCollector> collector;

    // Specify number of initial moves for which move selection is done randomly
    // baesd on visit count proportion.  Beyond this threshold, moves are
    // selected greedily.
    int num_randomized_moves;
    bool add_noise;

  public:
    ZeroAgent(torch::jit::script::Module model,
              std::shared_ptr<Encoder> encoder,
              int num_rounds,
              int num_randomized_moves = 0,
              bool add_noise = true,
              float c_uct = 1.5) :
      model(model), encoder(encoder), num_rounds(num_rounds), num_randomized_moves(num_randomized_moves), add_noise(add_noise), c_uct(c_uct) {}

    Move select_move(const board::Board&);

    void set_collector(std::shared_ptr<ExperienceCollector> c) {
      collector = c;
    }

  private:
    std::shared_ptr<ZeroNode> create_node(const board::Board& b,
                                          std::optional<Move> move = std::nullopt,
                                          std::weak_ptr<ZeroNode> parent = std::weak_ptr<ZeroNode>());
    Move select_branch(const ZeroNode& node) const;
    void debug_select_branch(const ZeroNode& node, int) const;
  };

};


#endif // AGENT_ZERO_H
