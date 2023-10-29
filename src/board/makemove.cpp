#include <cassert>
#include "board.h"

namespace board {

  static const std::array<int, BOARD_SQ_NUM> CASTLE_PERM = {
    13, 15, 15, 15, 12, 15, 15, 14,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    7, 15, 15, 15,  3, 15, 15, 11,
  };

  void Board::hash_piece(Piece piece, Square sq) {
    hash ^= hasher.piece_keys[static_cast<int>(piece.value)][sq];
  }

  void Board::hash_side() {
    hash ^= hasher.side_key;
  }

  void Board::hash_en_pas() {
    hash ^= hasher.piece_keys[static_cast<int>(Piece::none)][en_pas];
  }

  void Board::hash_castle() {
    hash ^= hasher.castle_keys[castle_perm.to_ulong()];
  }

  void Board::clear_piece(Square sq) {
    auto piece = pieces[sq];
    assert(piece.exists());

    auto color = piece.color();

    hash_piece(piece, sq);
    pieces[sq] = Piece::none;

    bitboards[piece.value].clear_bit(sq);
    bb_sides[static_cast<int>(color)].clear_bit(sq);
    bb_sides[static_cast<int>(Color::both)].clear_bit(sq);
  }

  void Board::add_piece(Piece piece, Square sq) {
    assert(piece.exists());

    auto color = piece.color();

    hash_piece(piece, sq);
    pieces[sq] = piece;

    bitboards[piece.value].set_bit(sq);
    bb_sides[static_cast<int>(color)].set_bit(sq);
    bb_sides[static_cast<int>(Color::both)].set_bit(sq);
  }

  void Board::move_piece(Square from, Square to) {
    auto piece = pieces[from];
    auto color = piece.color();

    hash_piece(piece, from);
    pieces[from] = Piece::none;

    hash_piece(piece, to);
    pieces[to] = piece;

    bitboards[piece.value].clear_bit(from);
    bitboards[piece.value].set_bit(to);
    bb_sides[static_cast<int>(color)].clear_bit(from);
    bb_sides[static_cast<int>(color)].set_bit(to);
    bb_sides[static_cast<int>(Color::both)].clear_bit(from);
    bb_sides[static_cast<int>(Color::both)].set_bit(to);
  }

  bool Board::make_move(game_moves::Move mv) {
    assert(check());

    auto from = mv.from;
    auto to = mv.to;

    assert(pieces[from].exists());

    auto prev_hash = hash;

    if (mv.is_en_pas()) {
      if (side == Color::white)
        clear_piece(to - 8);
      else
        clear_piece(to + 8);
    }
    else if (mv.is_castle()) {
      if (to == Position::C1)
        move_piece(Position::A1, Position::D1);
      else if (to == Position::C8)
        move_piece(Position::A8, Position::D8);
      else if (to == Position::G1)
        move_piece(Position::H1, Position::F1);
      else if (to == Position::G8)
        move_piece(Position::H8, Position::F8);
      else
        throw std::runtime_error("invalid position in move with castle flag");
    }

    if (en_pas != Position::none)
      hash_en_pas();
    // Hash out current state
    hash_castle();

    history.emplace_back(mv, castle_perm, en_pas, fifty_move, hash);

    castle_perm &= CASTLE_PERM[from];
    castle_perm &= CASTLE_PERM[to];
    en_pas = Position::none;

    ++fifty_move;

    if (mv.is_capture()) {
      clear_piece(to);
      fifty_move = 0;
    }

    if (pieces[from].is_pawn()) {
      fifty_move = 0;
      if (mv.is_pawn_start()) {
        if (side == Color::white) {
          en_pas = from + 8;
          assert(en_pas/8 == squares::RANK_3);
        } else {
          en_pas = from - 8;
          assert(en_pas/8 == squares::RANK_6);
        }
        hash_en_pas();
      }
    }

    move_piece(from, to);

    if (mv.is_promotion()) {
      assert(mv.promote.exists() && ! mv.promote.is_pawn());
      clear_piece(to);
      add_piece(mv.promote, to);
    }

    if (pieces[to].is_king())
      king_sq[static_cast<int>(side)] = to;

    side = swap_side(side);
    hash_side();

    assert(check());

    if (square_attacked(king_sq[static_cast<int>(side)], side)) {
      // todo: undo
      return false;
    }

    return true;
  }

  void Board::undo_move() {
    assert(check());

    auto undo = history.back();
    history.pop_back();
    auto mv = undo.mv;
    auto from = mv.from;
    auto to = mv.to;

    if (en_pas != Position::none)
      hash_en_pas();
    hash_castle();

    castle_perm = undo.castle_perm;
    fifty_move = undo.fifty_move;
    en_pas = undo.en_pas;

    if (en_pas != Position::none)
      hash_en_pas();
    hash_castle();

    side = swap_side(side);
    hash_side();

    if (mv.is_en_pas()) {
      if (side == Color::white)
        add_piece(Piece::BP, to - 8);
      else
        add_piece(Piece::WP, to + 8);
    }
    else if (mv.is_castle()) {
      if (to == Position::C1)
        move_piece(Position::D1, Position::A1);
      else if (to == Position::C8)
        move_piece(Position::D8, Position::A8);
      else if (to == Position::G1)
        move_piece(Position::F1, Position::H1);
      else if (to == Position::G8)
        move_piece(Position::F8, Position::H8);
      else
        throw std::runtime_error("invalid position in move with castle flag");
    }

    move_piece(to, from);

    if (pieces[from].is_king())
      king_sq[static_cast<int>(side)] = from;

    if (mv.is_capture())
      add_piece(mv.capture, to);

    if (mv.is_promotion()) {
        assert(mv.promote.exists() && ! mv.promote.is_pawn());
        clear_piece(from);
        add_piece((mv.promote.color() == Color::white) ? Piece::WP : Piece::BP, from);
    }

    assert(check());
  }

};
