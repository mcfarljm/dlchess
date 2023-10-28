#ifndef BITBOARD_H_
#define BITBOARD_H_

#include <cstdint>
#include <bitset>
#include <bit>

#include "squares.h"

using squares::Square;


namespace bitboard {

  struct BitboardIterator {

    using iterator_category = std::input_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = Square;
    using pointer           = value_type*;
    using reference         = const value_type&;

    BitboardIterator(std::bitset<64> bits) : bits(bits) {
      if (bits.any())
        sq = std::countr_zero(bits.to_ullong());
    }

    reference operator*() const { return sq; }

    // Prefix increment
    BitboardIterator& operator++() {
      bits.reset(sq);
      if (bits.any())
        sq = std::countr_zero(bits.to_ullong());
      return *this;
    }

    friend bool operator== (const BitboardIterator& a, const BitboardIterator& b) { return a.bits == b.bits; };
    friend bool operator!= (const BitboardIterator& a, const BitboardIterator& b) { return a.bits != b.bits; };

  private:
    std::bitset<64> bits;
    Square sq;
  };


  struct Bitboard : public std::bitset<64> {

    bool nonzero() const {
      return any();
    }

    void set_bit(Square index) {
      set(index);
    }

    void clear_bit(Square index) {
      reset(index);
    }

    // If this is not defined, the type of operator| will be bitset
    Bitboard operator|(const Bitboard& rhs) const {
      return *this | rhs;
    }

    friend std::ostream& operator<<(std::ostream&, const Bitboard& b);

    BitboardIterator begin() const { return BitboardIterator(*this); }
    BitboardIterator end()   const { return BitboardIterator(0); }

  };

};


#endif // BITBOARD_H_

