#ifndef ENCODER_H
#define ENCODER_H

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

};


#endif // ENCODER_H
