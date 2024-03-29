#pragma once
#include <format>
#include <unordered_map>

#include "position.hpp"
#include "openingBook.hpp"
#include "logging.hpp"
#include <direct.h>
constexpr short EXACT = 0;
constexpr short UPPER = 1;
constexpr short LOWER = 2;
#define tuning false
#define timingEngine false
constexpr short Red = (short)1;
constexpr short lateMoveReduction = 2;
static constexpr int full_depth_moves = 8;
static constexpr int reduction_limit = 3;
namespace std {
	string getCurDir();
}
struct stop_exception : std::exception {
	std::string source;
	stop_exception(std::string t_source);
	const std::string what() throw();
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

static std::array<std::array<U64, 64>, 12> history = std::array<std::array<U64, 64>, 12>{};
class Engine {
#if tuning
	std::array<int, 15> gradient{};
#endif
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
	std::string logging_path = "RosyV2.botlogs";
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
	MoveWEval pv_root_call(std::array<std::array<unsigned int, 128>, 40>& moves, int move_index, const short depth, short alpha, short beta);
	short pv_search(std::array<std::array<unsigned int, 128>, 40>& moves, int move_index, const short depth, short alpha, short beta, bool isPV);
	short quiescence(std::array<std::array<unsigned int, 128>, 40>& moves, int move_index, short alpha, short beta);
	inline void order(std::array<unsigned int,128>& moves, TableEntry& entry, int number_of_moves) {
#if timingEngine
		auto start = std::chrono::steady_clock::now();
#endif
		int hash_move = 0;
		if (((bool)(entry.get_move())) && (std::find(moves.begin(), moves.end(), entry.get_move()) != moves.end())&&entry.get_move()!=0) {
			hash_move = entry.get_move();
		}
		std::sort(moves.begin(), (std::array<unsigned int, 128>::iterator)(moves.begin() + number_of_moves), [&](const int& lhs, const int& rhs)
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
	inline void quiescence_order(std::array<unsigned int, 128>& moves, int number_of_moves) {
#if timingEngine
		auto start = std::chrono::steady_clock::now();
#endif
		std::sort(moves.begin(), (std::array<unsigned int,128>::iterator)(moves.begin()+number_of_moves), [&](const int& lhs, const int& rhs)
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
	Engine() :pos{}, max_depth{ 8 }, run{ false }, debug{ false }, killer_table{}, hash_map{}, nodes{ 0ULL }, use_opening_book{ true }, log{ logging_path } {};
	Engine(const bool t_debug) :pos{}, max_depth{ 8 }, run{ false }, debug{ t_debug }, killer_table{}, hash_map{}, nodes{ 0ULL }, use_opening_book{ true }, log{ logging_path } {};
	void perft();
	void perft_traversal(std::array<std::array<unsigned int, 128>, 40>& moves, int move_index, const int depth);
	int bestMove();
	int evaluate();
	inline void printBestMove(int move);
	void set_debug(const bool t_debug);
	void set_max_depth(const short depth);
	void parse_position(std::string fen);
	void parse_go(std::string str);
	void reset_position();
	void uci_loop();
#if tuning
	int goTune() {
		//return pos.evaluate(false, false);
		max_depth = 3;
		run = true;
		return evaluate();
		std::thread time_tracker = std::thread(&Engine::track_time, this, stoull(std::string{"100"}) * 1000000ULL);
		max_depth = infinity;
		run = true;
		const int ret = evaluate();
		while (true) {
			if (time_tracker.joinable()) {
				time_tracker.join();
				break;
			}
		}
		return ret;
	}
	/*
	void tune() {
		double previous_relative_err = 100000000000;
		double total_relative_err = 100000000000;
		int iterations = 0;
		while (previous_relative_err > total_relative_err || iterations==0) {
			std::cout << "Iteration: " << ++iterations << std::endl;
			int count = 0;
			previous_relative_err = total_relative_err;
			total_relative_err = 0;
			std::ifstream file;
			file.open("C:\\Users\\Marvin Rosenplänter\\source\\repos\\RosyV2\\tuneData2.txt");
			if (file.is_open()) {
				std::string fen = "";
				int goal = 0;
				while (file >> fen >> goal) {
					if (goal == 0) continue;
					count++;
					if (count % 1000 == 0) {
						std::cout << count << "\n";
						std::cout << "average relative error so far: " << (total_relative_err / count) << std::endl;
						for (int i = 0; i < pos.wheights.size(); i++) {
							std::cout << (int)(pos.wheights[i] + (double)gradient[i] * pos.wheights[i] / count) << ",";
						}
						std::cout<<std::endl;
					}
					std::string trueFen = "";
					for (int i = 0; i < fen.length(); i++) {
						if (fen[i] != '?') {
							trueFen += fen[i];
						}
						else {
							trueFen += ' ';
						}
					}
					fen = trueFen;
					pos = Position{ fen };
					const int current = goTune();
					const int diff = std::max(goal - current, current - goal);
					double relative_error = (current) / (double)(goal);
					relative_error = std::max(relative_error, -relative_error);
					//std::cout << relative_error << std::endl;
					total_relative_err += relative_error;
					for (int i = 0; i < pos.wheights.size(); i++) {
						pos = Position{ fen };
						pos.wheights[i]++;
						hash_map = std::unordered_map<U64, TableEntry>{};
						int after_change = goTune();
						int diff_now = std::max(goal - after_change, after_change - goal);
						pos.wheights[i]--;
						if (diff_now < diff) {
							gradient[i]++;
							continue;
						}
						pos = Position{ fen };
						pos.wheights[i]--;
						hash_map = std::unordered_map<U64, TableEntry>{};
						after_change = goTune();
						diff_now = std::max(goal - after_change, after_change - goal);
						pos.wheights[i]++;
						if (diff_now < diff) {
							gradient[i]--;
							continue;
						}
					}
					//for (int i = 0; i < pos.wheights.size(); i++) {
					//	std::cout << gradient[i] << ",";
					//}
					//std::cout << std::endl;
					//if (count == 10000) break;
				}
				file.close();
			}
			std::cout << total_relative_err << std::endl;
			std::cout << "average relative error: " << (total_relative_err / count) << std::endl;
			for (int i = 0; i < pos.wheights.size(); i++) {
				pos.wheights[i] = (int)(pos.wheights[i] + (double)gradient[i] * pos.wheights[i] / count);
				std::cout << pos.wheights[i] << ",";
				gradient[i] = 0;
			}
			std::cout << std::endl;
		}
	}
	*/
	void tune() {
		double previous_relative_err = 100000000000;
		double total_relative_err = 100000000000;
		int iterations = 0;
		std::ifstream file;
		file.open("C:\\Users\\Marvin Rosenplänter\\source\\repos\\RosyV2\\tuneData2.txt");
		if (file.is_open()) {
			while (previous_relative_err > total_relative_err || iterations == 0) {
				std::cout << "Iteration: " << ++iterations << std::endl;
				int count = 0;
				previous_relative_err = total_relative_err;
				total_relative_err = 0;
				std::string fen = "";
				int goal = 0;
				while (file >> fen >> goal) {
					if (goal == 0) continue;
					count++;
					//if (count % 5 != 0) continue;
					if (count % 1000 == 0) {
						std::cout << count << "\n";
						std::cout << "average relative error so far: " << (total_relative_err / count) << std::endl;
						for (int i = 0; i < pos.wheights.size(); i++) {
							std::cout << (int)(pos.wheights[i] + (double)gradient[i] * pos.wheights[i] / count) << ",";
						}
						std::cout << std::endl;
					}
					if (count % 20000 == 0) break;
					std::string trueFen = "";
					for (int i = 0; i < fen.length(); i++) {
						if (fen[i] != '?') {
							trueFen += fen[i];
						}
						else {
							trueFen += ' ';
						}
					}
					fen = trueFen;
					pos = Position{ fen };
					const int current = goTune();
					const int diff = std::max(goal - current, current - goal);
					double relative_error = (current) / (double)(goal);
					relative_error = std::max(relative_error, -relative_error);
					//std::cout << relative_error << std::endl;
					total_relative_err += relative_error;
					for (int i = 0; i < pos.wheights.size(); i++) {
						pos = Position{ fen };
						pos.wheights[i]++;
						hash_map = std::unordered_map<U64, TableEntry>{};
						int after_change = goTune();
						int diff_now = std::max(goal - after_change, after_change - goal);
						pos.wheights[i]--;
						if (diff_now < diff) {
							gradient[i]++;
							continue;
						}
						pos = Position{ fen };
						pos.wheights[i]--;
						hash_map = std::unordered_map<U64, TableEntry>{};
						after_change = goTune();
						diff_now = std::max(goal - after_change, after_change - goal);
						pos.wheights[i]++;
						if (diff_now < diff) {
							gradient[i]--;
							continue;
						}
					}
					//for (int i = 0; i < pos.wheights.size(); i++) {
					//	std::cout << gradient[i] << ",";
					//}
					//std::cout << std::endl;
					//if (count == 20000) break;
				}
				std::cout << total_relative_err << std::endl;
				std::cout << "average relative error: " << (total_relative_err / count) << std::endl;
				for (int i = 0; i < pos.wheights.size(); i++) {
					pos.wheights[i] = (int)(pos.wheights[i] + (double)gradient[i] * pos.wheights[i] / count);
					std::cout << pos.wheights[i] << ",";
					gradient[i] = 0;
				}
				std::cout << std::endl;
			}
		}
		file.close();
	}
#endif
};