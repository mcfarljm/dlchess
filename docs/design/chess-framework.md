# Chess Framework

This section introduces the *chess framework* that `dlchess` is built on.  The framework
provides the capabilities for representing the state of the board, enumerating all legal
moves, apply/undoing moves, determining the game termination state, etc.  Ideally, we
want the implementation to be very efficient, as we want to minimize all overhead
associated with the chess framework.

The goal of this section is to highlight some of the key functions that the deep
learning system interacts with, and we will also review some of the lesser-known chess
rules, which also come into play in the implementation of the search algorithms.

The chess framework used by dlchess was written by the author in C++ and is adapted from
the design detailed in the outstanding video tutorial series,
[VICE](https://www.youtube.com/playlist?list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg).  The
key data structures that we care about are:

- `chess::Board`: The main data structure, which describes the state of the game (board)
  at a given point in time, including the complete move history.
- `chess::Move`: A compact representation of a move, which contains the "from" and "to"
  squares, piece capture information, promotion information, and a flag for en passant,
  castling, and pawn starts.

The key functions that are  used within the search code are:

- `Board::make_move(Move)`: Update the board state in place by making the given move.
  Note that although our chess framework also provides a corresponding
  `Board::undo_move` function, we won't be using it in the deep learning implementation.
  Instead, as we expand our search tree, we will first copy the current `Board` state
  and then apply the move.
- `Board::generate_legal_moves()`: Return a vector of `Move` instances for all legal
  moves from a given `Board` position.
- `Board::is_over()`: Return a bool indicating whether the game represented by the given
  state is over.
- `Board::winner()`: Return an `optional` instance that is either null (if the game is
  not over) or indicates the game outcome (either one side wins or the game is drawn).

In order for the chess framework to be able to correctly enumerate all moves and
identify game termination, it must represent all rules, including some of the more
esoteric rules of the game.  These include:

Castling

:   Each player's king has the possibility of making two different castling
    moves, which are possible under certain conditions.  In particular, the king cannot
    castle while in check, and castling privileges are lost once the king moves or a given
    rook moves.  Thus, the `chess::Board` structure must keep track of the state of all
    four castling permissions.

En passant

:   This is a special pawn capture move that is only allowed in a special
    circumstance where the opponent's pawn has moved forward two squares.  Importantly, the
    en passant capture is only available on the next move.  Thus, the `chess::Board`
    structure must encode the status of whether en passant capture is available, and if
    so, on which square.
    
Three-fold repetition

:   Three-fold repetition: If the same position occurs on the board three times in total
    (not necessarily consecutive), then the game terminates in a draw.  The chess
    framework handles this by using a position hash.  A hash function encodes each
    position into a large integer value.  The `chess::Board` structure keeps track of the
    hash of every previous position, which enables it to check for three-fold repetition.

Fifty-move rule

:   If fifty full moves (one move from each side) are played without a
    capture or a pawn advance, then the game ends in a draw.  Again, the `chess::Board`
    structure must keep track of the fifty-move count.
