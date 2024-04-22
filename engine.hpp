#pragma once
#include <unordered_map>

#include "position.hpp"
#include "openingBook.hpp"
#include "logging.hpp"
#include<thread>
constexpr short EXACT = 0;
constexpr short UPPER = 1;
constexpr short LOWER = 2;
#define timingEngine false
constexpr short Red = (short)1;
constexpr short lateMoveReduction = 2;
static constexpr int full_depth_moves = 8;
static constexpr int reduction_limit = 3;
namespace std {
	string getCurDir();
}
struct stop_exception : std::exception {
	string source;
	stop_exception(string t_source);
	const string what() throw();
};
struct MoveWEval {
	unsigned int move;
	int eval;
	MoveWEval(const unsigned int t_move, const int t_eval) : move{ t_move }, eval{ t_eval } {}
};
struct TableEntry {
	unsigned int move_and_flag;
	unsigned int eval_and_depth;
	//28 bits are used in the move int thus two bits can be used to encode the flag
	//eval and depth can also be saved in a single int, with eval getting the first and depth getting the last 16 bits
	TableEntry() : move_and_flag { 0 }, eval_and_depth{ (int)(Position::infinity) << 16 } {}
	TableEntry(const unsigned int t_move, const short t_eval, const short t_flag, const short t_depth) : move_and_flag{ t_move | ((unsigned int)(t_flag) << 28) }, eval_and_depth{ ((unsigned int)t_eval) | ((unsigned int)t_depth << 16) } {}
	inline unsigned int get_move() const {
		return move_and_flag & 0xfffffff;
	}
	inline short get_flag() const {
		return (move_and_flag & 0xf0000000) >> 28;
	}
	inline short get_depth() const {
		return (eval_and_depth & 0xffff0000) >> 16;
	}
	inline short get_eval() const {
		return eval_and_depth & 0xffff;
	}
};
struct KillerTable {
	int table[512][3];
	void push_move(const unsigned int move, const short depth) {
		if (move == table[depth][0] || move == table[depth][2] || move == table[depth][1]) {
			return;
		}
		table[depth][2] = table[depth][1];
		table[depth][1] = table[depth][0];
		table[depth][0] = move;
	}
	bool find(const unsigned int move, const short depth) {
		return (table[depth][0] == move) || (table[depth][1] == move) || (table[depth][2] == move);
	}
	void shift_by(int shift) {
		int temp[512][3]{};
		for (int i = shift; i < 512; i++) {
			if (table[i][0] == 0 && table[i][1] == 0 && table[i][2] == 0) break;
			temp[i - shift][0] = table[i][0];
			temp[i - shift][1] = table[i][1];
			temp[i - shift][2] = table[i][2];
		}
	}
};

static array<array<U64, 64>, 12> history = array<array<U64, 64>, 12>{};
class Engine {
#if timingEngine
	U64 totalEngineTime = 0ULL;
	U64 moveGenerationTime = 0ULL;
	U64 captureGenerationTime = 0ULL;
	U64 quiescenceTime = 0ULL;
	U64 moveOrderingTime = 0ULL;
	U64 captureOrderingTime = 0ULL;
	U64 moveGenerationQuiescenceTime = 0ULL;
	U64 evaluationTime = 0ULL;
#endif
	string logging_path = "RosyV2.botlogs";
	Position pos;
	int current_desired_depth;
	int max_depth;
	std::atomic<bool> run;
	std::atomic<bool> pondering;
	bool debug;
	const static short infinity=Position::infinity;
	KillerTable killer_table;
	std::unordered_map<U64, TableEntry> hash_map;
	U64 nodes;
	U64 time_for_next_move;
	bool check_time;
	bool use_opening_book;
	Logger log;
	MoveWEval pv_root_call(array<array<unsigned int, 128>, 40>& moves, int move_index, const short depth, short alpha, short beta);
	short pv_search(array<array<unsigned int, 128>, 40>& moves, int move_index, const short depth, short alpha, short beta, bool isPV);
	short quiescence(array<array<unsigned int, 128>, 40>& moves, int move_index, short alpha, short beta);
	inline void order(array<unsigned int,128>& moves, TableEntry& entry, int number_of_moves) {
#if timingEngine
		auto start = std::chrono::steady_clock::now();
#endif
		int hash_move = 0;
		if (((bool)(entry.get_move())) && (std::find(moves.begin(), moves.end(), entry.get_move()) != moves.end())&&entry.get_move()!=0) {
			hash_move = entry.get_move();
		}
		std::sort(moves.begin(), (array<unsigned int, 128>::iterator)(moves.begin() + number_of_moves), [&](const int& lhs, const int& rhs)
			{
				if (lhs == hash_move) { return true; }
				else if (rhs == hash_move) { return false; }
				const bool lhs_is_promotion = get_promotion_type(lhs) != pos.no_piece;
				const bool rhs_is_promotion = get_promotion_type(rhs) != pos.no_piece;
				if (lhs_is_promotion && rhs_is_promotion) {
					return get_promotion_type(lhs) > get_promotion_type(rhs);
				}
				if (lhs_is_promotion || rhs_is_promotion) {
					return lhs_is_promotion;
				}
				const int ply = pos.get_ply();
				const bool lhs_is_killer = killer_table.find(lhs, ply);
				const bool rhs_is_killer = killer_table.find(rhs, ply);
				if (lhs_is_killer && rhs_is_killer) {
					return false;
				}
				if (lhs_is_killer || rhs_is_killer) {
					return lhs_is_killer;
				}
				const bool lhs_is_capture = get_capture_flag(lhs);
				const bool rhs_is_capture = get_capture_flag(rhs);
				const int lhs_piece = get_piece_type(lhs);
				const int rhs_piece = get_piece_type(rhs);
				//assert((lhs_piecs > -1) && (lhs_piece < 12));
				//assert((lhs_piecs > -1) && (lhs_piece < 12));
				if (lhs_is_capture && rhs_is_capture) {
					return pos.seeByMove(lhs) > pos.seeByMove(rhs);
				}
				if (lhs_is_capture) {
					return pos.seeByMove(lhs) > 0;
				}
				if (rhs_is_capture) return false;
				__assume(get_to_square(lhs) < 64);
				__assume(get_to_square(rhs) < 64);
				__assume(get_to_square(lhs) > -1);
				__assume(get_to_square(rhs) > -1);
				return (history[(size_t)(lhs_piece)][(size_t)(get_to_square(lhs))] > history[(size_t)(rhs_piece)][(size_t)(get_to_square(rhs))]);
			});
#if timingEngine
		auto end = std::chrono::steady_clock::now();
		moveOrderingTime += (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif
	}
	inline void quiescence_order(array<unsigned int, 128>& moves, int number_of_moves) {
#if timingEngine
		auto start = std::chrono::steady_clock::now();
#endif
		std::sort(moves.begin(), (array<unsigned int,128>::iterator)(moves.begin()+number_of_moves), [&](const int& lhs, const int& rhs)
			{
				return pos.seeByMove(lhs)>pos.seeByMove(rhs);
			});
#if timingEngine
		auto end = std::chrono::steady_clock::now();
		captureOrderingTime += (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif
	};
	inline TableEntry lookUp();
	void print_info(const short depth, const int eval, const U64 time);
	void track_time(const U64 max_time);
public:
	Engine() :pos{}, max_depth{ 8 }, run{ false }, debug{ false }, killer_table{}, hash_map{}, nodes{ 0ULL }, use_opening_book{ true }, log{ logging_path }, time_for_next_move{ 0 } {};
	Engine(const bool t_debug) :pos{}, max_depth{ 8 }, run{ false }, debug{ t_debug }, killer_table{}, hash_map{}, nodes{ 0ULL }, use_opening_book{ true }, log{ logging_path }, time_for_next_move{ 0 } {};
	void perft();
	void perft_traversal(array<array<unsigned int, 128>, 40>& moves, int move_index, const int depth);
	int bestMove();
	int evaluate();
	inline void printBestMove(int move);
	void set_debug(const bool t_debug);
	void set_max_depth(const short depth);
	void parse_position(string fen);
	void parse_go(string str);
	void reset_position();
	void uci_loop();
};