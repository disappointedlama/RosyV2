#include "engine.hpp"

invalid_move_exception::invalid_move_exception(const Position t_pos, const int t_move) {
	pos = t_pos;
	move = t_move;
	move_str = uci(t_move);
}
invalid_move_exception::invalid_move_exception(const Position t_pos, const std::string t_move) {
	pos = t_pos;
	move = -1;
	move_str = t_move;
}
const std::string invalid_move_exception::what() throw() {
	return std::format("Found invalid move {} in position {}", move_str, pos.fen());
}
stop_exception::stop_exception(std::string t_source) {
	source = t_source;
}
const std::string stop_exception::what() {
	return "Stop exception thrown by " + source;
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
	try {
		for (current_desired_depth = 1; current_desired_depth < max_depth + 1; current_desired_depth++) {
			nodes = 0;
			std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
			MoveWEval result = pv_root_call(current_desired_depth, alpha, beta);
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
				run = false;
				return result.eval;
			}
			if (result.eval == -infinity) {
				std::cout << "\nbestmove " << uci(old_best.move) << std::endl;
				run = false;
				return old_best.move;
			}
		}
	}
	catch (stop_exception e) {
		std::cout << "caught \"" << e.what() << "\"" << std::endl;
	}
	reset_position();
	if (best.move == -1) {
		throw invalid_move_exception(pos, best.move);
	}
	std::cout << "\nbestmove: " << uci(best.move) << std::endl;
	run = false;
	return best.move;
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
	if (!run) {
		throw stop_exception("pv search");
	}
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
					history[pos.get_side()][(int)get_piece_type(moves[0])][(int)get_to_square(moves[0])] += (!get_capture_flag(moves[0])) * (2ULL<<depth);
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
	if (!run) {
		throw stop_exception("pv search");
	}
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
	if (eval < alpha - 950) {
		return alpha;
	}
	if (eval > alpha) {
		if (eval >= beta) {
			return eval;
		}
		alpha = eval;
	}
	std::sort(captures.begin(), captures.end(), [&](const int& lhs, const int& rhs)
		{
			return (get_captured_type(lhs) - get_piece_type(lhs)) > (get_captured_type(rhs) - get_piece_type(rhs));
		});

	int current_best_move = -1;
	int current_best_eval = -infinity - 1;
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
inline TableEntry Engine::lookUp() {
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
void Engine::parse_position(std::string fen) {
	std::string moves="";
	std::string str = " moves ";
	auto substr_pos = fen.find(str);
	if (substr_pos != std::string::npos) {
		moves = fen.substr(substr_pos + str.size(), fen.size());
		fen = fen.substr(0, substr_pos);
	}
	str = "startpos";
	substr_pos = fen.find(str);
	if (substr_pos != std::string::npos) {
		fen = start_position;
	}
	str = "fen ";
	substr_pos = fen.find(str);
	if (substr_pos != std::string::npos) {
		fen = fen.substr(substr_pos + str.size(), fen.size());
	}
	pos = Position(fen);
	try {
		while (moves != "") {
			std::vector<int> move_list{};
			pos.get_legal_moves(move_list);
			std::string move_string = moves.substr(0, moves.find_first_of(' '));
			if (move_string.size() > 4) {
				const int last = move_string.size() - 1;
				if ((move_string[last] != 'n') && (move_string[last] != 'b') && (move_string[last] != 'r') && (move_string[last] != 'q')) {
					move_string = move_string.substr(0, last);
				}
			}
			bool matching_move_found = false;
			for (int i = 0; i < move_list.size(); i++) {
				if (uci(move_list[i]) == move_string) {
					pos.make_move(move_list[i]);
					matching_move_found = true;
					break;
				}
			}
			if (!matching_move_found) {
				throw invalid_move_exception(pos, move_string);
			}
			if (move_string.size() + 1 < moves.size()) {
				moves = moves.substr(move_string.size() + 1, moves.size());
			}
			else { moves = ""; }
		}
	}
	catch (invalid_move_exception e) {
		std::cout << e.what() << std::endl;
	}
	pos.print();
	run = false;
}
void Engine::reset_position() {
	pos = Position{ start_position };
}
void Engine::parse_go(std::string str){
	std::string command = "depth ";
	auto substr_pos = str.find(command);
	if (substr_pos != std::string::npos) {
		str = str.substr(substr_pos + command.size(), str.size());
		max_depth = stoi(str);
		bestMove();
		return;
	}
	command = "infinite";
	substr_pos = str.find(command);
	if (substr_pos != std::string::npos) {
		str = str.substr(substr_pos + command.size(), str.size());
		max_depth = infinity;
		bestMove();
		return;
	}
	command = "movetime ";
	substr_pos = str.find(command);
	if (substr_pos != std::string::npos) {
		str = str.substr(substr_pos + command.size(), str.size());
		std::thread time_tracker = std::thread(&Engine::track_time, this, stoull(str)*1000000ULL);
		max_depth = infinity;
		bestMove();
		while (true) {
			if (time_tracker.joinable()) {
				time_tracker.join();
				break;
			}
		}
		return;
	}
}
void Engine::print_info(const int depth, const int eval, const U64 time) {
	std::cout << "info score cp " << eval << " depth " << depth << " nodes " << nodes << " nps " << 1000000000 * nodes / time << " pv ";
	int j = 0;
	for (int i = 0; i < depth; i++) {
		TableEntry entry = lookUp();
		j++;
		std::cout << uci(entry.move) << " ";
		pos.make_move(entry.move);
	}
	for (int i = 0; i < j; i++) {
		pos.unmake_move();
	}
	std::cout << std::endl;
}
void Engine::track_time(const U64 max_time) {
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	while (run) {
		end = std::chrono::steady_clock::now();
		if ((U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() >= max_time) {
			std::cout << "stopping execution" << std::endl;
			run = false;
		}
	}
}
void Engine::uci_loop(){

	fflush(stdin);
	fflush(stdout);
	char input[2000];

	std::vector<std::thread> workers{};
	std::thread parse_runner{};
	run = false;
	char* command = nullptr;
	while (true) {
		memset(input, 0, sizeof(input));
		fflush(stdout);
		if (!fgets(input, 2000, stdin)) {
			continue;
		}
		if (!run) {
			if (parse_runner.joinable()) {
				parse_runner.join();
			}
		}
		/*
		if (!run) {
			for (int i = 0; i < workers.size(); i++) {
				if (workers[i].joinable()) {
					workers[i].join();
					workers.erase(workers.begin() + i);
				}
			}
			//pondering = false;
		}//if no thread is running join all threads and delete joined threads from workers
		*/
		if (input[0] == '\n') {
			continue;
		}
		if (strncmp(input, "isready", 7) == 0) {
			std::cout << "readyok\n";
		}
		else if (strncmp(input, "position", 8) == 0) {
			run = true;
			std::cout << "Launching thread" << std::endl;
			parse_runner = std::thread(&Engine::parse_position, this, std::string{ input });
		}
		else if (strncmp(input, "ucinewgame", 10) == 0) {
			reset_position();
		}
		else if (strncmp(input, "go", 2) == 0) {
			if (!run) {
				run = true;
				parse_runner = std::thread(&Engine::parse_go, this, std::string{ input });
			}

			using namespace std::literals::chrono_literals;
			std::this_thread::sleep_for(500us);
		}
		else if (strncmp(input, "quit", 4) == 0) {
			//pondering = false;
			run = false;
			if (parse_runner.joinable()) {
				parse_runner.join();
			}
			/*
			run = false;
			for (int i = 0; i < workers.size(); i++) {
				if (workers[i].joinable()) {
					workers[i].join();
					workers.erase(workers.begin() + i);
				}
			}
			*/
			break;
		}
		else if (strncmp(input, "stop", 4) == 0) {
			/*
			if (pondering) {
				pondering = false;
				run = false;
				for (int i = 0; i < workers.size(); i++) {
					if (workers[i].joinable()) {
						workers[i].join();
						workers.erase(workers.begin() + i);
					}
				}
				if (!run) {
					run = true;
					workers.push_back(std::thread(&Engine::parse_go, this, (char*)("go depth 7"), &run));
				}

				using namespace std::literals::chrono_literals;
				std::this_thread::sleep_for(500us);
			}
			else {
			*/
				run = false;
				if (parse_runner.joinable()) {
					parse_runner.join();
				}
				/*
				for (int i = 0; i < workers.size(); i++) {
					if (workers[i].joinable()) {
						workers[i].join();
						workers.erase(workers.begin() + i);
					}
				}
				*/
				reset_position();
				//pos = Position{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };
			//}
		}
		else if (strncmp(input, "uci", 3) == 0) {
			std::cout << "id name Rosy author disappointed_lama\n";
			std::cout << "option name Move Overhead type spin default 100 min 0 max 20000\noption name Threads type spin default 2 min 2 max 2\noption name Hash type spin default 512 min 256" << std::endl;
			std::cout << "uciok\n";
		}
		/*
		else if (strncmp(input, "ponderhit", 9) == 0) {
			run = false;
			for (int i = 0; i < workers.size(); i++) {
				if (workers[i].joinable()) {
					workers[i].join();
					workers.erase(workers.begin() + i);
				}
			}
			if (!run) {
				run = true;
				workers.push_back(std::thread(&Engine::parse_go, this, input, &run));
			}
			using namespace std::literals::chrono_literals;
			std::this_thread::sleep_for(500us);
		}
		*/
	}
}