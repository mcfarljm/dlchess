#ifndef ENCODER_H
#define ENCODER_H

#include <unordered_map>
#include <torch/torch.h>

#include "../board/board.h"

namespace zero {

  class Encoder {
  public:
    virtual torch::Tensor encode(const board::Board&) const = 0;
    /* virtual Move decode_move_index(int index) const = 0; */
  };

  class SimpleEncoder : public Encoder {
  public:
    torch::Tensor encode(const board::Board&) const;
    /* Move decode_move_index(int index) const; */
  };

  std::unordered_map<game_moves::Move, std::array<int,3>, game_moves::MoveHash>
  decode_legal_moves(const board::Board&);

};


#endif // ENCODER_H
