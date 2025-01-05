# DLChess

DLChess is a UCI-compatible chess engine and reinforcement learning system.  It implements an [AlphaZero](https://arxiv.org/abs/1712.01815)-style model that combines Monte Carlo Tree Search (MCTS) with a neural network for policy and value prediction.  The training process starts with a randomly initialized neural network, which is iteratively improved through a series of selfplay and training update steps.  The primary goal of the project is to explore how strong the program could become using a relatively small neural network together with selfplay and training on modest hardware.  The latest [Training Run](https://mcfarljm.github.io/dlchess/training-runs/) required about four weeks of computing time on a refurbished mini-pc and produced a network that plays at a strength of about 1400 ELO.

The design and implementation of the algorithm follow the sister project [DLGo](https://github.com/mcfarljm/dlgo), which itself largely followed the book [Deep Learning and the Game of Go](https://www.manning.com/books/deep-learning-and-the-game-of-go).  The implementation of the chess board structure, rules, and move generation follows [Chareth](https://github.com/mcfarljm/chareth).

The code for the chess engine is written in C++.  The code that defines the neural network arechitecture and training is written in Python using PyTorch.  The neural networks are exported to ONNX format for evaluation in the C++ code, which is currently done using the ONNX runtime.  The use of the ONNX runtime was found to provide about a 2.5x speedup relative to use of TorchScript and LibTorch.

Refer to the [Documentation](https://mcfarljm.github.io/dlchess/) for more details about the [Design](https://mcfarljm.github.io/dlchess/design/), information about [Training Runs](https://mcfarljm.github.io/dlchess/training-runs/), as well as [Sample Games](https://mcfarljm.github.io/dlchess/sample-games/) played against the networks.

## Features

* AlphaZero-style engine that combines MCTS with a multi-output neural network.
  * Dirichlet random noise added to move priors at the root node of each search during selfplay.
  * Supports both greedy and proportional move selection based on visit counts.
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
