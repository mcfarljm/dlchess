#ifndef HASHCAT_H_
#define HASHCAT_H_

#include <cstdint>

namespace utils {

  /* Adapated from Leela Chess Zero. */

  // Tries to scramble @val.
  inline uint64_t Hash(uint64_t val) {
    return 0xfad0d7f2fbb059f1ULL * (val + 0xbaad41cdcb839961ULL) +
      0x7acec0050bf82f43ULL * ((val >> 31) + 0xd571b3a92b1b2755ULL);
  }

  // Appends value to a hash.
  inline uint64_t HashCat(uint64_t hash, uint64_t x) {
    hash ^= 0x299799adf0d95defULL + Hash(x) + (hash << 6) + (hash >> 2);
    return hash;
  }

};


#endif // HASHCAT_H_
