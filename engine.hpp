#pragma once
#include <format>
#include <unordered_map>

#include "position.hpp"
constexpr short EXACT = 0;
constexpr short UPPER = 1;
constexpr short LOWER = 2;
struct invalid_move_exception : std::exception {
	int move;
	std::string move_str;
	Position pos;
	invalid_move_exception(const Position t_pos, const int t_move);
	invalid_move_exception(const Position t_pos, const std::string t_move);
	const std::string what() throw();
};
struct stop_exception : std::exception {
	std::string source;
	stop_exception(std::string t_source);
	const std::string what() throw();
};
struct MoveWEval {
	int move;
	int eval;
	MoveWEval(const int t_move, const int t_eval) {
		move = t_move;
		eval = t_eval;
	}
};
struct TableEntry {
	int move;
	int eval;
	int flag;
	int depth;
	TableEntry() {
		move = 0;
		eval = 0;
		flag = 0;
		depth = -9999;
	}
	TableEntry(const int t_move, const int t_eval, const int t_flag, const int t_depth) {
		move = t_move;
		eval = t_eval;
		flag = t_flag;
		depth = t_depth;
	}
};
struct KillerTable {
	int table[512][3];
	void push_move(const int move, const int depth) {
		table[depth][2] = table[depth][1];
		table[depth][1] = table[depth][0];
		table[depth][0] = move;
	}
	bool find(const int move, const int depth) {
		return (table[depth][0] == move) || (table[depth][1] == move) || (table[depth][2] == move);
	}
};

static U64 history[2][12][64] = { {
	{0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0},
	{ 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0 }
},{
	{0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0},
	{ 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0 }
} };
class Engine {
	Position pos;
	int current_desired_depth;
	int max_depth;
	std::atomic<bool> run;
	std::atomic<bool> pondering;
	bool debug;
	int infinity;
	KillerTable killer_table;
	std::unordered_map<U64, TableEntry> hash_map;
	U64 nodes;
	MoveWEval pv_root_call(std::array<std::array<int, 128>, 40>& moves, int move_index, const int depth, int alpha, int beta);
	int pv_search(std::array<std::array<int, 128>, 40>& moves, int move_index, const int depth, int alpha, int beta);
	int quiescence(std::array<std::array<int, 128>, 40>& moves, int move_index, int alpha, int beta);
	inline void order(std::array<int,128>& moves, TableEntry& entry, int number_of_moves) {
		int hash_move = 0;
		if (((bool)(entry.move)) && (std::find(moves.begin(), moves.end(), entry.move) != moves.end())) {
			hash_move = entry.move;
		}
		std::sort(moves.begin(), (std::array<int, 128>::iterator)(moves.begin() + number_of_moves), [&](const int& lhs, const int& rhs)
			{
				if (lhs == hash_move) { return true; }
				else if (rhs == hash_move) { return false; }
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
				if (lhs_is_capture && rhs_is_capture) {
					return (get_captured_type(lhs) - lhs_piece) > (get_captured_type(rhs) - rhs_piece);
				}
				if (lhs_is_capture || rhs_is_capture) {
					return lhs_is_capture;
				}
				const size_t index = (size_t)pos.get_side();
				__assume(get_to_square(lhs) < 64);
				__assume(get_to_square(rhs) < 64);
				__assume(get_to_square(lhs) > -1);
				__assume(get_to_square(rhs) > -1);
				return (history[index][(size_t)(lhs_piece)][(size_t)(get_to_square(lhs))] > history[index][(size_t)(rhs_piece)][(size_t)(get_to_square(rhs))]);
			});
	}
	inline void quiescence_order(std::array<int, 128>& moves, int number_of_moves) {
		std::sort(moves.begin(), (std::array<int,128>::iterator)(moves.begin()+number_of_moves), [&](const int& lhs, const int& rhs)
			{
				return (get_captured_type(lhs) - get_piece_type(lhs)) > (get_captured_type(rhs) - get_piece_type(rhs));
			});
	};
	inline TableEntry lookUp();
	void print_info(const int depth, const int eval, const U64 time);
	void track_time(const U64 max_time);
public:
	Engine();
	Engine(const bool t_debug);
	int bestMove();
	void set_debug(const bool t_debug);
	void set_max_depth(const int depth);
	void parse_position(std::string fen);
	void parse_go(std::string str);
	void reset_position();
	void uci_loop();
};