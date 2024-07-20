#include <array>
#include <bit>

#include "obs_diff.h"


namespace {

  using chess::u64;
  using chess::Square;

  u64 rank_mask(Square sq) {
    return 0xffLL << (sq & 56);
  }

  u64 file_mask(Square sq) {
    return 0x0101010101010101LL << (sq & 7);
  }

  u64 diagonal_mask(Square sq) {
    const u64 main_diag = 0x8040201008040201;
    const int diag = 8*(sq & 7) - (sq & 56);
    auto north = -diag & (diag >> 31);
    auto south = diag & (-diag >> 31);
    return (main_diag >> south) << north;
  }

  u64 anti_diagonal_mask(Square sq) {
    const u64 main_diag = 0x0102040810204080;
    const int diag = 56 - 8*(sq & 7) - (sq & 56);
    auto north = -diag & (diag >> 31);
    auto south = diag & (-diag >> 31);
    return (main_diag >> south) << north;
  }

  u64 positive_ray(Square sq, u64 line) {
    return line & (0LL - (2LL << sq));
  }

  u64 negative_ray(Square sq, u64 line) {
    return line & ((1ULL << sq) - 1);
  }

  struct ObsDiffMask {
    u64 lower = 0;
    u64 upper = 0;
    u64 line_exc = 0; // lower | upper

    ObsDiffMask() = default;

    ObsDiffMask(u64 lower, u64 upper) :
      lower(lower), upper(upper), line_exc(lower | upper) {}
  };

  using ObsDiffArray = std::array<std::array<ObsDiffMask, 64>, 4>;

  ObsDiffArray get_obstruction_diff_masks() {
    ObsDiffArray os_masks;

    for (Square sq=0; sq<64; ++sq) {
      // Rank masks:
      auto line = rank_mask(sq);
      auto upper = positive_ray(sq, line);
      auto lower = negative_ray(sq, line);
      os_masks[0][sq] = ObsDiffMask(lower, upper);

      // File masks:
      line = file_mask(sq);
      upper = positive_ray(sq, line);
      lower = negative_ray(sq, line);
      os_masks[1][sq] = ObsDiffMask(lower, upper);

      // Diagonal masks:
      line = diagonal_mask(sq);
      upper = positive_ray(sq, line);
      lower = negative_ray(sq, line);
      os_masks[2][sq] = ObsDiffMask(lower, upper);

      // Anti-diagonal masks:
      line = anti_diagonal_mask(sq);
      upper = positive_ray(sq, line);
      lower = negative_ray(sq, line);
      os_masks[3][sq] = ObsDiffMask(lower, upper);
    }
    
    return os_masks;
  }

  static const ObsDiffArray OBS_DIFF_MASKS = get_obstruction_diff_masks();

};

namespace chess {

  u64 get_line_attacks(u64 occ, int direction, Square sq) {
    auto os_mask = OBS_DIFF_MASKS[direction][sq];
    auto lower = os_mask.lower & occ;
    auto upper = os_mask.upper & occ;
    auto ms1b = 0x8000000000000000LL >> std::countl_zero(lower | 1);
    auto ls1b = upper & (-upper);
    auto odiff = 2*ls1b - ms1b;
    return os_mask.line_exc & odiff;
  }

};
