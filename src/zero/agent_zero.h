#ifndef AGENT_ZERO_H
#define AGENT_ZERO_H

#include <memory>
#include <optional>
#include <unordered_map>
#include <algorithm>
#include <atomic>

#include "encoder.h"
#include "experience.h"
#include "cached_inference.h"
#include "../agent_base.h"
#include "../utils.h"
#include "../time_management.h"

namespace zero {

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
      return total_value / static_cast<float>(visit_count);
    }

    int value_in_centipawns(float fpu) const {
      return static_cast<int>(value_to_centipawns(expected_value(fpu)));
    }
  };


  class ZeroNode {

  public:
    chess::Board game_board;

    std::weak_ptr<ZeroNode> parent;
    std::optional<chess::Move> last_move;
    std::unordered_map<chess::Move, std::shared_ptr<ZeroNode>, chess::MoveHash> children;
    std::unordered_map<chess::Move, Branch, chess::MoveHash> branches;
    // Value from neural net, or true terminal value.
    float value;
    int total_visit_count = 1;
    // Running average of expected value of child branches.
    float expected_value_ = 0.0;
    bool terminal;

    ZeroNode(const chess::Board& game_board, float value,
             const std::unordered_map<chess::Move, float, chess::MoveHash>& priors,
             std::weak_ptr<ZeroNode> parent,
             std::optional<chess::Move> last_move);

    void add_child(chess::Move move, const std::shared_ptr<ZeroNode>& child) {
      children.emplace(move, child);
    }

    void record_visit(chess::Move m, float val);

    float get_fpu() const;
    float expected_value(chess::Move m, float fpu) const;
    float get_visited_policy() const;

    float prior(chess::Move m) const {
      return branches.find(m)->second.prior;
    }

    int visit_count(chess::Move m) const {
      auto it = branches.find(m);
      if (it != branches.end())
        return it->second.visit_count;
      return 0;
    }

    int get_children_visits() const {
      assert(total_visit_count > 0);
      return total_visit_count - 1;
    }

    chess::Move get_best_move() const;
    void output_move_stats(float fpu, int playouts) const;
    void output_uci_info(int cumulative_depth, int max_depth, double time_seconds,
                         std::optional<chess::Move> best_move=std::nullopt) const;

  };

  enum class GameMode {
    uci,
    none,
  };

  struct SearchInfo {
    // Limit on number of playouts.  Negative number means no limit.
    int num_rounds = 800;
    // If set, prevent the UCI "go infinite" command from overriding num_rounds.  This
    // makes it possible to fix the number of rounds when using a chess GUI in infinite
    // time control (i.e., for tournaments with cutechess).
    bool sticky_num_rounds = false;
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
    float fpu_value = 0.33;

    bool disable_underpromotion = true;
    int debug = 0;
    bool verbose_move_stats = false;
    bool live_move_stats = false;

    GameMode game_mode = GameMode::none;

    float compute_cpuct(int N) const;
    float get_fpu(const ZeroNode& node) const;

    std::shared_ptr<TimeManager> time_manager;
    utils::Timer timer;
    bool have_time_limit = false;
    float time_limit_ms;

    int nn_cache_size = 100000;

    std::shared_ptr<std::atomic<bool>> stop_flag_ptr_;

    /// Set search time and start counting.
    void set_search_time(std::optional<int> move_time_ms,
                         std::optional<int> time_left_ms,
                         std::optional<int> inc_ms,
                         const chess::Board& b);

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

    std::shared_ptr<CachedInferenceModel> model_;
    std::shared_ptr<Encoder> encoder_;

    std::shared_ptr<ExperienceCollector> collector;

    int num_cache_hits_ = 0;

  public:
    SearchInfo info;

  public:
    ZeroAgent(const std::shared_ptr<InferenceModel>& model,
              std::shared_ptr<Encoder> encoder,
              SearchInfo info = SearchInfo()) :
      encoder_(encoder), info(info) {
      model_ = std::make_shared<CachedInferenceModel>(model, encoder, info.nn_cache_size, info.policy_softmax_temp, info.disable_underpromotion);
    }

    chess::Move select_move(const chess::Board&) override;

    void set_search_time(std::optional<int> move_time_ms,
                         std::optional<int> time_left_ms,
                         std::optional<int> inc_ms,
                         const chess::Board& b) override {
      info.set_search_time(move_time_ms, time_left_ms, inc_ms, b);
    }

    void set_search_nodes(std::optional<int> nodes) override {
      if (info.sticky_num_rounds && !nodes)
        return;
      info.num_rounds = nodes.value_or(-1);
    }

    void set_collector(std::shared_ptr<ExperienceCollector> c) {
      collector = std::move(c);
    }

  private:
    std::shared_ptr<ZeroNode> create_node(const chess::Board& b,
                                          std::optional<chess::Move> move = std::nullopt,
                                          const std::weak_ptr<ZeroNode>& parent = std::weak_ptr<ZeroNode>());
    void add_noise_to_priors(std::unordered_map<chess::Move, float, chess::MoveHash>& priors) const;
    chess::Move select_branch(const ZeroNode& node) const;
    void debug_select_branch(const ZeroNode& node, int) const;
  };

};


#endif // AGENT_ZERO_H
