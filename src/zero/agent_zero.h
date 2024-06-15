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

    float expected_value(float fpu) const {
      if (visit_count == 0) {
        // if (fpu != 0) std::cout << "FPU: " << fpu << "\n";
        return fpu;
      }
      return total_value / visit_count;
    }

    int value_in_centipawns(float fpu) const {
      return static_cast<int>(value_to_centipawns(expected_value(fpu)));
    }
  };


  class ZeroNode {

  public:
    board::Board game_board;

    std::weak_ptr<ZeroNode> parent;
    std::optional<Move> last_move;
    std::unordered_map<Move, std::shared_ptr<ZeroNode>, MoveHash> children;
    std::unordered_map<Move, Branch, MoveHash> branches;
    // Value from neural net, or true terminal value.
    float value;
    int total_visit_count = 1;
    // Running average of expected value of child branches.
    float expected_value_ = 0.0;
    bool terminal;

    ZeroNode(const board::Board& game_board, float value,
             std::unordered_map<Move, float, MoveHash> priors,
             std::weak_ptr<ZeroNode> parent,
             std::optional<Move> last_move);

    void add_child(Move move, std::shared_ptr<ZeroNode> child) {
      children.emplace(move, child);
    }

    void record_visit(Move m, float val);

    float get_fpu() const;
    float expected_value(Move m, float fpu) const;
    float get_visited_policy() const;

    float prior(Move m) const {
      return branches.find(m)->second.prior;
    }

    int visit_count(Move m) const {
      auto it = branches.find(m);
      if (it != branches.end())
        return it->second.visit_count;
      return 0;
    }

    int get_children_visits() const {
      assert(total_visit_count > 0);
      return total_visit_count - 1;
    }

  };

  enum class GameMode {
    uci,
    none,
  };

  struct SearchInfo {
    // Limit on number of playouts, regardless of tree re-use.  Negative number means no limit.
    int num_rounds = 800;
    // Limit on number of visits, which does include tree re-use.  Negative number means no limit.
    int num_visits = -1;
    // Specify number of initial moves for which move selection is done randomly
    // baesd on visit count proportion.  Beyond this threshold, moves are
    // selected greedily.
    int num_randomized_moves = 0;
    bool add_noise = true;
    float policy_softmax_temp = 1.0;

    float cpuct = 2.0;
    float cpuct_base = 25000.0;
    float cpuct_factor = 0.0;

    bool fpu_absolute = false;
    float fpu_value = 0.44;

    bool disable_underpromotion = true;
    int debug = 0;

    GameMode game_mode = GameMode::none;

    float compute_cpuct(int N) const;
    float get_fpu(const ZeroNode& node) const;

    std::shared_ptr<TimeManager> time_manager;
    utils::Timer timer;
    bool have_time_limit = false;
    float time_limit_ms;

    /// Set search time and start counting.
    void set_search_time(std::optional<int> move_time_ms,
                         std::optional<int> time_left_ms,
                         std::optional<int> inc_ms,
                         const board::Board& b);

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

    // Store pointer to tree root so that the tree can be re-used across moves.
    std::shared_ptr<ZeroNode> root_;

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
                         std::optional<int> inc_ms,
                         const board::Board& b) {
      info.set_search_time(move_time_ms, time_left_ms, inc_ms, b);
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
