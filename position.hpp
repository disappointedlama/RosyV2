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
#include "masks.hpp"
#include "evaluationTables.hpp"

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

	U64 get_pinned_pieces(const int kingpos, const U64 enemy_attacks);
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
	inline int get_phase() {
		int ret = count_bits(bitboards[P] | bitboards[p]);
		ret += 3 * count_bits(bitboards[N] | bitboards[n] | bitboards[B] | bitboards[b]);
		ret += 5 * count_bits(bitboards[R] | bitboards[r]);
		ret += 9 * count_bits(bitboards[Q] | bitboards[q]);
		return ret;
	}
	inline int raw_material(const bool is_endgame) {
		int ret = openingKingTableWhite[bitscan(bitboards[K])] - openingKingTableBlack[bitscan(bitboards[k])];
		U64 whitePawns = bitboards[P];
		while (whitePawns) {
			const U64 isolated = whitePawns & twos_complement(whitePawns);
			const int ind = bitscan(isolated);
			ret += basePieceValue[P] + (!is_endgame) * openingPawnTableWhite[ind] + (is_endgame)*endgamePawnTableWhite[ind];
			whitePawns &= ones_decrement(whitePawns);
		}
		U64 blackPawns = bitboards[p];
		while (blackPawns) {
			const U64 isolated = blackPawns & twos_complement(blackPawns);
			const int ind = bitscan(isolated);
			ret -= basePieceValue[P] + (!is_endgame) * openingPawnTableBlack[ind] + (is_endgame)*endgamePawnTableBlack[ind];
			blackPawns &= ones_decrement(blackPawns);
		}

		U64 whiteKnights = bitboards[N];
		while (whiteKnights) {
			const U64 isolated = whiteKnights & twos_complement(whiteKnights);
			ret += basePieceValue[N] + openingKnightsTable[bitscan(isolated)];
			whiteKnights &= ones_decrement(whiteKnights);
		}
		U64 blackKnights = bitboards[n];
		while (blackKnights) {
			const U64 isolated = blackKnights & twos_complement(blackKnights);
			ret -= basePieceValue[N] + openingKnightsTable[bitscan(isolated)];
			blackKnights &= ones_decrement(blackKnights);
		}

		U64 whiteBishops = bitboards[B];
		while (whiteBishops) {
			const U64 isolated = whiteBishops & twos_complement(whiteBishops);
			ret += basePieceValue[B] + openingBishopTableWhite[bitscan(isolated)];
			whiteBishops &= ones_decrement(whiteBishops);
		}
		U64 blackBishops = bitboards[b];
		while (blackBishops) {
			const U64 isolated = blackBishops & twos_complement(blackBishops);
			ret -= basePieceValue[B] + openingBishopTableBlack[bitscan(isolated)];
			blackBishops &= ones_decrement(blackBishops);
		}

		U64 whiteRooks = bitboards[R];
		while (whiteRooks) {
			const U64 isolated = whiteRooks & twos_complement(whiteRooks);
			ret += basePieceValue[R] + openingRookTableWhite[bitscan(isolated)];
			whiteRooks &= ones_decrement(whiteRooks);
		}
		U64 blackRooks = bitboards[r];
		while (blackRooks) {
			const U64 isolated = blackRooks & twos_complement(blackRooks);
			ret -= basePieceValue[R] + openingQueenTableBlack[bitscan(isolated)];
			blackRooks &= ones_decrement(blackRooks);
		}

		U64 whiteQueens = bitboards[Q];
		while (whiteQueens) {
			const U64 isolated = whiteQueens & twos_complement(whiteQueens);
			ret += basePieceValue[Q] + openingQueenTableWhite[bitscan(isolated)];
			whiteQueens &= ones_decrement(whiteQueens);
		}
		U64 blackQueens = bitboards[q];
		while (blackQueens) {
			const U64 isolated = blackQueens & twos_complement(blackQueens);
			ret -= basePieceValue[Q] + openingQueenTableBlack[bitscan(isolated)];
			blackQueens &= ones_decrement(blackQueens);
		}
		return ret;
	}
public:
	const static int infinity = 1000000;
	U64 current_hash;
	Position();
	Position(const std::string& fen);
	void parse_fen(std::string fen);
	std::string fen();
	void print() const;
	U64 get_hash() const;
	constexpr bool get_side() const { return side; };
	constexpr int get_ply() const { return ply; };
	inline void update_hash(const int move);
	inline void make_move(const int move);
	inline void unmake_move();
	inline void make_nullmove();
	inline void unmake_nullmove();
	void get_legal_moves(std::vector<int>& ret);
	void get_legal_captures(std::vector<int>& ret);
	constexpr bool is_draw_by_repetition() {
		return (std::count(hash_history.begin(), hash_history.end(), current_hash)) >= 3;
	}
	constexpr bool is_draw_by_fifty_moves() {
		return no_pawns_or_captures >= 50;
	}
	inline bool currently_in_check() {
		return is_attacked_by_side(bitscan(bitboards[5 + (side) * 6]), !side);
	}
	inline int evaluate(const bool is_draw, const bool is_lost) {
		if (is_draw) {
			return 0;
		}
		if (is_lost) {
			return -infinity;
		}
		const int phase = get_phase();
		const bool is_endgame = (phase < 30);
		return raw_material(is_endgame);
	}
};