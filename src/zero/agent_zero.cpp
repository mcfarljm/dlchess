#include <algorithm>
#include <cmath>

#include "agent_zero.h"
#include "../myrand.h"
#include "../utils.h"


using chess::Board;
using chess::Move;
using chess::MoveHash;
using chess::Color;


namespace zero {

  float SearchInfo::compute_cpuct(int N) const {
    return cpuct + (static_cast<bool>(cpuct_factor) ?
                    cpuct_factor * std::log((static_cast<float>(N) + cpuct_base) / cpuct_base) : 0.0f);
  }

  /// Set search time and start count.
  ///
  /// move_time_ms: Fixed move time (ms).  This overrides calculation based on remaining
  /// time and increment.
  void SearchInfo::set_search_time(std::optional<int> move_time_ms,
                                   std::optional<int> time_left_ms,
                                   std::optional<int> inc_ms,
                                   const chess::Board& b) {
    if (!time_manager)
      return;

    float duration_ms;

    if (move_time_ms)
      duration_ms = static_cast<float>(move_time_ms.value());
    else if (time_left_ms) {
      duration_ms = time_manager->budget_ms(*time_left_ms, inc_ms, b);
    }
    else {
      // Don't have any time settings, so just make sure the time limit is not set
      have_time_limit = false;
      return;
    }

    start_search_timer(duration_ms);
  }

  /// Get FPU at node.
  float SearchInfo::get_fpu(const ZeroNode& node) const {
    if (fpu_absolute)
      return fpu_value;

    // For reduction strategy, use node expected value, less the reduction amount
    // multiplied by the cumulative policy of visited nodes.  See LC0, GetFpu function
    // in search.cc.

    // Below calculation using branches is for verification.
    // float base_value = 0.0;
    // int branch_visit_count = 0;
    // for (const auto& [m, b] : node.branches) {
    //   base_value += b.total_value;
      // branch_visit_count += b.visit_count;
    // }
    // Note that node visit count is one greater than sum of its branch visit counts.
    // std::cout << "visits: " << branch_visit_count << " " << node.total_visit_count << std::endl;
    // assert(branch_visit_count + 1 == node.total_visit_count);
    // if (node.total_visit_count > 1)
    //   base_value /= (node.total_visit_count - 1);
    // std::cout << "EV: " << base_value << " " << node.expected_value_ << std::endl;
    return node.expected_value_ - fpu_value * std::sqrt(node.get_visited_policy());
  }



  float value_to_centipawns(float value) {
    return 111.714640912 * std::tan(1.5620688421 * value);
  }

  ZeroNode::ZeroNode(const chess::Board& game_board, float value,
                     const std::unordered_map<Move, float, MoveHash>& priors,
                     std::weak_ptr<ZeroNode> parent,
                     std::optional<Move> last_move) :
    game_board(game_board), value(value), parent(std::move(parent)), last_move(last_move),
    terminal(game_board.is_over()) {

    for (const auto &[move, p] : priors) {
      branches.emplace(move, p);
    }

    assert((! branches.empty()) || terminal);

    if (terminal) {
      // Override the model's value estimate with actual result
      auto winner = game_board.winner().value(); // NOLINT
      if (winner == game_board.side)
        // This is not possible, but we include this case for clarity
        ZeroNode::value = 1.0;
      else if (winner == Color::both)
        ZeroNode::value = 0.0;
      else
        ZeroNode::value = -1.0;
    }
  }

  void ZeroNode::record_visit(Move move, float value) {
    // Running average of node expected value is based on:
    // M_{k} = M_{k-1} + (x_k - M_{k-1}) / k
    // Note that k is number of child visits, which is (total_visit_count - 1)
    expected_value_ += (value - expected_value_) / static_cast<float>(total_visit_count);
    ++total_visit_count;

    auto it = branches.find(move);
    assert(it != branches.end());
    (it->second.visit_count)++;
    (it->second.total_value) += value;
  }

  float ZeroNode::expected_value(Move m, float fpu) const {
    auto branch = branches.find(m)->second;
    return branch.expected_value(fpu);
  }

  float ZeroNode::get_visited_policy() const {
    float sum = 0.0;
    for (const auto& [m, b] : branches)
      if (b.visit_count) sum += b.prior;
    return sum;
  }


  Move ZeroAgent::select_move(const chess::Board& game_board) {
    const utils::Timer timer; // Could be moved into SearchInfo to expand access.

    // std::cerr << "In select move, prior move count: " << game_board.total_moves << std::endl;

    // Note on tree reuse: there is code in the git history with a simple implementation
    // of tree reuse at depth 0 (running select_move multiple times on same position) or
    // depth 1.  It was intended initially for self play (occurs at depth 1 if same
    // agent is used for both sides).  But then I discovered that LC0 disables tree-use
    // for selfplay.  So decided to remove it to simplify the code.
    auto root = create_node(game_board);

    int max_depth = 0;
    long long cumulative_depth = 0;
    int round_number = 0;
    num_cache_hits_ = 0;
    for (;;) {
      // std::cout << "Round: " << round_number << std::endl;
      int depth = 0;
      auto node = root;
      // debug_select_branch(*node, round_number);
      auto next_move = select_branch(*node);
      ++depth;
      // std::cout << "Selected root move: " << next_move << std::endl;
      // for (auto it = node->children.find(next_move); it != node->children.end();) {
      for (std::unordered_map<Move, std::shared_ptr<ZeroNode>, MoveHash>::const_iterator it;
           it = node->children.find(next_move), it != node->children.end();) {
        node = it->second;
        if (node->terminal)
          break;
        next_move = select_branch(*node);
        ++depth;
      }
      max_depth = std::max(max_depth, depth);
      cumulative_depth += depth;

      float value;
      std::optional<Move> move;
      if (! node->terminal) {
        auto new_board = node->game_board;
        auto legal = new_board.make_move(next_move);
        assert(legal);
        auto child_node = create_node(new_board, next_move, node);
        value = -1 * child_node->value;
        move = next_move;
      }
      else {
        value = node->value;
      }

      while (node) {
        if (node->terminal)
          (node->total_visit_count)++;
        else
          node->record_visit(move.value(), value); // NOLINT(bugprone-unchecked-optional-access)
        move = node->last_move; // Will be null at root node
        node = node->parent.lock();
        value = -1 * value;
      }

      ++round_number;
      if (info.have_time_limit) {
        if (info.timer.elapsed() * 1000 > info.time_limit_ms)
          break;
      } else if ((info.num_visits > 0 && root->get_children_visits() >= info.num_visits) ||
                 (info.num_rounds > 0 && round_number >= info.num_rounds))
        break;
    }

    if (collector) {
      auto root_state_tensor = encoder_->encode(game_board);
      const std::vector<int64_t> visit_counts_shape {1, PRIOR_SHAPE[0], PRIOR_SHAPE[1], PRIOR_SHAPE[2]};
      Tensor<float> visit_counts(visit_counts_shape);
      auto get_visit_count = [&](Move mv) {
        auto it = root->branches.find(mv);
        if (it != root->branches.end())
          return it->second.visit_count;
        else
          return 0;
      };
      auto move_coord_map = encoder_->decode_legal_moves(game_board);
      for (const auto &[mv, coords] : move_coord_map) {
        visit_counts.at({0, coords[0], coords[1], coords[2]}) =
          static_cast<float>(get_visit_count(mv));
      }
      collector->record_decision(std::move(root_state_tensor), std::move(visit_counts), game_board.side);
    }

    auto best_move = [&](){
      if (game_board.total_moves >= info.num_randomized_moves) {
        // Select the move with the highest visit count
        auto max_it = std::max_element(root->branches.begin(), root->branches.end(),
                                       [](const auto& p1, const auto& p2) {
                                         return p1.second.visit_count < p2.second.visit_count;
                                       });
        // // Depth-2 debugging:
        // for (const auto& [m, b] : root->children.at(max_it->first)->branches)
        //   std::cout << "info string visits:   " << m << " " << b.visit_count << " " << b.prior << " " << b.expected_value() << std::endl;

        // std::cout << "info string move " << game_board.total_moves/2 + 1 << ": E[V] = " << max_it->second.total_value / max_it->second.visit_count << ", visits = " << max_it->second.visit_count << std::endl;
        return max_it->first;
      }
      else {
        // Select move randomly in proportion to visit counts
        std::vector<Move> moves;
        std::vector<int> visit_counts;
        for (const auto& [move, branch] : root->branches) {
          moves.push_back(move);
          visit_counts.push_back(branch.visit_count);
        }
        std::discrete_distribution<> dist(visit_counts.begin(), visit_counts.end());
        return moves[dist(rng)];
      }
    }();

    if (info.game_mode == GameMode::uci) {
      auto node_count = root->get_children_visits();
      std::cout << "info";
      std::cout << " depth " << static_cast<int>(cumulative_depth / round_number);
      std::cout << " seldepth " << max_depth;
      std::cout << " time " << static_cast<int>(timer.elapsed() * 1000);
      std::cout << " nodes " << node_count;
      std::cout << " score cp " << root->branches.at(best_move).value_in_centipawns(0.0);
      std::cout << " nps " << static_cast<int>(node_count / timer.elapsed());
      std::cout << " pv " << best_move;
      std::cout << std::endl;
    }

    if (info.debug >= 2) {
      std::cout << "info string cache hits: " << num_cache_hits_ << std::endl;;
      std::cout << "info string cache size: " << model_->cache_size() << std::endl;
    }

    if (info.debug > 0) {
      root->output_move_stats(info.get_fpu(*root), round_number);
    }

    return best_move;
  }

  void ZeroNode::output_move_stats(float fpu, int playouts) const {
    // Sort the moves in descending order.
    std::vector<std::pair<Move, Branch>> pairs(branches.begin(), branches.end());
    std::sort(pairs.begin(), pairs.end(), [](auto p1, auto p2) {
      return p1.second.visit_count < p2.second.visit_count; });
    for (const auto& [m, b] : pairs) {
      std::cout << "info string " << m << " N: " << b.visit_count;
      std::cout << " (P: " << b.prior * 100 << "%)";
      std::cout << " (Q: " << b.expected_value(fpu) << ")";
      auto child_it = children.find(m);
      if (child_it != children.end())
        std::cout << " (V: " << -child_it->second->value << ")";
      else
        std::cout << " (V:  -.----)";
      std::cout << "\n";
    }
    std::cout << "info string node (" << branches.size() << ")";
    std::cout << " N: " << playouts;
    std::cout << " (P: " << get_visited_policy() * 100 << "%)";
    std::cout << "\n";
  }


  /// Add Dirichlet noise to priors, modifying priors in place.
  ///
  /// This assumes that priors map is defined only for legal moves.
  void ZeroAgent::add_noise_to_priors(std::unordered_map<Move, float, MoveHash>& priors) const {
    if (priors.empty())
      return;

    // std::cout << "Orig prior: ";
    // for (const auto &[move, p] : priors)
    //   std::cout << move << " " << p << ", ";
    // std::cout << std::endl;

    // Sample noise on legal moves:
    // Adjust concentration based on number of legal moves, following Katago
    // paper.
    const double alpha = DIRICHLET_CONCENTRATION * 19.0 * 19.0 / static_cast<double>(priors.size());
    auto dirichlet_dist = DirichletDistribution(static_cast<int>(priors.size()), alpha);
    std::vector<double> noise = dirichlet_dist.sample();
    // std::cout << "Noise: " << noise << std::endl;

    size_t idx = 0;
    for (auto &[move, prior]: priors) {
      prior = (1.0 - DIRICHLET_WEIGHT) * prior + DIRICHLET_WEIGHT * noise[idx];
      // Force a flat prior for testing:
      // prior = 1.0 / priors.size();
      ++idx;
    }

    // std::cout << "Noised prior: ";
    // for (const auto &[move, p] : priors)
    //   std::cout << move << " " << p << ", ";
    // std::cout << std::endl;
  }


  std::shared_ptr<ZeroNode> ZeroAgent::create_node(const chess::Board& game_board,
                                                   std::optional<Move> move,
                                                   const std::weak_ptr<ZeroNode>& parent) {
    bool cache_hit = false;
    auto& output = model_->operator()(game_board, cache_hit);
    num_cache_hits_ += cache_hit;

    // Set up local pointer to move_priors.  If not adding noise, then we can just point
    // to cache reference, otherwise we need to make a local copy and point to that.
    const bool adding_noise = info.add_noise && !parent.lock() && !output.move_priors.empty();
    priors_type move_priors;  // Only needed if copying.
    const priors_type* move_priors_ptr = adding_noise ? &move_priors : &output.move_priors;

    if (adding_noise) {
      move_priors = output.move_priors; // Create copy
      add_noise_to_priors(move_priors);
    }

    auto new_node = std::make_shared<ZeroNode>(game_board, output.value,
                                               *move_priors_ptr,
                                               parent,
                                               move);
    auto parent_shared = parent.lock();
    if (parent_shared) {
      assert(move);
      parent_shared->add_child(move.value(), new_node);
    }
    return new_node;
  }

  Move ZeroAgent::select_branch(const ZeroNode& node) const {
    auto fpu = info.get_fpu(node);
    auto score_branch = [&] (Branch branch) {
      auto q = branch.expected_value(fpu);
      auto p = branch.prior;
      auto n = branch.visit_count;
      return q + info.compute_cpuct(node.total_visit_count) * p * sqrt(node.total_visit_count) / (n + 1);
    };
    auto max_it = std::max_element(node.branches.begin(), node.branches.end(),
                                   [score_branch] (const auto& p1, const auto& p2) {
                                     return score_branch(p1.second) < score_branch(p2.second);
                                   });
    assert(max_it != node.branches.end());
    return max_it->first;
  }

  void ZeroAgent::debug_select_branch(const ZeroNode& node, int round_number) const {
    // Call this at root node.  Print details about branch scoring.  Figure out
    // why we're not exploring moves with a flat prior.
    auto mv = select_branch(node);
    std::cout << "Select branch root, round " << round_number << ", " << node.branches.size() << " moves: " << mv << std::endl;
    std::cout << "  total visit count: " << node.total_visit_count << std::endl;
    // for (const auto& [move, prior] : node.branches) {

    // }
    auto fpu = info.get_fpu(node);
    std::cout << "  prior, EV, n: " << node.prior(mv) << " " << node.expected_value(mv, fpu) << " " << node.visit_count(mv) << std::endl;
    std::cout << "  c_puct: " << info.compute_cpuct(node.total_visit_count) << std::endl;
    std::cout << "  U: " << info.compute_cpuct(node.total_visit_count) * node.prior(mv) * sqrt(node.total_visit_count) / (node.visit_count(mv) + 1) << std::endl;
  }

};
