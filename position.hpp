#pragma once
#include <vector>
#include <chrono>
#include <cassert>
#include "immintrin.h"
#include "intrin.h"
#include <unordered_map>

#include "hashKeys.hpp"
#include "move.hpp"
#include "pawnAttacks.hpp"
#include "knightAttacks.hpp"
#include "bishopAttacks.hpp"
#include "rookAttacks.hpp"
#include "kingAttacks.hpp"
#include "masks.hpp"
#include "evaluationTables.hpp"
enum {
	a8, b8, c8, d8, e8, f8, g8, h8,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a1, b1, c1, d1, e1, f1, g1, h1
};
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
static const std::string start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
struct Position_Error : std::exception {
	std::string msg;
	Position_Error(std::string msg) {
		this->msg = msg;
	}
	const std::string what() throw() {
		return std::format("Position Error: {}", msg);
	}

};
class Position {
	inline bool is_attacked_by_side(const int sq, const bool color);
	inline U64 get_attacks_by(const bool color);
	inline int get_piece_type_on(const int sq);
	inline int get_piece_type_or_enpassant_on(const int sq) {
		if (sq == enpassant_square && sq != a8) return (!side) * p;
		return square_board[sq];
	};

	void legal_move_generator(std::array<unsigned int,128>& ret, const int kingpos, const U64 kings_queen_scope, const U64 enemy_attacks, int& ind);
	void legal_in_check_move_generator(std::array<unsigned int,128>& ret, const int kingpos, const U64 kings_queen_scope, const U64 enemy_attacks, int& ind);

	void legal_capture_gen(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, int& ind);
	void legal_in_check_capture_gen(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, int& ind);
	void in_check_get_pawn_captures(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, int& ind);
	void get_pawn_captures(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, int& ind);

	U64 get_pinned_pieces(const int kingpos, const U64 enemy_attacks);
	U64 get_moves_for_pinned_pieces(std::array<unsigned int,128>& ret, const int kingpos, const U64 enemy_attacks,int &ind);
	U64 get_captures_for_pinned_pieces(std::array<unsigned int,128>& ret, const int kingpos, const U64 enemy_attacks, int& ind);
	U64 get_checkers(const int kingpos);
	U64 get_checking_rays(const int kingpos);
	void try_out_move(std::array<unsigned int,128>& ret, unsigned int move, int& ind);
	inline void get_legal_pawn_moves(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, int& ind);
	inline void legal_bpawn_pushes(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, int& ind);
	inline void legal_wpawn_pushes(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, int& ind);
	inline void legal_bpawn_captures(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, int& ind);
	inline void legal_wpawn_captures(std::array<unsigned int, 128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, int& ind);

	inline void in_check_get_legal_pawn_moves(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, const U64 in_check_valid, int& ind);
	inline void in_check_legal_bpawn_pushes(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, const U64 in_check_valid, int& ind);
	inline void in_check_legal_wpawn_pushes(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, const U64 in_check_valid, int& ind);
	inline void in_check_legal_bpawn_captures(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, int& ind);
	inline void in_check_legal_wpawn_captures(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, int& ind);

	inline void get_castles(std::array<unsigned int,128>& ptr, const U64 enemy_attacks, int& ind);
	inline short raw_material(const short phase) {
		short ret = (((openingKingTableWhite[bitscan(bitboards[K])] - openingKingTableBlack[bitscan(bitboards[k])]) * (256 - phase)) + ((endgameKingTable[bitscan(bitboards[K])] - endgameKingTable[bitscan(bitboards[k])]) * phase)) / 256
					+ (count_bits(bitboards[P]) - count_bits(bitboards[p])) * basePieceValue[P]
					+ (count_bits(bitboards[N]) - count_bits(bitboards[n])) * basePieceValue[N]
					+ (count_bits(bitboards[B]) - count_bits(bitboards[b])) * basePieceValue[B]
					+ (count_bits(bitboards[R]) - count_bits(bitboards[r])) * basePieceValue[R]
					+ (count_bits(bitboards[Q]) - count_bits(bitboards[q])) * basePieceValue[Q];
		U64 whitePawns = bitboards[P];
		short pawnOpening = 0;
		short pawnEndgame = 0;
		while (whitePawns) {
			const U64 isolated = _blsi_u64(whitePawns);
			const int ind = bitscan(isolated);
			pawnOpening += openingPawnTableWhite[ind];
			pawnEndgame += endgamePawnTableWhite[ind];
			whitePawns = _blsr_u64(whitePawns);
		}
		U64 blackPawns = bitboards[p];
		while (blackPawns) {
			const U64 isolated = _blsi_u64(blackPawns);
			const int ind = bitscan(isolated);
			pawnOpening -= openingPawnTableBlack[ind];
			pawnEndgame -= endgamePawnTableBlack[ind];
			blackPawns = _blsr_u64(blackPawns);
		}
		ret += (pawnOpening * (256 - phase) + pawnEndgame * phase) / 256;
		U64 whiteKnights = bitboards[N];
		while (whiteKnights) {
			const U64 isolated = _blsi_u64(whiteKnights);
			ret += openingKnightsTable[bitscan(isolated)];
			whiteKnights = _blsr_u64(whiteKnights);
		}
		U64 blackKnights = bitboards[n];
		while (blackKnights) {
			const U64 isolated = _blsi_u64(blackKnights);
			ret -= openingKnightsTable[bitscan(isolated)];
			blackKnights = _blsr_u64(blackKnights);
		}

		U64 whiteBishops = bitboards[B];
		while (whiteBishops) {
			const U64 isolated = _blsi_u64(whiteBishops);
			ret += openingBishopTableWhite[bitscan(isolated)];
			whiteBishops = _blsr_u64(whiteBishops);
		}
		U64 blackBishops = bitboards[b];
		while (blackBishops) {
			const U64 isolated = _blsi_u64(blackBishops);
			ret -= openingBishopTableBlack[bitscan(isolated)];
			blackBishops = _blsr_u64(blackBishops);
		}

		U64 whiteRooks = bitboards[R];
		while (whiteRooks) {
			const U64 isolated = _blsi_u64(whiteRooks);
			ret += openingRookTableWhite[bitscan(isolated)];
			whiteRooks = _blsr_u64(whiteRooks);
		}
		U64 blackRooks = bitboards[r];
		while (blackRooks) {
			const U64 isolated = _blsi_u64(blackRooks);
			ret -= openingQueenTableBlack[bitscan(isolated)];
			blackRooks = _blsr_u64(blackRooks);
		}

		U64 whiteQueens = bitboards[Q];
		while (whiteQueens) {
			const U64 isolated = _blsi_u64(whiteQueens);
			ret += openingQueenTableWhite[bitscan(isolated)];
			whiteQueens = _blsr_u64(whiteQueens);
		}
		U64 blackQueens = bitboards[q];
		while (blackQueens) {
			const U64 isolated = _blsi_u64(blackQueens);
			ret -= openingQueenTableBlack[bitscan(isolated)];
			blackQueens = _blsr_u64(blackQueens);
		}
		return ret;
	}
public:
	std::array<U64, 12> bitboards; // P, N, B, R, Q, K, p, n, b, r, q, k
	std::array<U64, 3> occupancies;
	std::array<short, 64> square_board;
	bool side;//white: false, black: true
	int ply;
	short enpassant_square;
	short castling_rights;//wk,wq,bk,bq
	short no_pawns_or_captures;
	std::vector<unsigned int> move_history;
	std::vector<short> enpassant_history;
	std::vector<short> castling_rights_history;
	std::vector<short> no_pawns_or_captures_history;
	std::vector<U64> hash_history;
	std::unordered_map<U64, short> pawn_evaluation_map;
	const static short infinity = 32767 - 1;//one less than max(short)
	const static short no_piece = 15;
	U64 current_hash;
	Position();
	Position(const std::string& fen);
	void parse_fen(std::string fen);
	std::string fen();
	void print();
	void print_square_board() const;
	std::string to_string();
	std::string square_board_to_string() const;
	U64 get_hash() const;
	constexpr bool get_side() const { return side; };
	constexpr int get_ply() const { return ply; };
	inline void update_hash(const unsigned int move);
	inline void make_move(const unsigned int move);
	inline void unmake_move();
	inline void make_nullmove() {
		move_history.push_back(0);
		enpassant_history.push_back(enpassant_square);
		castling_rights_history.push_back(castling_rights);
		no_pawns_or_captures_history.push_back(no_pawns_or_captures);
		hash_history.push_back(current_hash);
		no_pawns_or_captures++;
		ply++;
		current_hash = get_hash();
		enpassant_square = a8;
		side = !side;
	}
	inline void unmake_nullmove() {
		no_pawns_or_captures = no_pawns_or_captures_history.back();
		no_pawns_or_captures_history.pop_back();
		enpassant_square = enpassant_history.back();
		enpassant_history.pop_back();
		castling_rights = castling_rights_history.back();
		castling_rights_history.pop_back();
		current_hash = hash_history.back();
		hash_history.pop_back();
		move_history.pop_back();
		ply--;
		side = !side;
	}
	int get_legal_moves(std::array<unsigned int,128>& ret);
	int get_legal_captures(std::array<unsigned int,128>& ret);
	constexpr bool is_draw_by_repetition() {
		return (std::count(hash_history.begin(), hash_history.end(), current_hash)) >= 3;
	}
	constexpr bool is_draw_by_fifty_moves() {
		return no_pawns_or_captures >= 50;
	}
	inline bool currently_in_check() {
		return is_attacked_by_side(bitscan(bitboards[5 + (side) * 6]), !side);
	}
	inline int get_phase() {
		const short PawnPhase = 0;
		const short KnightPhase = 1;
		const short BishopPhase = 1;
		const short RookPhase = 2;
		const short QueenPhase = 4;
		const short TotalPhase = PawnPhase * 16 + KnightPhase * 4 + BishopPhase * 4 + RookPhase * 4 + QueenPhase * 2;

		int phase = TotalPhase;

		phase -= count_bits(bitboards[P] | bitboards[p]) * PawnPhase;
		phase -= count_bits(bitboards[N] | bitboards[n]) * KnightPhase;
		phase -= count_bits(bitboards[B] | bitboards[b]) * BishopPhase;
		phase -= count_bits(bitboards[R] | bitboards[r]) * RookPhase;
		phase -= count_bits(bitboards[Q] | bitboards[q]) * QueenPhase;

		phase = (phase * 256 + (TotalPhase / 2)) / TotalPhase;
		return phase;
	}
	inline short evaluate(const bool is_draw, const bool is_lost) {
		if (is_draw) {
			return 0;
		}
		if (is_lost) {
			return -infinity;
		}
		const int phase = get_phase();
		const int sign = (side) ? (-1) : (1);
		return sign * (raw_material(phase) + pawn_eval() + king_shield(phase) + king_attack_zones() + knight_mobility() + bad_bishop() + trapped());
	}
	inline U64 get_pawn_hash() {
		U64 ret = 0ULL;

		U64 board = bitboards[P];
		while (board) {
			U64 isolated = _blsi_u64(board);
			ret ^= keys[bitscan(isolated) + P * 64];
			board = _blsr_u64(board);
		}
		board = bitboards[p];
		while (board) {
			U64 isolated = _blsi_u64(board);
			ret ^= keys[bitscan(isolated) + p * 64];
			board = _blsr_u64(board);
		}
		return ret;
	}
	inline short pawn_eval() {
		const U64 pawn_hash = get_pawn_hash();
		auto yield = pawn_evaluation_map.find(pawn_hash);
		if (yield != pawn_evaluation_map.end()) {
			return yield->second;
		}
		const short eval = doubledPawns() + pawn_structure();
		pawn_evaluation_map[pawn_hash]=eval;
		return eval;
	}
	inline short doubledPawns() {
		short ret = 0;
		U64 whitePawns = bitboards[P];
		while (whitePawns) {
			const U64 isolated = _blsi_u64(whitePawns);
			const int sq = bitscan(isolated);
			ret -= count_bits(whitePawns & doubled_pawn_masks[sq]);
			whitePawns = whitePawns & (~doubled_pawn_reset_masks[sq]);
		}
		U64 blackPawns = bitboards[p];
		while (blackPawns) {
			const U64 isolated = _blsi_u64(blackPawns);
			const int sq = bitscan(isolated);
			ret += count_bits(blackPawns & doubled_pawn_masks[sq]);
			blackPawns = blackPawns & (~doubled_pawn_reset_masks[sq]);
		}
		return 10 * ret;
	}
	inline short pawn_structure() {
		U64 whiteStops = bitboards[P] >> 8;
		U64 blackStops = bitboards[p] << 8;
		U64	blackAttacks = ((bitboards[p] << 7) & notHFile) | ((bitboards[p] << 9) & notAFile);
		U64 whiteAttacks = ((bitboards[P] >> 7) & notAFile) | ((bitboards[P] >> 9) & notHFile);
		U64 whiteAttackSpans = whiteAttacks | (whiteAttacks >> 8);
		whiteAttackSpans |= whiteAttackSpans >> 8;
		whiteAttackSpans |= whiteAttackSpans >> 8;
		whiteAttackSpans |= whiteAttackSpans >> 8;
		whiteAttackSpans |= whiteAttackSpans >> 8;
		U64 blackAttackSpans = blackAttacks | (blackAttacks << 8);
		blackAttackSpans |= blackAttackSpans << 8;
		blackAttackSpans |= blackAttackSpans << 8;
		blackAttackSpans |= blackAttackSpans << 8;
		blackAttackSpans |= blackAttackSpans << 8;
		const U64 whiteBackwardPawns = (whiteStops & blackAttacks & ~whiteAttackSpans) >> 8;
		const U64 blackBackwardPawns = (blackStops & whiteAttacks & ~blackAttackSpans) << 8;

		short backwards = -count_bits(whiteBackwardPawns) + count_bits(blackBackwardPawns);
		short passed = 0;
		short isolated = 0;
		short supported = 0;
		U64 tempPawns = bitboards[P];
		while (tempPawns) {
			U64 isolated = _blsi_u64(tempPawns);
			passed += (!((bool)(passed_pawn_masks[white][bitscan(isolated)] & bitboards[p])));//passed pawns
			isolated -= ((neighbour_pawn_masks[bitscan(isolated) % 8] & bitboards[P]) == 0);//isolated pawns
			supported += count_bits(pawn_attacks[black][bitscan(isolated)] & bitboards[P]);//check if pawn is supported by pears
			tempPawns = _blsr_u64(tempPawns);
		}
		tempPawns = bitboards[p];
		while (tempPawns) {
			U64 isolated = _blsi_u64(tempPawns);
			passed -= (!((bool)(passed_pawn_masks[black][bitscan(isolated)] & bitboards[P])));//passed pawns
			isolated += ((neighbour_pawn_masks[bitscan(isolated) % 8] & bitboards[p]) == 0);//isolated pawns
			supported -= count_bits(pawn_attacks[white][bitscan(isolated)] & bitboards[p]);//check if pawn is supported by pears
			tempPawns = _blsr_u64(tempPawns);
		}
		return 30 * passed + 25 * isolated + 3 * supported + no_piece * backwards;
	}
	inline short knight_mobility() {
		U64	blackPawnAttacks = ((bitboards[p] << 7) & notHFile) | ((bitboards[p] << 9) & notAFile);
		U64 whitePawnAttacks = ((bitboards[P] >> 7) & notAFile) | ((bitboards[P] >> 9) & notHFile);
		short ret=0;
		U64 whiteKnights = bitboards[N];
		while (whiteKnights) {
			U64 isolated = _blsi_u64(whiteKnights);
			ret -= count_bits(knight_attacks[bitscan(isolated)] & blackPawnAttacks);
			whiteKnights = _blsr_u64(whiteKnights);
		}
		U64 blackKnights = bitboards[n];
		while (blackKnights) {
			U64 isolated = _blsi_u64(blackKnights);
			ret = count_bits(knight_attacks[bitscan(isolated)] & whitePawnAttacks);
			blackKnights = _blsr_u64(blackKnights);
		}
		return 2 * ret;
	}
	inline short rook_on_semi_open_file() {
		short ret = 0;
		U64 whiteRooks = bitboards[R];
		while (whiteRooks) {
			const U64 isolated = _blsi_u64(whiteRooks);
			U64 file = files[bitscan(isolated) % 8];
			ret += (bool)(file & bitboards[P]) * rooks_semi_open[count_bits(file & bitboards[R])];
			whiteRooks &= ~file;
		}
		U64 blackRooks = bitboards[r];
		while (blackRooks) {
			const U64 isolated = _blsi_u64(blackRooks);
			U64 file = files[bitscan(isolated) % 8];
			ret -= (bool)(file & bitboards[p]) * rooks_semi_open[count_bits(file & bitboards[r])];
			blackRooks &= ~file;
		}
		return ret;
	}
	inline short bad_bishop() {
		short ret = 0;
		U64 whiteBishops = bitboards[B];
		while (whiteBishops) {
			U64 isolated = _blsi_u64(whiteBishops);
			ret -= count_bits(pawn_attacks[white][bitscan(isolated)] & bitboards[P]);
			whiteBishops = _blsr_u64(whiteBishops);
		}
		U64 blackBishops = bitboards[b];
		while (blackBishops) {
			U64 isolated = _blsi_u64(blackBishops);
			ret += count_bits(pawn_attacks[black][bitscan(isolated)] & bitboards[p]);
			blackBishops = _blsr_u64(blackBishops);
		}
		return 20 * ret;
	}
	inline short trapped() {
		short minor = 0;
		const U64 black_attacks = get_attacks_by(black);
		const U64 white_attacks = get_attacks_by(white);
		U64 whiteKnights = bitboards[N];
		while (whiteKnights) {
			U64 isolated = _blsi_u64(whiteKnights);
			U64 squares = (knight_attacks[bitscan(isolated)] & ~occupancies[white]) | isolated;
			minor -= (squares & black_attacks) == squares;
			whiteKnights = _blsr_u64(whiteKnights);
		}
		U64 blackKnights = bitboards[n];
		while (blackKnights) {
			U64 isolated = _blsi_u64(blackKnights);
			U64 squares = (knight_attacks[bitscan(isolated)] & ~occupancies[black]) | isolated;
			minor += (squares & white_attacks) == squares;
			blackKnights = _blsr_u64(blackKnights);
		}
		U64 whiteBishops = bitboards[B];
		while (whiteBishops) {
			U64 isolated = _blsi_u64(whiteBishops);
			U64 squares = (get_bishop_attacks(occupancies[both], bitscan(isolated)) & ~occupancies[white]) | isolated;
			minor -= (squares & black_attacks) == squares;
			whiteBishops = _blsr_u64(whiteBishops);
		}
		U64 blackBishops = bitboards[b];
		while (blackBishops) {
			U64 isolated = _blsi_u64(blackBishops);
			U64 squares = (get_bishop_attacks(occupancies[both], bitscan(isolated)) & ~occupancies[black]) | isolated;
			minor += (squares & white_attacks) == squares;
			blackBishops = _blsr_u64(blackBishops);
		}
		short rooks = 0;
		U64 whiteRooks = bitboards[R];
		while (whiteRooks) {
			U64 isolated = _blsi_u64(whiteRooks);
			U64 squares = (get_rook_attacks(occupancies[both], bitscan(isolated)) & ~occupancies[white]) | isolated;
			rooks -= (squares & black_attacks) == squares;
			whiteRooks = _blsr_u64(whiteRooks);
		}
		U64 blackRooks = bitboards[r];
		while (blackRooks) {
			U64 isolated = _blsi_u64(blackRooks);
			U64 squares = (get_rook_attacks(occupancies[both], bitscan(isolated)) & ~occupancies[black]) | isolated;
			rooks += (squares & white_attacks) == squares;
			blackRooks = _blsr_u64(blackRooks);
		}
		short queens = 0;
		U64 whiteQueens = bitboards[Q];
		while (whiteQueens) {
			U64 isolated = _blsi_u64(whiteQueens);
			int sq = bitscan(isolated);
			U64 squares = ((get_rook_attacks(occupancies[both], sq) | get_bishop_attacks(occupancies[both], sq)) & ~occupancies[white]) | isolated;
			queens -= (squares & black_attacks) == squares;
			whiteQueens = _blsr_u64(whiteQueens);
		}
		U64 blackQueens = bitboards[q];
		while (blackQueens) {
			U64 isolated = _blsi_u64(blackQueens);
			int sq = bitscan(isolated);
			U64 squares = ((get_rook_attacks(occupancies[both], sq) | get_bishop_attacks(occupancies[both],sq)) & ~occupancies[black]) | isolated;
			queens += (squares & white_attacks) == squares;
			blackQueens = _blsr_u64(blackQueens);
		}
		return 150 * minor + 350  * rooks + 750 * queens;
	}
	inline short king_attack_zones() {
		U64 black_king_zone = blackKingZones[bitscan(bitboards[k])];

		short attackersOnWhite = 0;
		int table_index = 0;
		U64 tempKnights = bitboards[N];
		while (tempKnights) {
			U64 isolated = _blsi_u64(tempKnights);
			int attacks = count_bits(knight_attacks[bitscan(isolated)] & black_king_zone);
			table_index += 2 * attacks;
			attackersOnWhite += (attacks != 0);
			tempKnights = _blsr_u64(tempKnights);
		}
		U64 tempBishops = bitboards[B];
		while (tempBishops) {
			U64 isolated = _blsi_u64(tempBishops);
			int attacks = count_bits(get_bishop_attacks(occupancies[both], bitscan(isolated)) & black_king_zone);
			table_index += 2 * attacks;
			attackersOnWhite += (attacks != 0);
			tempBishops = _blsr_u64(tempBishops);
		}
		U64 tempRooks = bitboards[R];
		while (tempRooks) {
			U64 isolated = _blsi_u64(tempRooks);
			int attacks = count_bits(get_bishop_attacks(occupancies[both], bitscan(isolated)) & black_king_zone);
			table_index += 3 * attacks;
			attackersOnWhite += (attacks != 0);
			tempRooks = _blsr_u64(tempRooks);
		}
		U64 tempQueens = bitboards[Q];
		while (tempQueens) {
			U64 isolated = _blsi_u64(tempQueens);
			const int ind = bitscan(isolated);
			int attacks = count_bits((get_bishop_attacks(occupancies[both], ind) | get_rook_attacks(occupancies[both], ind)) & black_king_zone);
			table_index += 5 * attacks;
			attackersOnWhite += (attacks != 0);
			tempQueens = _blsr_u64(tempQueens);
		}

		int ret = SafetyTable[table_index] * (attackersOnWhite>2);

		short attackersOnBlack = 0;
		U64 white_king_zone = whiteKingZones[bitscan(bitboards[K])];
		table_index = 0;
		tempKnights = bitboards[n];
		while (tempKnights) {
			U64 isolated = _blsi_u64(tempKnights);
			int attacks = count_bits(knight_attacks[bitscan(isolated)] & white_king_zone);
			table_index += 2 * attacks;
			attackersOnBlack += (attacks != 0);
			tempKnights = _blsr_u64(tempKnights);
		}
		tempBishops = bitboards[b];
		while (tempBishops) {
			U64 isolated = _blsi_u64(tempBishops);
			int attacks = count_bits(get_bishop_attacks(occupancies[both], bitscan(isolated)) & white_king_zone);
			table_index += 2 * attacks;
			attackersOnBlack += (attacks != 0);
			tempBishops = _blsr_u64(tempBishops);
		}
		tempRooks = bitboards[r];
		while (tempRooks) {
			U64 isolated = _blsi_u64(tempRooks);
			int attacks = count_bits(get_rook_attacks(occupancies[both], bitscan(isolated)) & white_king_zone);
			table_index += 3 * attacks;
			attackersOnBlack += (attacks != 0);
			tempRooks = _blsr_u64(tempRooks);
		}
		tempQueens = bitboards[q];
		while (tempQueens) {
			U64 isolated = _blsi_u64(tempQueens);
			const int ind = bitscan(isolated);
			int attacks = count_bits((get_bishop_attacks(occupancies[both], ind) | get_rook_attacks(occupancies[both], ind)) & white_king_zone);
			table_index += 5 * attacks;
			attackersOnBlack += (attacks != 0);
			tempQueens = _blsr_u64(tempQueens);
		}

		return ret - SafetyTable[table_index] * (attackersOnBlack>2);
	}
	inline short king_shield(const short phase) {
		short ret = 0;
		const U64 bPawns = bitboards[p];
		const U64 bKing = bitboards[k];
		if (bKing & bKingposABCPawnShield) {
			short to_key = ((bPawns >> 8) & 7ULL) | (((bPawns >> 16) & 7ULL) << 3);
			auto yield = pawn_shield.find(to_key);
			if (yield != pawn_shield.end()) {
				ret -= yield->second;
			}
		}
		else if (bKing & bKingposFGHPawnShield) {
			short to_key = (short)(((bPawns >> 13) & 7ULL) | ((bPawns >> 18) & (7ULL << 3)));
			to_key = (to_key & 9) << 2 | (to_key & (9 << 1)) | (to_key & (9 << 2)) >> 2;
			auto yield = pawn_shield.find(to_key);
			if (yield != pawn_shield.end()) {
				ret -= yield->second;
			}
		}
		const U64 wPawns = bitboards[P];
		const U64 wKing = bitboards[K];
		if (wKing & wKingposFGHPawnShield) {
			short to_key = (short)(((wPawns >> 53) & 7ULL) | ((wPawns >> 42) & (7ULL << 3)));
			to_key = (to_key & 9) << 2 | (to_key & (9 << 1)) | (to_key & (9 << 2)) >> 2;
			auto yield = pawn_shield.find(to_key);
			if (yield != pawn_shield.end()) {
				ret += yield->second;
			}
		}
		else if (wKing & wKingposABCPawnShield) {
			short to_key = (wPawns >> 37) & (7ULL << 3) | (wPawns >> 48) & 7ULL;
			auto yield = pawn_shield.find(to_key);
			if (yield != pawn_shield.end()) {
				ret += yield->second;
			}
		}
		return (ret * (256 - phase)) / 256;
	}
	inline int get_kind_of_piece_on(const int sq) {
		bool found_piece;
		int piece_type;
		bool bit;
		for (int ind = 0; ind < 12; ind++) {
			bit = get_bit(bitboards[ind], sq);
			found_piece = bit;
			piece_type = (bit)*ind;
			if (bit) { break; }
		}
		const bool is_enpassant = (sq == enpassant_square);
		return no_piece * (!(found_piece || is_enpassant)) + (found_piece)*piece_type;
	}
	inline int get_smallest_attack(const int sq, const bool color) {
		if (get_bit(occupancies[both], sq)) {
			unsigned int move = 0;
			set_promotion_type(move, no_piece);
			set_to_square(move, sq);
			set_captured_type(move, get_piece_type_on(sq));
			set_capture_flag(move, true);
			const int offset = 6 * (color);
			U64 pot_pawns = pawn_attacks[(color)][sq] & bitboards[offset];
			if (pot_pawns) {
				set_piece_type(move, P + offset);
				set_from_square(move, bitscan(pot_pawns));
				return move;
			}
			U64 pot_knights = knight_attacks[sq] & bitboards[N + offset];
			if (pot_knights) {
				set_piece_type(move, N + offset);
				set_from_square(move, bitscan(pot_knights));
				return move;
			}
			const U64 bishop_attacks = get_bishop_attacks(occupancies[both], sq);
			U64 pot_bishops = bishop_attacks & bitboards[B + offset];
			if (pot_bishops) {
				set_piece_type(move, B + offset);
				set_from_square(move, bitscan(pot_bishops));
				return move;
			}
			const U64 rook_attacks = get_rook_attacks(occupancies[both], sq);
			U64 pot_rooks = rook_attacks & bitboards[R + offset];
			if (pot_rooks) {
				set_piece_type(move, R + offset);
				set_from_square(move, bitscan(pot_rooks));
				return move;
			}
			U64 pot_queens = (rook_attacks | bishop_attacks) & bitboards[Q + offset];
			if (pot_queens) {
				set_piece_type(move, Q + offset);
				set_from_square(move, bitscan(pot_queens));
				return move;
			}
		}
		return 0;
	}
	inline int see(const int square) {
		int value = 0;
		unsigned int move = get_smallest_attack(square, side);
		/* skip if the square isn't attacked anymore by this side */
		if (move) {
			make_move(move);
			/* Do not consider captures if they lose material, therefor max zero */
			value = std::max(-infinity, basePieceValue[basePiece[get_captured_type(move)]] - see(square));
			unmake_move();
		}
		return value;
	}
	inline bool boardsMatch() {
		for (int i = 0; i < 64; i++) {
			short type = get_piece_type_on(i);
			if (i == enpassant_square && i!=a8) {
				if (type != p && type != P && square_board[i] != no_piece) {
					std::cout << std::endl<< "mismatch at " << square_coordinates[i] << " : "<<type<<", " << square_board[i] << std::endl;
					return false;
				}
			}
			else if (type != square_board[i]) {
				std::cout << std::endl<< "mismatch at " << square_coordinates[i] << " : " << type << ", " << square_board[i] << std::endl;
				return false;
			}
		}
		return true;
	}
};
struct invalid_move_exception : std::exception {
	unsigned int move;
	std::string move_str;
	Position pos;
	invalid_move_exception(const Position t_pos, const int t_move);
	invalid_move_exception(const Position t_pos, const std::string t_move);
	const std::string what() throw();
};