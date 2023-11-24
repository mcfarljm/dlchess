#include <algorithm>

#include "agent_zero.h"
#include "../myrand.h"


namespace zero {

  ZeroNode::ZeroNode(const board::Board& game_board, float value,
                     std::unordered_map<Move, float, MoveHash> priors,
                     std::weak_ptr<ZeroNode> parent,
                     std::optional<Move> last_move,
                     bool add_noise) :
    game_board(game_board), value(value), parent(parent), last_move(last_move),
    terminal(game_board.is_over()) {

    for (const auto &[move, p] : priors) {
      branches.emplace(move, p);
    }

    assert((! branches.empty()) || terminal);

    if (add_noise && ! branches.empty()) {
      // std::cout << "Orig prior: ";
      // for (const auto &[move, p] : priors)
      //   std::cout << move << " " << p << ", ";
      // std::cout << std::endl;

      // Sample noise on legal moves:
      // Adjust concentration based on number of legal moves, following Katago
      // paper.
      double alpha = DIRICHLET_CONCENTRATION * 19.0 * 19.0 / branches.size();
      auto dirichlet_dist = DirichletDistribution(branches.size(), alpha);
      std::vector<double> noise = dirichlet_dist.sample();
      // std::cout << "Noise: " << noise << std::endl;

      size_t idx = 0;
      for (auto &[move, prior]: priors) {
        prior = (1.0 - DIRICHLET_WEIGHT) * prior +
          DIRICHLET_WEIGHT * noise[idx];
        ++idx;
      }

      // std::cout << "Noised prior: ";
      // for (const auto &[move, p] : priors)
      //   std::cout << move << " " << p << ", ";
      // std::cout << std::endl;
    }

    if (terminal) {
      // Override the model's value estimate with actual result
      auto winner = game_board.winner().value();
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
    ++total_visit_count;
    auto it = branches.find(move);
    assert(it != branches.end());
    (it->second.visit_count)++;
    (it->second.total_value) += value;
  }


  float ZeroNode::expected_value(Move m) const {
    auto branch = branches.find(m)->second;
    if (branch.visit_count == 0)
      return 0.0;
    return branch.total_value / branch.visit_count;
  }


  Move ZeroAgent::select_move(const board::Board& game_board) {
    // std::cerr << "In select move, prior move count: " << game_board.total_moves << std::endl;
    auto root = create_node(game_board);

    for (auto round_number=0; round_number < num_rounds; ++round_number) {
      // std::cout << "Round: " << round_number << std::endl;
      auto node = root;
      // debug_select_branch(*node, round_number);
      auto next_move = select_branch(*node);
      // std::cout << "Selected root move: " << next_move << std::endl;
      // for (auto it = node->children.find(next_move); it != node->children.end();) {
      for (std::unordered_map<Move, std::shared_ptr<ZeroNode>, MoveHash>::const_iterator it;
           it = node->children.find(next_move), it != node->children.end();) {
        node = it->second;
        if (node->terminal)
          break;
        next_move = select_branch(*node);
      }

      float value;
      std::optional<Move> move;
      if (! node->terminal) {
        auto new_board = node->game_board;
        auto legal = new_board.make_move(next_move);
        assert(legal);
        auto child_node = create_node(std::move(new_board), next_move, node);
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
          node->record_visit(move.value(), value);
        move = node->last_move; // Will be null at root node
        node = node->parent.lock();
        value = -1 * value;
      }
    }

    if (collector) {
      auto root_state_tensor = encoder->encode(game_board);
      auto visit_counts = torch::zeros({PRIOR_SHAPE[0], PRIOR_SHAPE[1], PRIOR_SHAPE[2]});
      auto get_visit_count = [&](Move mv) {
        auto it = root->branches.find(mv);
        if (it != root->branches.end())
          return it->second.visit_count;
        else
          return 0;
      };
      auto move_coord_map = decode_legal_moves(game_board);
      for (const auto &[mv, coords] : move_coord_map) {
        visit_counts.index_put_({coords[0], coords[1], coords[2]},
                                static_cast<float>(get_visit_count(mv)));
      }
      collector->record_decision(root_state_tensor, visit_counts);
    }

    if (game_board.total_moves >= num_randomized_moves) {
      // Select the move with the highest visit count
      auto max_it = std::max_element(root->branches.begin(), root->branches.end(),
                                     [&root] (const auto& p1, const auto& p2) {
                                       return root->visit_count(p1.first) < root->visit_count(p2.first);
                                     });

      // for (const auto& [m, b] : root->branches)
      //   std::cerr << "visits: " << m << " " << b.visit_count << std::endl;
      // std::cout << "info string move " << game_board.total_moves/2 + 1 << ": E[V] = " << max_it->second.total_value / max_it->second.visit_count << ", visits = " << max_it->second.visit_count << std::endl;
      return max_it->first;
    }
    else {
      // Select move randomly in proportion to visit counts
      std::vector<Move> moves;
      std::vector<int> visit_counts;
      for (const auto& [move, branch] : root->branches) {
        moves.push_back(move);
        visit_counts.push_back(root->visit_count(move));
      }
      std::discrete_distribution<> dist(visit_counts.begin(), visit_counts.end());
      return moves[dist(rng)];
    }
  }



  std::shared_ptr<ZeroNode> ZeroAgent::create_node(const board::Board& game_board,
                                                   std::optional<Move> move,
                                                   std::weak_ptr<ZeroNode> parent) {

    // Note: also want to place this prior to loading jit model as well
    c10::InferenceMode guard;

    auto state_tensor = encoder->encode(game_board);
    state_tensor.unsqueeze_(0);

    std::vector<torch::jit::IValue> input({state_tensor});
    auto output = model.forward(input);
    auto priors = output.toTuple()->elements()[0].toTensor(); // Shape: (1, 8, 8, 73)
    auto values = output.toTuple()->elements()[1].toTensor();

    // std::cout << "priors from nn: " << priors << std::endl;

    values.squeeze_();
    priors.squeeze_();
    // std::cout << "priors size: " << at::numel(priors) << std::endl;
    // std::cout << "prior shape: " << priors.sizes() << std::endl;
    auto value = values.item().toFloat();

    auto move_coord_map = decode_legal_moves(game_board);
    std::unordered_map<Move, float, MoveHash> move_priors;
    for (const auto &[mv, coords] : move_coord_map) {
      // std::cout << "move prior coords: " << mv << ": " << coords << std::endl;
      move_priors.emplace(mv, priors.index({coords[0], coords[1], coords[2]}).item().toFloat());
    }

    bool _add_noise = add_noise && (! parent.lock());
    auto new_node = std::make_shared<ZeroNode>(game_board, value,
                                               std::move(move_priors),
                                               parent,
                                               move,
                                               _add_noise);
    auto parent_shared = parent.lock();
    if (parent_shared) {
      assert(move);
      parent_shared->add_child(move.value(), new_node);
    }
    return new_node;
  }



  Move ZeroAgent::select_branch(const ZeroNode& node) const {
    auto score_branch = [&] (Move move) {
      auto q = node.expected_value(move);
      auto p = node.prior(move);
      auto n = node.visit_count(move);
      return q + c_uct * p * sqrt(node.total_visit_count) / (n + 1);
    };
    auto max_it = std::max_element(node.branches.begin(), node.branches.end(),
                                   [score_branch] (const auto& p1, const auto& p2) {
                                     return score_branch(p1.first) < score_branch(p2.first);
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
    std::cout << "  prior, EV, n: " << node.prior(mv) << " " << node.expected_value(mv) << " " << node.visit_count(mv) << std::endl;
    std::cout << "  U: " << c_uct * node.prior(mv) * sqrt(node.total_visit_count) / (node.visit_count(mv) + 1) << std::endl;
  }

};
