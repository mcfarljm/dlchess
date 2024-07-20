#ifndef ENCODER_H
#define ENCODER_H

#include <vector>
#include <unordered_map>

#include "tensor.h"
#include "../chess/board.h"


namespace zero {

  class Encoder {
  public:
    virtual Tensor<float> encode(const chess::Board&) const = 0;
    /* virtual Move decode_move_index(int index) const = 0; */
  };

  class SimpleEncoder : public Encoder {
    bool en_passant_;
  public:
    SimpleEncoder(int version=1) : en_passant_{version>0} {}
    Tensor<float> encode(const chess::Board&) const override;
    /* Move decode_move_index(int index) const; */
  };

  std::unordered_map<chess::Move, std::array<int,3>, chess::MoveHash>
  decode_legal_moves(const chess::Board&);
  extern const std::array<int, 3> PRIOR_SHAPE;

};


#endif // ENCODER_H
