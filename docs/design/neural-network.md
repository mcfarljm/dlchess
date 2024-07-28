# Neural Network

## Architecture

The neural network architecture follows that of the original AlphaZero work, as shown
below.  The architecture is made up of a "base/tower" of residual blocks, which is
shared by both the policy and value outputs.  Following the base, there is a "policy
head" and "value head".  The network is parameterized in terms of the number of blocks
in the base, and the number of filters.  The original AlphaZero work used something like
19 residual blocks and 256 filters.  See [Training Runs](../training-runs.md) for
information about the architecture settings that have been tried with `dlchess`.
Details of the implementation, which uses PyTorch, can be seen in the source for
[resid_net.py](https://github.com/mcfarljm/dlchess/blob/main/nn/resid_net.py).

``` mermaid
graph TB
  Input{Input} --> |22 x 8 x 8|b1[Residual Block 1]
  b1 --> |filters x 8 x 8|bn[Residual Block n]
  bn --> pc1[Convolution]
  pc1 --> |filters x 8 x 8|pc2[Convolution]
  pc2 --> |73x 8 x 8|policy{Policy}
  bn --> vc1[Convolution, 1x1]
  vc1 --> |1 x 8 x 8|vl1[Linear]
  vl1 --> |256|vl2[Linear]
  vl2 --> |1|tanh[Tanh]
  tanh --> Value{Value}
```

The above diagram shows the use of residual blocks to construct the base; earlier
versions of `dlchess` used simple convolution blocks instead, which results in a more
compact network.

## Input Encoding

The game state must be represented in a form that the neural network can process.  This
is done by _encoding_ the game state into a tensor, which can be fed into the network.
dlchess uses a simple encoding that is based on the original AlphaZero work, but without
the use of history.

The building block of the input encoding is the concept of a _plane_.  A plane is an 8
by 8 array that represents some particular feature of the game.  The elements of the
plane correspond to the squares on the chess board.  dlchess uses 22 input planes[^1]:

- 12 planes that encode piece occupation.  For each piece type, the corresponding plane
  is set to 1 where a piece exists and 0 otherwise.
- 2 planes that encode the repetition count (which is relevant for the three-fold
  repetition rule).  The first plane is set to all ones if the repetition count is one
  or greater, and zeros otherwise.  The second plane is set to all ones if the
  repetition count is two or greater, and zeros otherwise.
- 1 plane for color of side to move: set to all ones if black is to move, zeros
  otherwise.
- 4 planes for castling permissions, each set to all ones if the associated castling
  permission is available.
- 1 plane that encodes the no-progress count (relevant for the fifty-move rule).  This
  plane is filled with the numerical value of the number half-moves that count towards
  the fifty-move rule.
- 1 plane that encodes the en passant square, if any.
- 1 constant plane that is set to all ones, presumably to help with edge detection.

The input encoder is designed against the following interface, where `Tensor` is a simple user-defined template that wraps `std::vector` with shape and stride information.

```c++
class Encoder {
public:
  virtual Tensor<float> encode(const chess::Board&) const = 0;
};
```

[^1]: Earlier versions of dlchess used an encoding with 21 input planes, which did not include an en passant plane.

## Output Decoding

A convention is required for how to map the neural network policy output, which is a
tensor, to moves.  The challenge is that the space of legal moves in chess varies
depending on the game state, and the different pieces move in different ways, including
special moves such as castling, promotion, and en passant.  The idea is that the network
policy output encompasses all possible moves in any position, and then we can filter on
position-specific legal moves later.

dlchess uses the same approach as AlphaZero, where the space of possible moves is
represented by an 73x8x8 tensor, where only a subset of the elements correspond to
possible moves.  To work with this mapping, we define a function that takes a game state
as input and returns the indices of the legal moves, within the network output tensor.
The interface for this function is:

```c++
std::unordered_map<chess::Move, std::array<int,3>, chess::MoveHash>
decode_legal_moves(const chess::Board&);
```

The output of this function is a mapping from a `Move` to a length-3 array of indices,
which indexes into the 73x8x8 policy tensor.  The length of the mapping that the
function returns is the number of legal moves for the given game state.  For example,
suppose that there is a particular position with only two legal moves.  Calling
`decode_legal_moves` would return a map with two items: the keys define the `Move`
objects for the two moves, and the values are index arrays that tell us the
corresponding locations within the policy tensor.  For each move, we can use the indices
to look up and extract the corresponding network policy.

The details of how the function works are a little bit involved and can be seen by
reading the code in
[encoder.cpp](https://github.com/mcfarljm/dlchess/blob/main/src/zero/encoder.cpp).

## Inference Backend

The design of `dlchess` is such that the neural network architecture and training code
are written in Python using PyTorch, but the search is written in C++.  Thus, we need to
be able to run network inference from C++.  The original version of `dlchess` used
[TorchScript](https://pytorch.org/docs/stable/jit.html) to run inference from C++.
However, the current version uses [ONNX Runtime](https://onnxruntime.ai/), which
resulted in a roughly 2.5x total speedup in search throughput using CPUs.

The implementation of neural network inference using ONNX Runtime in `dlchess` can be
seen here:
[inference.h](https://github.com/mcfarljm/dlchess/blob/main/src/zero/inference.h).  This
largely follows the structure of the ONNX Runtime C++ tutorials.  The `InferenceModel`
class is instantiated from a path to the saved ONNX model file, and inference is wrapped
using the `operator()` method, which accepts an input tensor and returns two output
tensors.

## Caching

Other than switching from TorchScript to ONNX Runtime for inference, the biggest
performance improvement for search throughput (which governs how long it takes to
generate selfplay data) was the implementation of a cache for the neural network output.
The idea is that we keep a record of previously evaluated positions, so that we can
avoid running the same network inference again in the future.  There are a couple of
reasons that we might see the same network inputs occur during search:

1. Transpositions: The same position can be reached via different move orders.
2. Selfplay: During selfplay, the same network is playing both sides of the match, which
   means that positions searched by one side will frequently appear when the other side
   is searching on the next move.  Similarly, since `dlchess` does not try to re-use the
   search tree, previously evaluated positions may appear in later searches even if the
   agent is only playing one side.

`dlchess` uses a fixed-size, first-in first-out hash map to store the network inference
cache.  This is implemented using a class that contains a `std::unordered_map<uint64_t,
NetworkOutput>` and a `std::queue<uint64_t>`.  The map stores previously computed
network outputs based on a hash key of the input.  The queue keeps track of which keys
(positions) should be evicted once the maximum size is reached.

There is a small subtlety in defining the keys.  Nominally, the network input is an
encoding of a game position.  One might think of using the standard Zobrist position hash key
(which is already available in the `chess::Board` structure) as the network
input hash key.  However, this is not correct because the Zobrist hash does not account
for the repetition count or the fifty-move count, both of which are part of the network
input encoding.  As such, these two values are hashed and concatenated together with the
Zobrist position hash to produce the hash key for the network input.

The degree to which this reduces the need to query the neural network can be
significant.  For a trained network, the proportion of positions that can be found in
the cache (even when using a modest cache size) may be 25% or more.  This value tends to
be higher for trained networks, as untrained networks tend to have a less structured,
more random search.

## Experience Data

The experience data generated during selfplay are stored in a simple raw binary format
with accompanying metadata in json.  This makes it possible to efficiently load and take
subsets of the data in Python during training using
[`numpy.memmap`](https://numpy.org/doc/stable/reference/generated/numpy.memmap.html).

For each game, the experience data include:

* A tensor describing the input state before each move.
* A tensor describing the MCTS visit counts associated with each move (used as the
  target for the policy network).
* A tensor describing the reward (game outcome) associated with each move.

## Training

Training updates follow the approach outlined in the original AlphaZero work: the
network weights are updated using gradient descent based on a loss function that sums
over mean-squared error and cross-entropy losses.  In other words, the network's value
output is trained to predict the game outcome measure, which is defined as -1 for a
loss, 0 for a draw, and 1 for a win.  The network's policy output is trained to predict
the Monte Carlo tree search visit probabilities.  The training code is found here:
[train.py](https://github.com/mcfarljm/dlchess/blob/main/train/train.py).

Because training updates are implemented using PyTorch, training run sequences that
iterate between selfplay and training updates retain two versions of the network data:
(1) a PyTorch model state dict for use in subsequent training updates, and (2) an ONNX
export for use in the C++ code.
