# AlphaZero Search

## Algorithm

The AlphaZero search algorithm is a modified version of [Monte Carlo tree
search](https://en.wikipedia.org/wiki/Monte_Carlo_tree_search) that incorporates a
neural network.  The neural network expresses a mapping between an encoding of the game
state and two outputs: (a) a value that measures probability of win, and (b) a policy vector
that assigns probabilities to moves.  The key differences from standard Monte Carlo tree
search are:

1. In the "Simulation" ("playout") step, instead of simulating a game from the given
   position, the value of the neural network is used as the playout result.
2. In the selection step, the policy output from the network is used as a sort of
   "prior" in a modified version of the PUCT algorithm.  The idea is to encourage the
   selection of nodes with high policy, high value, and low visit count.

The algorithm is implemented as a tree structure made up of nodes, where each node
represents a particular game state.  Each node has a set of "branches" that represent
the legal moves that can be played in that position.  When a branch is selected for the
first time, this leads to a new game state and the creation of a new node.

The diagram below depicts a simple tree with four nodes.  The "Root" node corresponds to
the start of the game.  From this starting position, the search would begin by querying
the neural network using an encoding of this game state as input.  Based on the PUCT
formula, one of the possible moves would be selected.  Because no branches have been
visited yet, the first selection would be governed by the move that has the highest
policy from the neural network.  Suppose that this was the move **1.Na3**.  The
`Board::make_move` method (see [Chess Framework](chess-framework.md)) is used to create
a new game state, and that new game state is used to instantiate a new node in the tree.
When the new node is created, the neural network is evaluated using that state as input
to obtain a corresponding value and policy.  This completes the first "playout" in the
search.

``` mermaid
graph TB
  Root --> |Na3|m1[1.Na3]
  Root --> |e4|m2[1.e4]
  m2 --> |e5|m3[1.e4 e5]
```

The search would then continue with the second playout.  We return to the root node and
use the PUCT algorithm to select a move.  Now that the visit count for the **Na3** move
has changed, the PUCT algorithm may prioritize a different move; suppose that in the
second playout, the move selected by PUCT is **e4**.  Since this move has not yet been
visited, we again use `Board::make_move` to create a new game state, instantiate a new
node, and query the network for value and policy.  This completes the second playout,
and we return to the root.

For the third playout, we again use the PUCT
algorithm to select among the moves at the Root node.  Suppose that just like in the
second round, it selects the move **e4**.  Since this branch has already been visited,
we traverse to the **1.e4** node and apply the PUCT selection algorithm to select a move
at that node.  Suppose now that the algorithm selects the move **e5** (which is now a
move played by the player with the black pieces).  This branch has not yet been visited,
so it results in the creation of a new node **1.e4 e5** and a new neural network
evaluation.
   
Once the playouts are complete, the move is selected among the branches at root based on
their numbers of visit counts.  Typically, the branch with the largest visit count is
selected, but randomization can be introduced during self-play by selecting the move
randomly in proportion to the number of visit counts.

Some things to note about this process:

- For each playout in the search, the tree is traversed using branch selection until we
  either arrive at a node that has not yet been visited or we arrive at a terminal node.
  This means that not counting terminal nodes, every playout corresponds to one
  evaluation of the neural network.
- At the end of each playout, the value obtained from the neural network (or the
  terminal game value of 1, -1, or 0) is backpropagated to all parent branches (flipping
  sign at each step to maintain appropriate perspective).  In this way, each branch
  maintains a running average of values from all leaf nodes that traversed through it.
  In our example with three playouts, the **e4** branch from the root node has been
  visited twice, and its value is the average of the neural network value predictions
  from the **1.e4** and **1.e4 e5** nodes.

## Data Structures

The fundamental data structures used in the AlphaZero search method are the _node_ and
the _branch_.  The node records information about a particular location in the search
tree, which includes the game state, the corresponding neural network output, visit
count, and more.  The data members of the `ZeroNode` class are shown below.

```c++ title="ZeroNode data"
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
};
```

The `Branch` class contains information associated with a legal move that can be made
from the game state of a particular `Node`.  The `Branch` class has a simple set of data
members, as shown below.  Note that the `ZeroNode` class stores branches as part of a
mapping from `chess::Move` to `Branch` instances.  Here `prior` is the neural network
policy output corresponding to a particular branch.


```c++ title="Branch data"
class Branch {
public:
  float prior;
  int visit_count = 0;
  float total_value = 0.0;
};
```
