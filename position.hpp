#pragma once
#include <vector>
#include <chrono>

#include "hashKeys.hpp"
#include "move.hpp"
#include "pawnAttacks.hpp"
#include "knightAttacks.hpp"
#include "bishopAttacks.hpp"
#include "rookAttacks.hpp"
#include "kingAttacks.hpp"

//sides to move
enum { white, black, both };
//castling bits
enum { wk = 1, wq = 2, bk = 4, bq = 8 };
//pieces
enum { P, N, B, R, Q, K, p, n, b, r, q, k };
//ASCII pieces
constexpr char ascii_pieces[] = "PNBRQKpnbrqk";

//convert ascii char pieces to encoded constants
static constexpr int char_pieces(const char piece) {
	switch (piece) {
	case 'P':return P;
	case 'N':return N;
	case 'B':return B;
	case 'R':return R;
	case 'Q':return Q;
	case 'K':return K;
	case 'p':return p;
	case 'n':return n;
	case 'b':return b;
	case 'r':return r;
	case 'q':return q;
	case 'k':return k;
	default:return -1;
	}
};
static constexpr U64 get_queen_attacks(U64 occ, const int sq) {
	return get_bishop_attacks(occ, sq) | get_rook_attacks(occ, sq);
};
class Position {
	std::array<U64, 12> bitboards; // P, N, B, R, Q, K, p, n, b, r, q, k
	std::array<U64, 3> occupancies;
	bool side;
	int ply;
	int enpassant_square;
	int castling_rights;//wk,wq,bk,bq
	int no_pawns_or_captures;
	U64 current_hash;
	std::vector<int> move_history;
	std::vector<int> enpassant_history;
	std::vector<int> castling_rights_history;
	std::vector<int> no_pawns_or_captures_history;
	std::vector<U64> hash_history;
	inline bool is_attacked_by_side(const int sq, const bool color);
	inline U64 get_attacks_by(const bool color);
	inline int get_piece_type_on(const int sq);

	void legal_move_generator(std::vector<int>& ret, const int kingpos, const U64 kings_queen_scope, const U64 enemy_attacks);
	void legal_in_check_move_generator(std::vector<int>& ret, const int kingpos, const U64 kings_queen_scope, const U64 enemy_attacks);

	void legal_capture_gen(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks);
	void legal_in_check_capture_gen(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks);
	void in_check_get_pawn_captures(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets);
	void get_pawn_captures(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned);

	U64 get_pinned_pieces(const int& kingpos, const U64 enemy_attacks);
	U64 get_moves_for_pinned_pieces(std::vector<int>& ret, const int kingpos, const U64 enemy_attacks);
	U64 get_captures_for_pinned_pieces(std::vector<int>& ret, const int kingpos, const U64 enemy_attacks);
	U64 get_checkers(const int kingpos);
	U64 get_checking_rays(const int kingpos);
	void try_out_move(std::vector<int>& ret, int move);
	inline void get_legal_pawn_moves(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned);
	inline void legal_bpawn_pushes(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned);
	inline void legal_wpawn_pushes(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned);
	inline void legal_bpawn_captures(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned);
	inline void legal_wpawn_captures(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned);

	inline void in_check_get_legal_pawn_moves(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, const U64 in_check_valid);
	inline void in_check_legal_bpawn_pushes(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, const U64 in_check_valid);
	inline void in_check_legal_wpawn_pushes(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, const U64 in_check_valid);
	inline void in_check_legal_bpawn_captures(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets);
	inline void in_check_legal_wpawn_captures(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets);

	inline void get_castles(std::vector<int>& ptr);
public:
	Position();
	Position(const std::string& fen);
	void parse_fen(std::string fen);
	void print() const;
	U64 get_hash() const;
	inline void update_hash(const int move);
	inline void make_move(const int move);
	inline void unmake_move();
	inline void make_nullmove();
	inline void unmake_nullmove();
	void get_legal_moves(std::vector<int>& ret);
	void get_legal_captures(std::vector<int>& ret);
};