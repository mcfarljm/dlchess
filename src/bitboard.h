#ifndef BITBOARD_H_
#define BITBOARD_H_

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


  struct Bitboard {
    std::bitset<64> bits;

    bool nonzero() const {
      return bits.any();
    }

    void set_bit(Square index) {
      bits.set(index);
    }

    void clear_bit(Square index) {
      bits.reset(index);
    }

    int count() const {
      return bits.count();
    }

    Square pop_bit() {
      auto val = bits.to_ullong();
      auto sq = std::countr_zero(val); // Least-significant bit

      // bits.reset(sq); // This is one solution, but is it slower?

      val &= val - 1; // Clear least-significant bit
      bits = val;

      return sq;
    }

    friend std::ostream& operator<<(std::ostream&, const Bitboard& b);

    BitboardIterator begin() const { return BitboardIterator(bits); }
    BitboardIterator end()   const { return BitboardIterator(0); }

  };

};


#endif // BITBOARD_H_

