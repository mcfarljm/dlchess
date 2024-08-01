# UCI (Universal Chess Interface)

The Universal Chess Interface, known as UCI, is the standard interface for communicating
with a chess engine.  This is used by chess GUIs, programs for managing tournament play
between chess engines, connecting bots online, and more.  Here we review the basics of
how this works and how it is implemented in `dlchess`.

The basic concept is that all communication is done via standard input and output using
text commands.  During the course of a game, the process managing the game (e.g., the
chess GUI) will send a message about the current game state, followed by a message
telling the chess enging to find the best move.  The chess engine processes this
information, performs its search, and then responds with a message indicating the
selected move.  Of course, there are some additional details for providing the chess
engine with information about the time clocks, setting options, and more.

In `dlchess`, the AlphaZero search algorithm is implemented against a simple abstract
interface, which we refer to as an "Agent".  At a minimum, the Agent must provide a
method for selecting a move when shown a game state.  In the following example, we also
provide a method for the agent to configure itself based on the state of the clock.  As
discussed in [Chess Framework](chess-framework.md), the `Board` structure represents the
game state.


```c++ title="Agent interface"
class Agent {
 public:
  virtual chess::Move select_move(const chess::Board&) = 0;
  virtual void set_search_time(std::optional<int> move_time_ms,
                               std::optional<int> time_left_ms,
                               std::optional<int> inc_ms,
                               const chess::Board& b) {}
};
```

UCI communication can then be managed via a loop that repeatedly polls standard input
for commands and then handles them accordingly.  The following example shows handling of
of a subset of the UCI commands, where the corresponding actions are implemented via
separate functions.  The most important commands are `"position"` and `"go"`.  The
`"position"` command is used to supply information about the current game state.  This
is done through a combination of a FEN string and an optional sequence of moves.  During
a game, a chess GUI will typically represent each position by providing the starting FEN
followed by a list of all subsequent moves, so that the engine has access to the game
history.  Note that we handle the `"position"` command by calling the `parse_pos`
function, which returns a `Board` instance.  Then, the `"go"` command is handled by the
`parse_go` function, where we provide a copy of the current UCI line (which may contain
additional parameters after the "go" command), the `Board` instance, and the `agent`
pointer.  A portion of the implementation of the `parse_go` function is shown below,
where we see that the chess engine responds with the `"bestmove"` command, to
communicate the selected move back to the process that is managing the game.


```c++
void uci_loop(zero::Agent* agent) {
  auto b = chess::Board();

  agent->info.game_mode = zero::GameMode::uci;

  uci_ok();

  std::string input;

  while (true) {
    std::cout << std::flush;

    std::getline(std::cin, input);

    if (input[0] == '\n')
      continue;
    else if (input.starts_with("isready"))
      std::cout << "readyok" << std::endl;
    else if (input.starts_with("position"))
      b = parse_pos(input);
    else if (input.starts_with("ucinewgame"))
      b = parse_pos("position startpos\n");
    else if (input.starts_with("setoption"))
      parse_setoption(input, agent);
    else if (input.starts_with("go"))
      parse_go(input, b, agent);
    else if (input.starts_with("quit"))
      break;
  }
};
```

```c++
void parse_go(std::string& line, const chess::Board& b, Agent* agent) {
  ...
  
  agent->set_search_time(move_time_ms, time_left_ms, inc_ms, b);
  auto mv = agent->select_move(b);
  std::cout << "bestmove " << mv << std::endl;
}
```
