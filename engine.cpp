#include "engine.hpp"
OpeningBook book{ "./engines/openingBook.txt" };

stop_exception::stop_exception(string t_source) {
	source = t_source;
}
const string stop_exception::what() throw(){
	return "Stop exception thrown by " + source;
}

int Engine::bestMove() {
	if (use_opening_book) {
		const int move = book.find_move(pos.current_hash);
		if (move) {
			cout << "bestmove " << uci(move) << endl;
			run = false;
			pos = Position{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };
			return move;
		}
	}
	std::chrono::steady_clock::time_point search_start = std::chrono::steady_clock::now();
	killer_table.shift_by(2);
	const int aspiration_window = 200;
	array<array<unsigned int,128>,40> moves{};
	const int number_of_legal_moves = pos.get_legal_moves(moves[0]);
	MoveWEval best{ moves[0][0],0};
	MoveWEval old_best = best;
	short alpha = -infinity;
	short beta = infinity;
#if timingEngine
	auto start = std::chrono::steady_clock::now();
#endif
	for (current_desired_depth = 1; current_desired_depth < max_depth + 1; current_desired_depth++) {
		nodes = 0;
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		MoveWEval result = pv_root_call(moves, 0, current_desired_depth, alpha, beta);
		const bool fell_outside_window = (result.eval <= alpha) || (result.eval >= beta);
		if (fell_outside_window && run && !(result.eval==infinity+2 || result.eval==-infinity-2)) {
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
#if timingEngine
	auto end = std::chrono::steady_clock::now();
	totalEngineTime += (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	cout << "Time spend:\n";
	cout << "pv search: " << (1 - (double)quiescenceTime / totalEngineTime) * 100.0 << "%\n";
	cout<<"move generation : " << ((double)moveGenerationTime / totalEngineTime) * 100.0 << " % \n";
	cout << "move ordering: " << ((double)moveOrderingTime / totalEngineTime) * 100.0 << "%\n";
	cout << "quiescence: " << ((double)quiescenceTime / totalEngineTime) * 100.0 << "%\n";
	cout << "move generation quiescence: " << ((double)moveGenerationQuiescenceTime / totalEngineTime) * 100.0 << "%\n";
	cout << "capture generation : " << ((double)captureGenerationTime / totalEngineTime) * 100.0 << " % \n";
	cout << "capture ordering: " << ((double)captureOrderingTime / totalEngineTime) * 100.0 << "%\n";
	cout << "evaluation: " << ((double)evaluationTime / totalEngineTime) * 100.0 << "%\n";
	totalEngineTime = 0ULL;
	moveGenerationTime = 0ULL;
	captureGenerationTime = 0ULL;
	quiescenceTime = 0ULL;
	moveOrderingTime = 0ULL;
	captureOrderingTime = 0ULL;
	moveGenerationQuiescenceTime = 0ULL;
	evaluationTime = 0ULL;
#endif
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
	array<array<unsigned int, 128>, 40> moves{};
	const int number_of_legal_moves = pos.get_legal_moves(moves[0]);
	MoveWEval best{ moves[0][0],0 };
	MoveWEval old_best = best;
	short alpha = -infinity;
	short beta = infinity;
	for (current_desired_depth = 1; current_desired_depth < max_depth + 1; current_desired_depth++) {
		nodes = 0;
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		MoveWEval result = pv_root_call(moves, 0, current_desired_depth, alpha, beta);
		const bool fell_outside_window = (result.eval <= alpha) || (result.eval >= beta);
		if (fell_outside_window && run && !(result.eval == infinity + 2 || result.eval == -infinity - 2)) {
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
MoveWEval Engine::pv_root_call(array<array<unsigned int, 128>, 40>& moves, int move_index, const short depth, short alpha, short beta) {
	TableEntry entry = lookUp();
#if timingEngine
	auto start = std::chrono::steady_clock::now();
#endif
	const int number_of_moves = pos.get_legal_moves(moves[move_index]);
#if timingEngine
	auto end = std::chrono::steady_clock::now();
	moveGenerationTime += (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif
	order(moves[move_index], entry, number_of_moves);
	unsigned int current_best_move = 0;
	short current_best_eval = -infinity-1;
	for (int i = 0; i < number_of_moves; i++) {
		if (debug) {
			cout << "starting " << uci(moves[move_index][i]) << endl;
		}
		short value = 0;
		pos.make_move(moves[move_index][i]);
		value = -pv_search(moves, move_index+1, depth - 1, -beta, -alpha,true);
		pos.unmake_move();
		if (value == infinity + 2 || value == infinity - 2) {
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
				cout << "Found new best move in: " << "\n";
				print_move(moves[move_index][i]);
				cout << " (" << value << ")" << endl;
			}
			else {
				cout << "finished " << uci(moves[move_index][i]) << endl;
			}
		}
		if (value > alpha) {
			alpha = value;
			if (value >= infinity) {
				if (debug) {
					cout << "Finished depth " << depth << endl;
				}
				hash_map[pos.current_hash] = TableEntry{ current_best_move,current_best_eval,EXACT,depth };
				return MoveWEval{current_best_move, current_best_eval};
			}
		}
	}
	if (debug) {
		cout << "Finished depth " << depth << endl;
	}
	hash_map[pos.current_hash] = TableEntry{ current_best_move, current_best_eval, EXACT, depth };
	return MoveWEval{ current_best_move, current_best_eval };
}
short Engine::pv_search(array<array<unsigned int, 128>, 40>& moves, int move_index, const short depth, short alpha, short beta, bool isPV) {
	if (!run) {
		return infinity + 2;
	}
	nodes++;

#if timingEngine
	auto start = std::chrono::steady_clock::now();
#endif

	const int number_of_moves = pos.get_legal_moves(moves[move_index]);

#if timingEngine
	auto end = std::chrono::steady_clock::now();
	moveGenerationTime += (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif

	const bool draw_by_repetition = pos.is_draw_by_repetition();
	const bool draw_by_fifty_move_rule = pos.is_draw_by_fifty_moves();
	const bool in_check = pos.currently_in_check();
	const bool no_moves_left = number_of_moves == 0;
	const bool draw_by_stalemate = no_moves_left && (!in_check);
	const bool is_draw = draw_by_fifty_move_rule || draw_by_repetition || draw_by_stalemate;
	const bool is_lost = no_moves_left && in_check;

	if (is_draw || is_lost) {
#if timingEngine
		start = std::chrono::steady_clock::now();
#endif

		const short eval = pos.evaluate(is_draw, is_lost);

#if timingEngine
		end = std::chrono::steady_clock::now();
		evaluationTime += (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif
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
#if timingEngine
		start = std::chrono::steady_clock::now();
		short eval = quiescence(moves, move_index + 1, alpha, beta);
		end = std::chrono::steady_clock::now();
		quiescenceTime += (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		return eval;
#else

		return quiescence(moves, move_index+1,alpha, beta);

#endif
	}
	const int phase = pos.get_phase();
	if ((!isPV) && (depth >= 1 + Red) && (!in_check) && (phase>=14)) {
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
	//string before_moves = pos.fen();
	pos.make_move(moves[move_index][0]);
	bool in_check_now = pos.currently_in_check();
	short value = -pv_search(moves, move_index + 1, depth - 1, -beta, -alpha, true);
	pos.unmake_move();
	//if (pos.fen() != before_moves) {
	//	pos.print();
	//	pos.print_square_board();
	//	cout << "Should have been: " << before_moves << endl;
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
				[[assume(get_piece_type(moves[move_index][0]) > -1)]];
				[[assume(get_piece_type(moves[move_index][0]) < 12)]];
				[[assume(get_to_square(moves[move_index][0]) > -1)]];
				[[assume(get_to_square(moves[move_index][0]) < 64)]];
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
		//	cout << "Should have been: " << before_moves << endl;
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
					[[assume(get_piece_type(moves[move_index][i]) > -1)]];
					[[assume(get_piece_type(moves[move_index][i]) < 12)]];
					[[assume(get_to_square(moves[move_index][i]) > -1)]];
					[[assume(get_to_square(moves[move_index][i]) < 64)]];
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
short Engine::quiescence(array<array<unsigned int, 128>, 40>& moves, int move_index, short alpha, short beta) {
	if (!run) {
		return infinity + 2;
	}
	nodes++;
	const short alphaOrigin = alpha;
#if timingEngine
	auto start = std::chrono::steady_clock::now();
#endif

	const int number_of_captures = pos.get_legal_captures(moves[move_index]);

#if timingEngine
	auto end = std::chrono::steady_clock::now();
	captureGenerationTime += (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif

	const bool draw_by_repetition = pos.is_draw_by_repetition();
	const bool draw_by_fifty_move_rule = pos.is_draw_by_fifty_moves();
	bool is_draw = draw_by_fifty_move_rule || draw_by_repetition;

	if (is_draw) {
#if timingEngine
		start = std::chrono::steady_clock::now();
#endif

		const short eval = pos.evaluate(is_draw, false);

#if timingEngine
		end = std::chrono::steady_clock::now();
		evaluationTime += (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif
		return eval;
	}
	if (number_of_captures == 0) {
#if timingEngine
		start = std::chrono::steady_clock::now();
#endif

		const int number_of_moves = pos.get_legal_moves(moves[move_index]);

#if timingEngine
		end = std::chrono::steady_clock::now();
		moveGenerationQuiescenceTime += (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif
		const bool no_moves_left = number_of_moves == 0;
		const bool in_check = pos.currently_in_check();
		const bool draw_by_stalemate = no_moves_left && (!in_check);
		is_draw = is_draw || draw_by_stalemate;
		const bool is_lost = no_moves_left && in_check;
#if timingEngine
		start = std::chrono::steady_clock::now();
#endif

		const short eval = pos.evaluate(is_draw, is_lost);

#if timingEngine
		end = std::chrono::steady_clock::now();
		evaluationTime += (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif
		return eval;
	}
#if timingEngine
	start = std::chrono::steady_clock::now();
#endif

	short eval = pos.evaluate(false, false);

#if timingEngine
	end = std::chrono::steady_clock::now();
	evaluationTime += (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
#endif
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

	//string before_moves = pos.fen();
	unsigned int current_best_move = 0;
	short current_best_eval = -infinity - 1;
	for (int i = 0; i < number_of_captures; i++) {
		const int static_exchange_eval = pos.seeByMove(moves[move_index][i]);
		if (static_exchange_eval <= 0) break;
		if (eval + 200 + static_exchange_eval < alpha) continue;
		pos.make_move(moves[move_index][i]);
		const short value = -quiescence(moves, move_index+1, -beta, -alpha);
		pos.unmake_move();
		//if (pos.fen() != before_moves) {
		//	pos.print();
		//	pos.print_square_board();
		//	cout << "Should have been: " << before_moves << endl;
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
void Engine::parse_position(string fen) {
	if (debug) log << fen;
	else log.log(fen);
	string moves="";
	string str = " moves ";
	auto substr_pos = fen.find(str);
	if (substr_pos != string::npos) {
		moves = fen.substr(substr_pos + str.size(), fen.size());
		fen = fen.substr(0, substr_pos);
	}
	str = "startpos";
	substr_pos = fen.find(str);
	if (substr_pos != string::npos) {
		fen = start_position;
	}
	str = "fen ";
	substr_pos = fen.find(str);
	if (substr_pos != string::npos) {
		fen = fen.substr(substr_pos + str.size(), fen.size());
	}
	pos = Position(fen);
	try {
		while (moves != "") {
			array<unsigned,128> move_list{};
			pos.get_legal_moves(move_list);
			string move_string = moves.substr(0, moves.find_first_of(' '));
			if (move_string.size() > 4) {
				const size_t last = move_string.size() - 1;
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
		cout << e.what() << endl;
	}
	if(debug) pos.print();
}
void Engine::reset_position() {
	pos = Position{ start_position };
}
void Engine::parse_go(string str){
	if(debug) log << str;
	check_time = false;
	string command = "depth ";
	auto substr_pos = str.find(command);
	if (substr_pos != string::npos) {
		str = str.substr(substr_pos + command.size(), str.size());
		max_depth = stoi(str);
		bestMove();
		return;
	}
	command = "infinite";
	substr_pos = str.find(command);
	if (substr_pos != string::npos) {
		str = str.substr(substr_pos + command.size(), str.size());
		max_depth = infinity;
		bestMove();
		return;
	}
	command = "movetime ";
	substr_pos = str.find(command);
	if (substr_pos != string::npos) {
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
	if (substr_pos != string::npos) {
		str = str.substr(substr_pos + command.size(), str.size());
		string time_str = str.substr(0, str.find(" "));
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

		const int increment = ((pos.side) * binc) + (!(pos.side) * winc);
		const int time = ((pos.side) * btime) + ((!pos.side) * wtime);
		time_for_next_move = time / 25 + increment / 2;
		if (time_for_next_move >= time) {
			time_for_next_move = time - 500;
		}
		if (time_for_next_move < 0) {
			time_for_next_move = 100;
		}
		cout << "Thinking time: " << time_for_next_move << endl;
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
	command = "perft ";
	substr_pos = str.find(command);
	if (substr_pos != string::npos) {
		str = str.substr(substr_pos + command.size(), str.size());
		max_depth = stoi(str);
		perft();
		return;
	}
}
void Engine::perft() {
	pos.print();
	array<array<unsigned int, 128>, 40> moves{};
	const int number_of_moves = pos.get_legal_moves(moves[0]);
	cout << "Nodes from different branches:\n";
	U64 total_nodes = 0ULL;
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	for (int i = 0; i < number_of_moves; ++i) {
		nodes = 0ULL;
		pos.make_move(moves[0][i]);
		perft_traversal(moves, 1, max_depth - 1);
		pos.unmake_move();
		cout << "\t" << uci(moves[0][i]) << ": " << nodes << " Nodes\n";
		total_nodes += nodes;
	}
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	U64 totalTime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	cout << "Total Nodes: " << total_nodes << "\n";
	cout << "Time: " << totalTime/1000000000.0 << "s (" << 1000.0 * total_nodes / totalTime << " MHz)\n";
	reset_position();
	run = false;
}
void Engine::perft_traversal(array<array<unsigned int, 128>, 40>& moves, int move_index, const int depth) {
	if (depth == 0) {
		++nodes;
		return;
	}
	const int number_of_moves = pos.get_legal_moves(moves[move_index]);
	for (int i = 0; i < number_of_moves; ++i) {
		pos.make_move(moves[move_index][i]);
		perft_traversal(moves, move_index + 1, depth - 1);
		pos.unmake_move();
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
	string str = std::move(stream).str();
	log << str;
}
void Engine::track_time(const U64 max_time) {
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	while (run && (U64)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - begin).count() < max_time) {
		std::this_thread::sleep_for(std::chrono::nanoseconds{ max_time / 50 });
		if (debug)cout << "stopping execution" << endl;
	}
	run = false;
}
void Engine::uci_loop(){

	fflush(stdin);
	fflush(stdout);
	char input[2000];

	vector<std::thread> workers{};
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
		log.log(input);
		if (strncmp(input, "isready", 7) == 0) {
			cout << "readyok\n";
		}
		else if (strncmp(input, "position", 8) == 0) {
			parse_position(input);
		}
		else if (strncmp(input, "ucinewgame", 10) == 0) {
			parse_position("position startpos");
			hash_map = std::unordered_map<U64, TableEntry>{};
			history = array<array<U64, 64>, 12>{};
			if(debug) cout << "Done with cleanup\n";
		}
		else if (strncmp(input, "go", 2) == 0) {
			using namespace std::literals::chrono_literals;
			log.log((run) ? "run: true" : "run: false");
			if (!run) {
				run = true;
				std::this_thread::sleep_for(500us);
				if (!workers.empty()) {
					log << "joining threads, not all threads were cleared before starting new go";
					for (int i = 0; i < workers.size(); i++) {
						if (workers[i].joinable()) {
							workers[i].join();
							workers.erase(workers.begin() + i);
						}
					}
				}
				workers.push_back(std::thread(&Engine::parse_go, this, input));
			}

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
				reset_position();
			}
		}
		else if (strncmp(input, "uci", 3) == 0) {
			cout << "id name Rosy author disappointed_lama\n";
			cout << "option name Move Overhead type spin default 100 min 0 max 20000\noption name Threads type spin default 2 min 2 max 2\noption name Hash type spin default 512 min 256" << endl;
			cout << "uciok\n";
		}
		else if (strncmp(input, "debug", 5) == 0) {
			if (strncmp(input + 6, "on", 2) == 0) {
				debug = true;
			}
			else if (strncmp(input +6 , "off", 3) == 0) {
				debug = false;
			}
			//cout << "Debug is set to " << ((debug) ? ("true") : ("false")) << endl;
		}
	}
}