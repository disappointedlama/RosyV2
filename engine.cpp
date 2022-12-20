#include "engine.hpp"

invalid_move_exception::invalid_move_exception(const Position t_pos, const int t_move) {
	pos = t_pos;
	move = t_move;
}
const std::string invalid_move_exception::what() throw() {
	return std::format("Found invalid move %s in position %s", uci(move), pos.fen());
}
Engine::Engine() {
	pos = Position{};
	max_depth = 8;
	run = false;
	debug = false;
	infinity = pos.infinity;
	killer_table = KillerTable{};
	hash_map = std::unordered_map<U64, TableEntry>{};
	nodes = 0ULL;
}
Engine::Engine(const bool t_debug) {
	pos = Position{};
	max_depth = 8;
	run = false;
	debug = t_debug;
	infinity = pos.infinity;
	killer_table = KillerTable{};
	hash_map = std::unordered_map<U64, TableEntry>{};
	nodes = 0ULL;
}
int Engine::bestMove() {
	const int aspiration_window = 200;
	std::vector<int> moves{};
	pos.get_legal_moves(moves);
	MoveWEval best{ moves[0],0 };
	MoveWEval old_best = best;
	int alpha = -infinity;
	int beta = infinity;
	for (current_desired_depth = 1; current_desired_depth < max_depth + 1; current_desired_depth++) {
		nodes = 0;
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		MoveWEval result = pv_root_call(current_desired_depth,alpha,beta);
		const bool fell_outside_window = (result.eval <= alpha) || (result.eval >= beta);
		if (fell_outside_window) {
			nodes = 0;
			result = pv_root_call(current_desired_depth, -infinity, infinity);
		}
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		const U64 time = (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
		print_info(current_desired_depth, result.eval, time);
		old_best = best;
		best = result;
		alpha = result.eval - aspiration_window;
		beta = result.eval + aspiration_window;
		if (result.eval == infinity) {
			std::cout << "\nbestmove " << uci(best.move) << std::endl;
			return result.eval;
		}
		if (result.eval == -infinity) {
			std::cout << "\nbestmove " << uci(old_best.move) << std::endl;
			return old_best.move;
		}
	}
	if (best.move == -1) {
		throw invalid_move_exception(pos, best.move);
	}
	std::cout << "\nbestmove: " << uci(best.move) << std::endl;
}
MoveWEval Engine::pv_root_call(const int depth, int alpha, int beta) {
	TableEntry entry = lookUp();
	std::vector<int> moves{};
	pos.get_legal_moves(moves);
	order(moves, entry);
	const int number_of_moves = moves.size();
	int current_best_move = -1;
	int current_best_eval = -infinity-1;
	for (int i = 0; i < number_of_moves; i++) {
		pos.make_move(moves[i]);
		int value = -pv_search(depth - 1, -beta, -alpha);
		pos.unmake_move();
		const bool is_best_move = (value > current_best_eval);
		current_best_move = (is_best_move)*moves[i] + (!is_best_move) * current_best_move;
		current_best_eval = (is_best_move)*value + (!is_best_move) * current_best_eval;
		if (debug) {
			if (depth == max_depth) {
			}
			if (is_best_move) {
				std::cout << "Found new best move in: " << uci(moves[i]) << " (" << value << ")" << "\n";
			}
			else {
				std::cout << "finished " << uci(moves[i]) << "\n";
			}
		}
		if (value > alpha) {
			alpha = value;
			if (value >= infinity) {
				hash_map[pos.current_hash] = TableEntry{ current_best_move,current_best_eval,EXACT,depth };
				return MoveWEval{current_best_move, current_best_eval};
			}
		}
	}
	hash_map[pos.current_hash] = TableEntry{ current_best_move, current_best_eval, EXACT, depth };
	return MoveWEval{ current_best_move, current_best_eval };
}
int Engine::pv_search(const int depth, int alpha, int beta) {
	nodes++;
	const int alphaOrigin = alpha;
	TableEntry entry = lookUp();

	if ((entry.depth > depth)) {
		if (entry.flag == EXACT) {
			if (entry.eval > alpha) {
			}
			return entry.eval;
		}
		else if ((entry.flag == LOWER) && (entry.eval > alpha)) {
			alpha = entry.eval;
		}
		else if (entry.flag == UPPER) {
			beta = std::min(beta, entry.eval);
		}
		if (alpha >= beta) {
			return entry.eval;
		}
	}

	std::vector<int> moves{};
	pos.get_legal_moves(moves);
	const int number_of_moves = moves.size();

	const bool draw_by_repetition = pos.is_draw_by_repetition();
	const bool draw_by_fifty_move_rule = pos.is_draw_by_fifty_moves();
	const bool in_check = pos.currently_in_check();
	const bool no_moves_left = number_of_moves == 0;
	const bool draw_by_stalemate = no_moves_left && (!in_check);
	const bool is_draw = draw_by_fifty_move_rule || draw_by_repetition || draw_by_stalemate;
	const bool is_lost = no_moves_left && in_check;

	if (is_draw || is_lost) {
		const int eval = pos.evaluate(is_draw, is_lost);
		hash_map[pos.current_hash] = TableEntry{ 0,eval,EXACT,depth };
		return eval;
	}
	if (depth == 0) {
		return quiescence(alpha, beta);
	}
	order(moves, entry);
	int current_best_move = -1;
	int current_best_eval = -infinity-1;
	for (int i = 0; i < number_of_moves; i++) {
		pos.make_move(moves[i]);
		int value = -pv_search(depth - 1, -beta, -alpha);
		pos.unmake_move();
		const bool is_best_move = (value > current_best_eval);
		current_best_move = (is_best_move)*moves[i] + (!is_best_move) * current_best_move;
		current_best_eval = (is_best_move)*value + (!is_best_move) * current_best_eval;
		if (value > alpha) {
			if (value >= beta) {
				const bool not_capture = !get_capture_flag(moves[0]);
				if (not_capture) {
					killer_table.push_move(moves[0], pos.get_ply());
					history[pos.get_side()][(int)get_piece_type(moves[0])][(int)get_to_square(moves[0])] += (!get_capture_flag(moves[0])) * (unsigned long long)(2<<depth);
				}
				if (depth >= entry.depth) {
					hash_map[pos.current_hash] = TableEntry{ moves[i],value,LOWER,depth };
				}
				return beta;
			}
			alpha = value;
		}
	}
	if (depth >= entry.depth) {
		hash_map[pos.current_hash] = TableEntry{ current_best_move, alpha, (current_best_eval <= alphaOrigin), depth };
	}
	return alpha;
}
int Engine::quiescence(int alpha, int beta) {
	nodes++;
	const int alphaOrigin = alpha;
	std::vector<int> captures{};
	pos.get_legal_captures(captures);
	const int number_of_captures = captures.size();

	const bool draw_by_repetition = pos.is_draw_by_repetition();
	const bool draw_by_fifty_move_rule = pos.is_draw_by_fifty_moves();
	bool is_draw = draw_by_fifty_move_rule || draw_by_repetition;

	if (is_draw) {
		const int eval = pos.evaluate(is_draw, false);
		return eval;
	}
	if (number_of_captures == 0) {
		std::vector<int> moves{};
		pos.get_legal_moves(moves);
		const int number_of_moves = (const int)moves.size();
		const bool no_moves_left = number_of_moves == 0;
		const bool in_check = pos.currently_in_check();
		const bool draw_by_stalemate = no_moves_left && (!in_check);
		is_draw = is_draw || draw_by_stalemate;
		const bool is_lost = no_moves_left && in_check;
		const int eval = pos.evaluate(is_draw, is_lost);
		return eval;
	}
	int eval = pos.evaluate(false, false);
	std::sort(captures.begin(), captures.end(), [&](const int& lhs, const int& rhs)
		{
			return (get_captured_type(lhs) - get_piece_type(lhs)) > (get_captured_type(rhs) - get_piece_type(rhs));
		});

	int current_best_move = -1;
	int current_best_eval = -1000000;
	for (int i = 0; i < number_of_captures; i++) {
		pos.make_move(captures[i]);
		const int value = -quiescence(-beta, -alpha);
		pos.unmake_move();

		const bool is_new_best = (current_best_eval < value);
		current_best_eval = (is_new_best)*value + (!is_new_best) * current_best_eval;
		current_best_move = (is_new_best)*captures[i] + (!is_new_best) * current_best_move;

		if (value > alpha) {
			if (value >= beta) {
				hash_map[pos.current_hash] = TableEntry{ captures[i], value, UPPER, 0 };
				return beta;
			}
			alpha = value;
		}
	}
	hash_map[pos.current_hash] = TableEntry{ current_best_move, alpha, (current_best_eval <= alphaOrigin) * LOWER, 0 };
	return alpha;
}
TableEntry Engine::lookUp() {
	auto yield = hash_map.find(pos.current_hash);
	if (yield != hash_map.end()) {
		return yield->second;
	}
	return TableEntry{ 0,0,0,-9999 };
}
void Engine::set_max_depth(const int depth) {
	max_depth = depth;
}
void Engine::set_debug(const bool t_debug) {
	debug = t_debug;
}
void Engine::set_position(std::string fen) {
	pos = Position(fen);
}
void Engine::print_info(const int depth, const int eval, const U64 time) {
	std::cout << "info score cp " << eval << " depth " << depth << " nodes " << nodes << " nps " << 1000000000 * nodes / time << " pv ";
	int j = 0;
	for (int i = 0; i < depth; i++) {
		TableEntry entry = lookUp();
		//if (entry.flag != EXACT) break;
		j++;
		std::cout << uci(entry.move) << " ";
		pos.make_move(entry.move);
	}
	for (int i = 0; i < j; i++) {
		pos.unmake_move();
	}
	std::cout << std::endl;
}