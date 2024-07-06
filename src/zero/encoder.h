#ifndef ENCODER_H
#define ENCODER_H

#include <vector>
#include <unordered_map>

#include "tensor.h"
#include "../board/board.h"


namespace zero {

  class Encoder {
  public:
    // Todo: restore const
    virtual Tensor<float> encode(const board::Board&) = 0;
    /* virtual Move decode_move_index(int index) const = 0; */
  };

  class SimpleEncoder : public Encoder {
    bool _transform_position;
  public:
    int _transform_count = 0; // tmp
    int _num_calls = 0; // tmp
    SimpleEncoder(bool transform_position = true) : _transform_position{transform_position} {}
    Tensor<float> encode(const board::Board&);
    /* Move decode_move_index(int index) const; */
  };

  std::unordered_map<game_moves::Move, std::array<int,3>, game_moves::MoveHash>
  decode_legal_moves(const board::Board&);
  extern const std::array<int, 3> PRIOR_SHAPE;

};


#endif // ENCODER_H
