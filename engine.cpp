#include "engine.hpp"
namespace std {
	string getCurDir() {
		char buff[512];
		_getcwd(buff, 512);
		string current_working_dir(buff);
		return current_working_dir;
	}
}
OpeningBook book{ (std::getCurDir() + std::string("\\engines\\openingBook.txt")).c_str() };

stop_exception::stop_exception(std::string t_source) {
	source = t_source;
}
const std::string stop_exception::what() throw(){
	return "Stop exception thrown by " + source;
}

Engine::Engine() {
	pos = Position{};
	max_depth = 8;
	run = false;
	debug = false;
	killer_table = KillerTable{};
	hash_map = std::unordered_map<U64, TableEntry>{};
	nodes = 0ULL;
	use_opening_book = true;
	log = Logger{ logging_path };
}
Engine::Engine(const bool t_debug) {
	pos = Position{};
	max_depth = 8;
	run = false;
	debug = t_debug;
	killer_table = KillerTable{};
	hash_map = std::unordered_map<U64, TableEntry>{};
	nodes = 0ULL;
	use_opening_book = true;
	log = Logger{ logging_path };
}
int Engine::bestMove() {
	if (use_opening_book) {
		const int move = book.find_move(pos.current_hash);
		if (move) {
			std::cout << "bestmove " << uci(move) << std::endl;
			run = false;
			pos = Position{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };
			return move;
		}
	}
	std::chrono::steady_clock::time_point search_start = std::chrono::steady_clock::now();
	killer_table.shift_by(2);
	const int aspiration_window = 200;
	std::array<std::array<unsigned int,128>,40> moves{};
	const int number_of_legal_moves = pos.get_legal_moves(moves[0]);
	MoveWEval best{ moves[0][0],0};
	MoveWEval old_best = best;
	short alpha = -infinity;
	short beta = infinity;
	try {
		for (current_desired_depth = 1; current_desired_depth < max_depth + 1; current_desired_depth++) {
			nodes = 0;
			std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
			MoveWEval result = pv_root_call(moves,0, current_desired_depth, alpha, beta);
			const bool fell_outside_window = (result.eval <= alpha) || (result.eval >= beta);
			if (fell_outside_window && run) {
				nodes = 0;
				result = pv_root_call(moves,0, current_desired_depth, -infinity, infinity);
			}
			std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
			const U64 time = (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
			old_best = best;
			best = result;
			if (!run) {
				if (best.move == 0) best = old_best;
				break;
			}
			print_info(current_desired_depth, result.eval, time);
			if (check_time) {
				const U64 total_time_searched = (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - search_start).count();
				if (total_time_searched * 2 > time_for_next_move) {
					break;
				}
			}
			alpha = result.eval - aspiration_window;
			beta = result.eval + aspiration_window;
			if (result.eval == infinity) {
				printBestMove(best.move);
				run = false;
				return result.eval;
			}
			if (result.eval == -infinity) {
				printBestMove(old_best.move);
				run = false;
				return old_best.move;
			}
		}
	}
	catch (Position_Error e) {
		log.error(e.what());
	}
	reset_position();
	bool found = false;
	for (int i = 0; i < number_of_legal_moves; i++) {
		if (moves[0][i] == best.move) {
			found = true;
			break;
		}
	}
	if (!found) {
		log.log(pos.to_string() + "invalid move encountered " + move_to_string(best.move));
		throw invalid_move_exception{pos, best.move};
	}
	printBestMove(best.move);
	run = false;
	return best.move;
}
int Engine::evaluate() {
	std::chrono::steady_clock::time_point search_start = std::chrono::steady_clock::now();
	killer_table.shift_by(2);
	const int aspiration_window = 200;
	std::array<std::array<unsigned int, 128>, 40> moves{};
	const int number_of_legal_moves = pos.get_legal_moves(moves[0]);
	MoveWEval best{ moves[0][0],0 };
	MoveWEval old_best = best;
	short alpha = -infinity;
	short beta = infinity;
	try {
		for (current_desired_depth = 1; current_desired_depth < max_depth + 1; current_desired_depth++) {
			nodes = 0;
			std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
			MoveWEval result = pv_root_call(moves, 0, current_desired_depth, alpha, beta);
			const bool fell_outside_window = (result.eval <= alpha) || (result.eval >= beta);
			if (fell_outside_window && run) {
				nodes = 0;
				result = pv_root_call(moves, 0, current_desired_depth, -infinity, infinity);
			}
			std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
			const U64 time = (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
			old_best = best;
			best = result;
			if (!run) {
				if (best.move == 0) best = old_best;
				break;
			}
			//print_info(current_desired_depth, result.eval, time);
			if (check_time) {
				const U64 total_time_searched = (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - search_start).count();
				if (total_time_searched * 2 > time_for_next_move) {
					break;
				}
			}
			alpha = result.eval - aspiration_window;
			beta = result.eval + aspiration_window;
			if (result.eval == infinity) {
				run = false;
				return result.eval;
			}
			if (result.eval == -infinity) {
				run = false;
				return result.eval;
			}
		}
	}
	catch (Position_Error e) {
		log.error(e.what());
	}
	reset_position();
	bool found = false;
	for (int i = 0; i < number_of_legal_moves; i++) {
		if (moves[0][i] == best.move) {
			found = true;
			break;
		}
	}
	if (!found) {
		log.log(pos.to_string() + "invalid move encountered " + move_to_string(best.move));
		throw invalid_move_exception{pos, best.move};
	}
	run = false;
	return best.eval;
}
inline void Engine::printBestMove(int move) {
	//std::this_thread::sleep_for(std::chrono::milliseconds(100));
	log<< "bestmove " + uci(move);
}
MoveWEval Engine::pv_root_call(std::array<std::array<unsigned int, 128>, 40>& moves, int move_index, const short depth, short alpha, short beta) {
	TableEntry entry = lookUp();
	const int number_of_moves = pos.get_legal_moves(moves[move_index]);
	order(moves[move_index], entry, number_of_moves);
	unsigned int current_best_move = 0;
	short current_best_eval = -infinity-1;
	for (int i = 0; i < number_of_moves; i++) {
		if (debug) {
			std::cout << "starting " << uci(moves[move_index][i]) << std::endl;
		}
		short value = 0;
		try {
			pos.make_move(moves[move_index][i]);
			value = -pv_search(moves, move_index+1, depth - 1, -beta, -alpha,true);
			pos.unmake_move();
		}
		catch (stop_exception e) {
			if(debug) std::cout << "caught \"" << e.what() << "\"" << std::endl;
			return MoveWEval{ current_best_move, current_best_eval };
		}
		//if(debug){
		//	pos.print();
		//}
		const bool is_best_move = (value > current_best_eval);
		current_best_move = (is_best_move)*moves[move_index][i] + (!is_best_move) * current_best_move;
		current_best_eval = (is_best_move)*value + (!is_best_move) * current_best_eval;
		if (debug) {
			if (is_best_move) {
				std::cout << "Found new best move in: " << "\n";
				print_move(moves[move_index][i]);
				std::cout << " (" << value << ")" << std::endl;
			}
			else {
				std::cout << "finished " << uci(moves[move_index][i]) << std::endl;
			}
		}
		if (value > alpha) {
			alpha = value;
			if (value >= infinity) {
				if (debug) {
					std::cout << "Finished depth " << depth << std::endl;
				}
				hash_map[pos.current_hash] = TableEntry{ current_best_move,current_best_eval,EXACT,depth };
				return MoveWEval{current_best_move, current_best_eval};
			}
		}
	}
	if (debug) {
		std::cout << "Finished depth " << depth << std::endl;
	}
	hash_map[pos.current_hash] = TableEntry{ current_best_move, current_best_eval, EXACT, depth };
	return MoveWEval{ current_best_move, current_best_eval };
}
short Engine::pv_search(std::array<std::array<unsigned int, 128>, 40>& moves, int move_index, const short depth, short alpha, short beta, bool isPV) {
	if (!run) {
		throw stop_exception("pv search");
	}
	nodes++;
	
	const int number_of_moves = pos.get_legal_moves(moves[move_index]);

	const bool draw_by_repetition = pos.is_draw_by_repetition();
	const bool draw_by_fifty_move_rule = pos.is_draw_by_fifty_moves();
	const bool in_check = pos.currently_in_check();
	const bool no_moves_left = number_of_moves == 0;
	const bool draw_by_stalemate = no_moves_left && (!in_check);
	const bool is_draw = draw_by_fifty_move_rule || draw_by_repetition || draw_by_stalemate;
	const bool is_lost = no_moves_left && in_check;

	if (is_draw || is_lost) {
		const short eval = pos.evaluate(is_draw, is_lost);
		hash_map[pos.current_hash] = TableEntry{ 0,eval,EXACT,depth };
		return eval;
	}

	const short alphaOrigin = alpha;
	TableEntry entry = lookUp();
	if (entry.get_depth() >= depth && !isPV) {
		const short eval= entry.get_eval();
		const short flag = entry.get_flag();
		if ((flag == EXACT) || (flag == LOWER && eval>=beta) || (flag==UPPER && eval<=alpha)) {
			return eval;
		}
	}

	if (depth <= 0) {
		return quiescence(moves, move_index+1,alpha, beta);
	}
	const int phase = pos.get_phase();
	if ((!isPV) && (depth >= 1 + Red) && (!in_check) && (phase>=8)) {
		pos.make_nullmove();
		short nm_value = -pv_search(moves, move_index + 1, depth - 1 - Red, -beta, -beta + 1, false);
		pos.unmake_nullmove();
		if (nm_value >= beta) {
			if (depth >= entry.get_depth()) {
				hash_map[pos.current_hash] = TableEntry{ moves[move_index][0], nm_value, LOWER, (short)(depth - Red) };
			}
			return beta;
		}
	}
	order(moves[move_index], entry,number_of_moves);
	unsigned int current_best_move = 0;
	short current_best_eval = -infinity-1;
	//std::string before_moves = pos.fen();
	pos.make_move(moves[move_index][0]);
	bool in_check_now = pos.currently_in_check();
	short value = -pv_search(moves, move_index + 1, depth - 1, -beta, -alpha, true);
	pos.unmake_move();
	//if (pos.fen() != before_moves) {
	//	pos.print();
	//	pos.print_square_board();
	//	std::cout << "Should have been: " << before_moves << std::endl;
	//	print_move(pos.move_history.back());
	//	throw stop_exception("pv");
	//}
	const bool is_best_move = (value > current_best_eval);
	current_best_move = (is_best_move)*moves[move_index][0] + (!is_best_move) * current_best_move;
	current_best_eval = (is_best_move)*value + (!is_best_move) * current_best_eval;
	if (value > alpha) {
		if (value >= beta) {
			const bool not_capture = !get_capture_flag(moves[move_index][0]);
			if (not_capture) {
				killer_table.push_move(moves[move_index][0], pos.get_ply());
				__assume(get_piece_type(moves[move_index][0]) > -1);
				__assume(get_piece_type(moves[move_index][0]) < 12);
				__assume(get_to_square(moves[move_index][0]) > -1);
				__assume(get_to_square(moves[move_index][0]) < 64);
				history[get_piece_type(moves[move_index][0])][get_to_square(moves[move_index][0])] += 1ULL << depth;
			}
			if (depth >= entry.get_depth()) {
				hash_map[pos.current_hash] = TableEntry{ moves[move_index][0],value,LOWER,depth };
			}
			return beta;
		}
		alpha = value;
	}

	for (int i = 1; i < number_of_moves; i++) {
		pos.make_move(moves[move_index][i]);
		in_check_now = pos.currently_in_check();
		int r = (i>2 && !(in_check_now || isPV || get_capture_flag(moves[move_index][i]) || get_promotion_type(moves[move_index][i])!=pos.no_piece)) * lateMoveReduction;
		value = -pv_search(moves, move_index + 1, depth - 1 - r, -alpha - 1, -alpha, false);
		if ((value > alpha) && (value < beta)) {
			value = -pv_search(moves, move_index + 1, depth - 1, -beta, -alpha, true);
		}
		pos.unmake_move();
		//if (pos.fen() != before_moves) {
		//	pos.print();
		//	pos.print_square_board();
		//	std::cout << "Should have been: " << before_moves << std::endl;
		//	print_move(pos.move_history.back());
		//	throw stop_exception("pv");
		//}
		const bool is_best_move = (value > current_best_eval);
		current_best_move = (is_best_move)*moves[move_index][i] + (!is_best_move) * current_best_move;
		current_best_eval = (is_best_move)*value + (!is_best_move) * current_best_eval;
		if (value > alpha) {
			if (value >= beta) {
				const bool not_capture = !get_capture_flag(moves[move_index][i]);
				if (not_capture) {
					killer_table.push_move(moves[move_index][i], pos.get_ply());
					__assume(get_piece_type(moves[move_index][i]) > -1);
					__assume(get_piece_type(moves[move_index][i]) < 12);
					__assume(get_to_square(moves[move_index][i]) > -1);
					__assume(get_to_square(moves[move_index][i]) < 64);
					__assume(pos.get_side()==0 || pos.get_side()==1);
					history[get_piece_type(moves[move_index][i])][get_to_square(moves[move_index][i])] += 1ULL << depth;
				}
				if (depth >= entry.get_depth()) {
					hash_map[pos.current_hash] = TableEntry{ moves[move_index][i],value,LOWER,depth };
				}
				return beta;
			}
			alpha = value;
		}
	}
	if (depth >= entry.get_depth()) {
		hash_map[pos.current_hash] = TableEntry{ current_best_move, alpha, (current_best_eval <= alphaOrigin), depth };
	}
	return alpha;
}
short Engine::quiescence(std::array<std::array<unsigned int, 128>, 40>& moves, int move_index, short alpha, short beta) {
	if (!run) {
		throw stop_exception("pv search");
	}
	nodes++;
	const short alphaOrigin = alpha;
	const int number_of_captures = pos.get_legal_captures(moves[move_index]);

	const bool draw_by_repetition = pos.is_draw_by_repetition();
	const bool draw_by_fifty_move_rule = pos.is_draw_by_fifty_moves();
	bool is_draw = draw_by_fifty_move_rule || draw_by_repetition;

	if (is_draw) {
		const short eval = pos.evaluate(is_draw, false);
		return eval;
	}
	if (number_of_captures == 0) {
		const int number_of_moves = pos.get_legal_moves(moves[move_index]);
		const bool no_moves_left = number_of_moves == 0;
		const bool in_check = pos.currently_in_check();
		const bool draw_by_stalemate = no_moves_left && (!in_check);
		is_draw = is_draw || draw_by_stalemate;
		const bool is_lost = no_moves_left && in_check;
		const short eval = pos.evaluate(is_draw, is_lost);
		return eval;
	}
	short eval = pos.evaluate(false, false);
	if (eval < alpha - 950) {//delta pruning
		return alpha;
	}
	if (eval > alpha) {
		if (eval >= beta) {
			return eval;
		}
		alpha = eval;
	}
	quiescence_order(moves[move_index], number_of_captures);

	//std::string before_moves = pos.fen();
	unsigned int current_best_move = 0;
	short current_best_eval = -infinity - 1;
	for (int i = 0; i < number_of_captures; i++) {
		const int static_exchange_eval = pos.see(get_to_square(moves[move_index][i]));
		if (static_exchange_eval <= 0) break;
		if (eval + 200 + static_exchange_eval < alpha) continue;
		pos.make_move(moves[move_index][i]);
		const short value = -quiescence(moves, move_index+1, -beta, -alpha);
		pos.unmake_move();
		//if (pos.fen() != before_moves) {
		//	pos.print();
		//	pos.print_square_board();
		//	std::cout << "Should have been: " << before_moves << std::endl;
		//	print_move(pos.move_history.back());
		//	throw stop_exception("quiescence");
		//}

		const bool is_new_best = (current_best_eval < value);
		current_best_eval = (is_new_best)*value + (!is_new_best) * current_best_eval;
		current_best_move = (is_new_best)*moves[move_index][i] + (!is_new_best) * current_best_move;

		if (value > alpha) {
			if (value >= beta) {
				//hash_map[pos.current_hash] = TableEntry{ moves[move_index][i], value, UPPER, 0 };
				return beta;
			}
			alpha = value;
		}
	}
	//hash_map[pos.current_hash] = TableEntry{ current_best_move, alpha, (current_best_eval <= alphaOrigin) * LOWER, 0 };
	return alpha;
}
inline TableEntry Engine::lookUp() {
	auto yield = hash_map.find(pos.current_hash);
	if (yield != hash_map.end()) {
		return yield->second;
	}
	return TableEntry{ 0,0,0,-infinity-1 };
}
void Engine::set_max_depth(const short depth) {
	max_depth = depth;
}
void Engine::set_debug(const bool t_debug) {
	debug = t_debug;
}
void Engine::parse_position(std::string fen) {
	if (debug) log << fen;
	else log.log(fen);
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
			std::array<unsigned,128> move_list{};
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
	if(debug) pos.print();
}
void Engine::reset_position() {
	pos = Position{ start_position };
}
void Engine::parse_go(std::string str){
	if(debug) log << str;
	check_time = false;
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
	command = "wtime ";
	substr_pos = str.find(command);
	if (substr_pos != std::string::npos) {
		str = str.substr(substr_pos + command.size(), str.size());
		std::string time_str = str.substr(0, str.find(" "));
		const int wtime = stoi(time_str);
		command = "btime ";
		substr_pos = str.find(command);
		str = str.substr(substr_pos + command.size(), str.size());
		time_str = str.substr(0, str.find(" "));
		const int btime = stoi(time_str);
		command = "winc ";
		substr_pos = str.find(command);
		str = str.substr(substr_pos + command.size(), str.size());
		time_str = str.substr(0, str.find(" "));
		const int winc = stoi(time_str);
		command = "binc ";
		substr_pos = str.find(command);
		str = str.substr(substr_pos + command.size(), str.size());
		time_str = str.substr(0, str.find(" "));
		const int binc = stoi(time_str);

		const int increment = (pos.side) * binc + (!pos.side) * winc;
		const int time = (pos.side) * btime + (!pos.side) * wtime;
		time_for_next_move = time / 25 + increment / 2;
		if (time_for_next_move >= time) {
			time_for_next_move = time - 500;
		}
		if (time_for_next_move < 0) {
			time_for_next_move = 100;
		}
		std::cout << "Thinking time: " << time_for_next_move << std::endl;
		time_for_next_move *= 1000000ULL;
		std::thread time_tracker = std::thread(&Engine::track_time, this, time_for_next_move);
		max_depth = 100;
		check_time = true;
		bestMove();
		while (true) {
			if (time_tracker.joinable()) {
				time_tracker.join();
				break;
			}
		}
	}
}
void Engine::print_info(const short depth, const int eval, const U64 time) {
	std::stringstream stream{};
	stream << "info score cp " << eval << " depth " << depth << " nodes " << nodes << " nps " << 1000000000 * nodes / time << " pv ";
	int j = 0;
	for (int i = 0; i < depth; i++) {
		TableEntry entry = lookUp();
		if (!entry.get_move()) break;
		j++;
		stream << uci(entry.get_move()) << " ";
		pos.make_move(entry.get_move());
	}
	for (int i = 0; i < j; i++) {
		pos.unmake_move();
	}
	stream<<"\n";
	std::string str = std::move(stream).str();
	log << str;
}
void Engine::track_time(const U64 max_time) {
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	while (run) {
		end = std::chrono::steady_clock::now();
		if ((U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() >= max_time) {
			run = false;
			if(debug)std::cout << "stopping execution" << std::endl;
		}
	}
}
void Engine::uci_loop(){

	fflush(stdin);
	fflush(stdout);
	char input[2000];

	std::vector<std::thread> workers{};
	while (true) {
		memset(input, 0, sizeof(input));
		fflush(stdout);
		if (!fgets(input, 2000, stdin)) {
			continue;
		}
		if (!run) {
			for (int i = 0; i < workers.size(); i++) {
				if (workers[i].joinable()) {
					workers[i].join();
					workers.erase(workers.begin() + i);
				}
			}
			pondering = false;
		}//if no thread is running join all threads and delete joined threads from workers

		if (input[0] == '\n') {
			continue;
		}
		if (strncmp(input, "isready", 7) == 0) {
			std::cout << "readyok\n";
		}
		else if (strncmp(input, "position", 8) == 0) {
			parse_position(input);
		}
		else if (strncmp(input, "ucinewgame", 10) == 0) {
			parse_position("position startpos");
			hash_map = std::unordered_map<U64, TableEntry>{};
			history = std::array<std::array<U64, 64>, 12>{};
			if(debug) std::cout << "Done with cleanup\n";
		}
#if tune
		else if (strncmp(input, "tune", 4) == 0) {
			tune();
			break;
		}
#endif
		else if (strncmp(input, "go", 2) == 0) {
			if (!run) {
				run = true;
				workers.push_back(std::thread(&Engine::parse_go, this, input));
			}

			using namespace std::literals::chrono_literals;
			std::this_thread::sleep_for(500us);
		}
		else if (strncmp(input, "quit", 4) == 0) {
			pondering = false;
			run = false;
			for (int i = 0; i < workers.size(); i++) {
				if (workers[i].joinable()) {
					workers[i].join();
					workers.erase(workers.begin() + i);
				}
			}
			break;
		}
		else if (strncmp(input, "stop", 4) == 0) {
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
					workers.push_back(std::thread(&Engine::parse_go, this, (char*)("go depth 7")));
				}

				using namespace std::literals::chrono_literals;
				std::this_thread::sleep_for(500us);
			}
			else {
				run = false;
				for (int i = 0; i < workers.size(); i++) {
					if (workers[i].joinable()) {
						workers[i].join();
						workers.erase(workers.begin() + i);
					}
				}
				pos = Position{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };
			}
		}
		else if (strncmp(input, "uci", 3) == 0) {
			std::cout << "id name Rosy author disappointed_lama\n";
			std::cout << "option name Move Overhead type spin default 100 min 0 max 20000\noption name Threads type spin default 2 min 2 max 2\noption name Hash type spin default 512 min 256" << std::endl;
			std::cout << "uciok\n";
		}
		else if (strncmp(input, "debug", 5) == 0) {
			if (strncmp(input + 6, "true", 4) == 0) {
				debug = true;
			}
			else if (strncmp(input +6 , "false", 5) == 0) {
				debug = false;
			}
			std::cout << "Debug is set to " << ((debug) ? ("true") : ("false")) << std::endl;
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
				workers.push_back(std::thread(&Engine::parse_go, this, input));
			}
			using namespace std::literals::chrono_literals;
			std::this_thread::sleep_for(500us);
		}
		*/
	}
	/*
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
			break;
		}
		else if (strncmp(input, "stop", 4) == 0) {
			run = false;
			if (parse_runner.joinable()) {
				parse_runner.join();
			}
			reset_position();
		}
		else if (strncmp(input, "uci", 3) == 0) {
			std::cout << "id name Rosy author disappointed_lama\n";
			std::cout << "option name Move Overhead type spin default 100 min 0 max 20000\noption name Threads type spin default 2 min 2 max 2\noption name Hash type spin default 512 min 256" << std::endl;
			std::cout << "uciok\n";
		}
	*/
}