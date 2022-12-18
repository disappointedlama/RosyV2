#pragma once
#include <vector>

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
int char_pieces(const char piece);

class Position {
	std::array<U64, 12> bitboards;
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
public:
	Position();
	Position(std::string fen);
	void parse_fen(std::string fen);
	void print() const;
	U64 get_hash() const;
	void update_hash();
	void make_move(const int move);
	void unmake_move();
	void make_nullmove();
	void unmake_nullmove();
};