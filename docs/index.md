# dlchess

Welcome to the documentation for the dlchess (deep learning for chess) project.

dlchess is a small, efficient implementation of the AlphaZero reinforcement learning
method for chess, written from scratch in C++.  The goals of the project are to explore
and learn about the method, while seeking to produce an intermediate strength chess
engine through training on modest consumer hardware in practical time-scales
(days-weeks).

Note that the AlphaZero method uses self-play and reinforcement learning to train a
neural network, without any domain knowledge or hand-crafted evaluation functions (e.g.,
the network starts out knowing nothing about piece values).  No external game data is
used in the process of training the network.  This makes the training process very
computationally demanding (the original work on AlphaZero by DeepMind employed thousands
of GPUs).  However, this project shows that using smaller networks, training an
intermediate strength model is possible even using just one, consumer-grade CPU.

See the [Design](design) section for more information about the design and
implementation of dlchess.  See [Training Runs](training-runs) for information about the
runs that have been performed so far to train networks.  There is also a page of [Sample
Games](sample-games) that gives a sense of how the networks play versus a human.
