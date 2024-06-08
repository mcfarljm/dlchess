#ifndef AGENT_ZERO_H
#define AGENT_ZERO_H

#include <memory>
#include <optional>
#include <unordered_map>
#include <algorithm>

#include "encoder.h"
#include "experience.h"
#include "inference.h"
#include "../agent_base.h"
#include "../utils.h"
#include "../time_management.h"

namespace zero {

  using namespace game_moves;

  float value_to_centipawns(float val);

  class Branch {
  public:
    float prior;
    int visit_count = 0;
    float total_value = 0.0;

  public:
    Branch(float prior) : prior(prior) {}

    float expected_value() const {
      if (visit_count == 0)
        return -1.0;
      return total_value / visit_count;
    }

    int value_in_centipawns() const {
      return static_cast<int>(value_to_centipawns(expected_value()));
    }
  };


  class ZeroNode {

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
             std::optional<Move> last_move);

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

  enum class GameMode {
    uci,
    none,
  };

  struct SearchInfo {
    int num_rounds = 800;
    // Specify number of initial moves for which move selection is done randomly
    // baesd on visit count proportion.  Beyond this threshold, moves are
    // selected greedily.
    int num_randomized_moves = 0;
    bool add_noise = true;
    float policy_softmax_temp = 1.0;

    float cpuct = 2.0;
    float cpuct_base = 25000.0;
    float cpuct_factor = 0.0;

    bool disable_underpromotion = true;
    int debug = 0;

    GameMode game_mode = GameMode::none;

    float compute_cpuct(int N) const;

    std::shared_ptr<TimeManager> time_manager;
    utils::Timer timer;
    bool have_time_limit = false;
    float time_limit_ms;

    /// Set search time and start counting.
    void set_search_time(std::optional<int> move_time_ms,
                         std::optional<int> time_left_ms,
                         std::optional<int> inc_ms);

  private:
    /// Assign the time limit and start the search timer.
    void start_search_timer(float duration_ms) {
      time_limit_ms = duration_ms;
      have_time_limit = true;
      // std::cout << "Starting search timer for " << time_limit_ms << " ms\n";
      timer.reset();
    }
  };

  class ZeroAgent : public Agent {

    // Concentration parameter for dirichlet noise:
    constexpr static double DIRICHLET_CONCENTRATION = 0.03;
    constexpr static float DIRICHLET_WEIGHT = 0.25;

    std::shared_ptr<InferenceModel> model;

    std::shared_ptr<Encoder> encoder;

    std::shared_ptr<ExperienceCollector> collector;

  public:
    SearchInfo info;

  public:
    ZeroAgent(std::shared_ptr<InferenceModel> model,
              std::shared_ptr<Encoder> encoder,
              SearchInfo info = SearchInfo()) :
      model(std::move(model)), encoder(encoder), info(info) {}

    Move select_move(const board::Board&);

    void set_search_time(std::optional<int> move_time_ms,
                         std::optional<int> time_left_ms,
                         std::optional<int> inc_ms) {
      info.set_search_time(move_time_ms, time_left_ms, inc_ms);
    }

    void set_collector(std::shared_ptr<ExperienceCollector> c) {
      collector = c;
    }

  private:
    std::shared_ptr<ZeroNode> create_node(const board::Board& b,
                                          std::optional<Move> move = std::nullopt,
                                          std::weak_ptr<ZeroNode> parent = std::weak_ptr<ZeroNode>());
    void add_noise_to_priors(std::unordered_map<Move, float, MoveHash>& priors) const;
    Move select_branch(const ZeroNode& node) const;
    void debug_select_branch(const ZeroNode& node, int) const;
  };

};


#endif // AGENT_ZERO_H
