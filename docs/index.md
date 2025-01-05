# DLChess

Welcome to the documentation for the DLChess (deep learning for chess) project.

DLChess is a UCI-compatible chess engine and reinforcement learning system.  It
implements an [AlphaZero](https://arxiv.org/abs/1712.01815)-style model that combines
Monte Carlo Tree Search (MCTS) with a neural network for policy and value prediction.
The primary goal of the project is to explore how strong the program could become using
a relatively small neural network together with selfplay and training on modest
hardware.

Note that the AlphaZero method uses self-play and reinforcement learning to train a
neural network, without any domain knowledge or hand-crafted evaluation functions (e.g.,
the network starts out knowing nothing about piece values).  No external game data is
used in the process of training the network.  This makes the training process very
computationally demanding (the original work on AlphaZero by DeepMind employed thousands
of GPUs).  However, this project shows that using smaller networks, training an
intermediate strength model is possible even using just one, consumer-grade CPU.

See the [Design](design/index.md) section for more information about the design and
implementation of dlchess.  See [Training Runs](training-runs.md) for information about the
runs that have been performed so far to train networks.  There is also a page of [Sample
Games](sample-games.md) that gives a sense of how the networks play versus a human.
