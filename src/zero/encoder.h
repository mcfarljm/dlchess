#ifndef ENCODER_H
#define ENCODER_H

#include <vector>
#include <unordered_map>

#include "tensor.h"
#include "../board/board.h"


namespace zero {

  class Encoder {
  public:
    virtual Tensor<float> encode(const board::Board&) const = 0;
    /* virtual Move decode_move_index(int index) const = 0; */
  };

  class SimpleEncoder : public Encoder {
  public:
    Tensor<float> encode(const board::Board&) const override;
    /* Move decode_move_index(int index) const; */
  };

  std::unordered_map<game_moves::Move, std::array<int,3>, game_moves::MoveHash>
  decode_legal_moves(const board::Board&);
  extern const std::array<int, 3> PRIOR_SHAPE;

};


#endif // ENCODER_H
