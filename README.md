# DLChess

DLChess is a small project that was used to test and investigate deep reinforcement learning for chess.  It implements an [AlphaZero](https://arxiv.org/abs/1712.01815)-style model that combines Monte Carlo Tree Search (MCTS) with a neural network for policy and value prediction.  The primary goal was to explore how strong the program could become using a relatively small neural network together with selfplay and training on modest hardware.

Refer to the [Documentation](https://mcfarljm.github.io/dlchess/) for more details about the [Design](https://mcfarljm.github.io/dlchess/design/), information about [Training Runs](https://mcfarljm.github.io/dlchess/training-runs/), as well as [Sample Games](https://mcfarljm.github.io/dlchess/sample-games/) played against the networks.

The design and implementation of the algorithm follows the sister project [DLGo](https://github.com/mcfarljm/dlgo), which itself largely followed the book [Deep Learning and the Game of Go](https://www.manning.com/books/deep-learning-and-the-game-of-go).  The implementation of the chess board structure, rules, and move generation follows [Chareth](https://github.com/mcfarljm/chareth).

The code is written in C++.  The neural networks are defined in PyTorch and exported to ONNX format for evaluation in the C++ code, which is currently done using the ONNX runtime.  This was found to provide about a 2.5x speedup relative to use of TorchScript and LibTorch.

## Features

* AlphaZero-style engine that combines MCTS with a multi-output neural network.
  * Dirichlet random noise added to move priors at the root node of each search.
  * Accommodates both greedy and proportional move selection based on visit counts.
  * Monte Carlo Tree Search is done in serial, dynamic memory is used for tree expansion, and the search tree is reset for each move.
  * Neural network results are cached using a fixed-size map with a first-in, first-out eviction policy.
* Support for UCI communication protocol.
* Complete framework for self-play and training.  The [`run_training.sh`](scripts/run_training.sh) Bash script is provided as an example for fully-automated and parallelized self-play and training updates.

## Usage

CMake and ONNX Runtime are required to build and run the engine using an existing neural network file.  PyTorch is required for training.

* `mkdir build; cd build; cmake .. -DONNXRUNTIME_ROOTDIR=<path-to-onnxruntime> -DCMAKE_BUILD_TYPE=RELEASE; make`
* Run tests using `ctest`
* See usage information for the UCI driver: `./dlchess -h`
* See usage information for the self-play driver: `./selfplay -h`
* To run self-play training iterations, see the [`run_training.sh`](scripts/run_training.sh) example script, which provides a starting point.

Refer also to the provided [`Dockerfile`](Dockerfile), which prepares a container with all tools necessary for training, selfplay, and evaluation using cutechess-cli.

## Design

The neural network is implemented using PyTorch.  During training, two versions of the network data are retained: (1) the network weights for use in subsequent training updates, and (2) an ONNX export for use in the C++ code.

The experience data generated during self-play are stored in a simple raw binary format with accompanying metadata in json.  This makes it possible to efficiently load and take subsets of the data in Python during training using [`numpy.memmap`](https://numpy.org/doc/stable/reference/generated/numpy.memmap.html).

For each game, the experience data include:

* A tensor describing the input state before each move.
* A tensor describing the MCTS visit counts associated with each move (used as the target for the policy network).
* A tensor describing the reward (game outcome) associated with each move.
