#include "position.hpp"
int char_pieces(const char piece) {
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
Position::Position() {
	bitboards = std::array<U64, 12>{ { 71776119061217280ULL, 4755801206503243776ULL, 2594073385365405696ULL, 9295429630892703744ULL, 576460752303423488ULL, 1152921504606846976ULL, 65280ULL, 66ULL, 36ULL, 129ULL, 8ULL, 16ULL } };
	occupancies = std::array<U64, 3>{ { 18446462598732840960ULL, 65535ULL, 18446462598732906495ULL} };
	side = white;
	ply = 1;
	enpassant_square = a8;
	castling_rights = 15;
	no_pawns_or_captures = 0;
	current_hash = 0;
	move_history = std::vector<int>{};
	move_history.reserve(256);
	enpassant_history = std::vector<int>{};
	enpassant_history.reserve(256);
	castling_rights_history = std::vector<int>{};
	castling_rights_history.reserve(256);
	no_pawns_or_captures_history = std::vector<int>{};
	no_pawns_or_captures_history.reserve(256);
	hash_history = std::vector<U64>{};
	hash_history.reserve(256);
}
Position::Position(std::string fen) {
	parse_fen(fen);
}
void Position::parse_fen(std::string fen) {

	unsigned int i, j;
	int sq;
	char letter;
	int aRank, aFile;
	std::vector<std::string> strList;
	const std::string delimiter = " ";
	strList.push_back(fen.substr(0, fen.find(delimiter)));
	fen = fen.substr(fen.find(delimiter));
	fen = fen.substr(1);
	// Empty the board quares
	for (int i = 0; i < 12; i++) {
		bitboards[i] = 0ULL;
	}
	castling_rights = 0;
	no_pawns_or_captures = 0;
	// read the board - translate each loop idx into a square
	j = 1; i = 0;
	while ((j <= 64) && (i <= strList[0].length()))
	{
		letter = strList[0].at(i);
		i++;
		aFile = 1 + ((j - 1) % 8);
		aRank = 1 + ((j - 1) / 8);
		sq = (int)(((aRank - 1) * 8) + (aFile - 1));
		switch (letter)
		{
		case '/': j--; break;
		case '1': break;
		case '2': j++; break;
		case '3': j += 2; break;
		case '4': j += 3; break;
		case '5': j += 4; break;
		case '6': j += 5; break;
		case '7': j += 6; break;
		case '8': j += 7; break;
		default: set_bit(bitboards[char_pieces(letter)], sq);
		}
		j++;
	}

	strList.push_back(fen.substr(0, fen.find(delimiter)));
	fen = fen.substr(fen.find(delimiter));
	fen = fen.substr(1);

	// set the turn; default = White
	int sideToMove = white;
	if (strList.size() >= 2)
	{
		if (strList[1] == "w") side = white; else
			if (strList[1] == "b") side = black;
	}

	strList.push_back(fen.substr(0, fen.find(delimiter)));
	fen = fen.substr(fen.find(delimiter));
	fen = fen.substr(1);

	if (strList[2].find('K') != std::string::npos) {
		castling_rights |= wk;
	}
	if (strList[2].find('Q') != std::string::npos) {
		castling_rights |= wq;
	}
	if (strList[2].find('k') != std::string::npos) {
		castling_rights |= bk;
	}
	if (strList[2].find('q') != std::string::npos) {
		castling_rights |= bq;
	}

	strList.push_back(fen.substr(0, fen.find(delimiter)));
	fen = fen.substr(fen.find(delimiter));
	fen = fen.substr(1);

	if (fen[0] == '-') {
		enpassant_square = a8;
	}
	else {
		enpassant_square = (int)(std::find(square_coordinates, square_coordinates + sizeof(square_coordinates) / sizeof(square_coordinates[0]), strList[3]) - square_coordinates);
		enpassant_square = (enpassant_square == 64) * a8 + (enpassant_square != 64) * enpassant_square;
	}

	strList.push_back(fen.substr(0, fen.find(delimiter)));
	fen = fen.substr(fen.find(delimiter));
	fen = fen.substr(1);

	move_history.clear();
	no_pawns_or_captures_history.clear();
	castling_rights_history.clear();
	enpassant_history.clear();
	hash_history.clear();

	move_history.reserve(256);
	no_pawns_or_captures_history.reserve(256);
	castling_rights_history.reserve(256);
	enpassant_history.reserve(256);
	hash_history.reserve(256);

	no_pawns_or_captures = stoi(strList[4]);
	ply = 2* stoi(fen) - (side == white);
	occupancies[0] = bitboards[0] | bitboards[1] | bitboards[2] | bitboards[3] | bitboards[4] | bitboards[5];
	occupancies[1] = bitboards[6] | bitboards[7] | bitboards[8] | bitboards[9] | bitboards[10] | bitboards[11];
	occupancies[2] = occupancies[0] | occupancies[1];
	no_pawns_or_captures_history.push_back(no_pawns_or_captures);
	castling_rights_history.push_back(0);
	castling_rights_history.push_back(castling_rights);
	enpassant_history.push_back(0);
	enpassant_history.push_back(enpassant_square);
	current_hash = get_hash();
	hash_history.push_back(current_hash);
}

U64 Position::get_hash() const{

	size_t ret = 0;
	for (int i = 0; i < 12; i++) {
		U64 board = bitboards[i];
		while (board) {
			U64 isolated = board & twos_complement(board);
			ret ^= keys[bitscan(isolated) + i * 64];
			board = board & ones_decrement(board);
		}
	}

	ret ^= (get_bit(castling_rights, 0)) * keys[12 * 64];
	ret ^= (get_bit(castling_rights, 1)) * keys[12 * 64 + 1];
	ret ^= (get_bit(castling_rights, 2)) * keys[12 * 64 + 2];
	ret ^= (get_bit(castling_rights, 3)) * keys[12 * 64 + 3];

	ret ^= side * keys[772];

	ret ^= ((enpassant_square != a8) && (enpassant_square != 64)) * keys[773 + (enpassant_square % 8)];
	return ret % 4294967296;
}
void Position::update_hash() {

}
void Position::print()const {
	printf("\n");
	for (int rank = 0; rank < 8; rank++) {
		for (int file = 0; file < 8; file++) {
			//loop over board ranks and files

			int square = rank * 8 + file;
			//convert to square index

			if (!file) {
				printf(" %d ", 8 - rank);
			}//print rank on the left side

			int piece = -1;

			for (int piece_on_square = P; piece_on_square <= k; piece_on_square++) {
				if (get_bit(bitboards[piece_on_square], square)) {
					piece = piece_on_square;
					break;
				}
			}
			printf(" %c", (piece == -1) ? ' .' : ascii_pieces[piece]);
		}
		printf("\n");
	}
	printf("\n    a b c d e f g h \n\n");
	printf("    To move: %s\n", (side) ? "black" : "white");
	printf("    enpassant square: %s\n", (enpassant_square != a8) ? square_coordinates[enpassant_square].c_str() : "none");

	printf("    castling rights: %c%c%c%c\n", (get_bit(castling_rights, 0)) ? 'K' : '-', (get_bit(castling_rights, 1)) ? 'Q' : '-', (get_bit(castling_rights, 2)) ? 'k' : '-', (get_bit(castling_rights, 3)) ? 'q' : '-');
	//print castling rights

	printf("    halfmoves since last pawn move or capture: %d\n", no_pawns_or_captures);
	printf("    current halfclock turn: %d\n", ply);
	printf("    current game turn: %d\n", (int)ply/2 + (side==white));
	std::cout << "    current hash: " << current_hash << "\n";
}
void Position::make_move(const int move) {
	no_pawns_or_captures_history.push_back(no_pawns_or_captures);
	enpassant_history.push_back(enpassant_square);
	move_history.push_back(move);
	castling_rights_history.push_back(castling_rights);
	hash_history.push_back(current_hash);
	ply++;

	const int piece_type = get_piece_type(move);
	const int from_square = get_from_square(move);
	const int to_square = get_to_square(move);
	const int captured_type = get_captured_type(move);
	const int promoted_type = get_promotion_type(move);

	const bool double_pawn_push = get_double_push_flag(move);
	const bool capture = get_capture_flag(move);
	const bool is_enpassant = get_enpassant_flag(move);
	const bool is_castle = get_castling_flag(move);


	const bool is_white_pawn = (piece_type == P);
	const bool is_black_pawn = (piece_type == p);
	no_pawns_or_captures = (!(is_white_pawn || is_black_pawn || capture)) * (no_pawns_or_captures + 1);
	//branchlessly increment the counter if move was not a pawn move^or a capture

	enpassant_square = (double_pawn_push) * ((is_black_pawn) * (to_square - 8) + (is_white_pawn) * (to_square + 8));
	//branchlessly set enpassant square

	const int offset = 6 * (side);

	pop_bit(bitboards[(capture)*captured_type], (capture) * (!is_enpassant) * to_square + (is_enpassant) * (to_square + 8 * ((!side) - (side))));
	//pop bit on captured square. if move was not a capture the a8 bit of the white pawn is poped. however white pawns should never be on the 8th rank anyway
	const bool is_promotion = promoted_type != 15;
	const int true_piece_type = (!(is_promotion)) * piece_type + (is_promotion) * (promoted_type);
	pop_bit(bitboards[piece_type], from_square);
	set_bit(bitboards[true_piece_type], to_square);
	//make move of piece
	const bool bK = (piece_type == k);
	const bool bR = (piece_type == r);
	pop_bit(castling_rights, 4 - (bK || (bR && (from_square == a8)) || (to_square == a8)));
	pop_bit(castling_rights, 4 - 2 * (bK || (bR && (from_square == h8)) || (to_square == h8)));
	const bool wK = (piece_type == K);
	const bool wR = (piece_type == R);
	pop_bit(castling_rights, 4 - 3 * (wK || (wR && (from_square == a1)) || (to_square == a1)));
	pop_bit(castling_rights, 4 - 4 * (wK || (wR && (from_square == h1)) || (to_square == h1)));
	//branchlessly pop castle right bits

	const int square_offset = 56 * (side);
	const bool is_kingside = to_square > from_square;
	const int rook_source = (is_kingside) * (h1 - square_offset) + (!is_kingside) * (a1 - square_offset);
	const int rook_target = from_square + (to_square == g1 - square_offset) - (to_square == c1 - square_offset);
	pop_bit(bitboards[(is_castle) * (piece_type - 2)], (is_castle)*rook_source);
	//pop rooks if move is castling
	bitboards[(is_castle) * (piece_type - 2)] |= ((is_castle) * (1ULL) << (rook_target));
	//set rook if move is castling
	castling_rights &= ~(((is_castle) * 1ULL) << (2 * (piece_type == k)));
	castling_rights &= ~(((is_castle) * 1ULL) << (1 + 2 * (piece_type == k)));
	//pop castle right bits if move is castling

	side = !side;
	occupancies[0] = (bitboards[0] | bitboards[1]) | (bitboards[2] | bitboards[3]) | (bitboards[4] | bitboards[5]);
	occupancies[1] = bitboards[6] | bitboards[7] | bitboards[8] | bitboards[9] | bitboards[10] | bitboards[11];
	occupancies[2] = occupancies[0] | occupancies[1];

	update_hash();
}
void Position::unmake_move() {
	const int move = move_history.back();
	move_history.pop_back();
	no_pawns_or_captures = no_pawns_or_captures_history.back();
	no_pawns_or_captures_history.pop_back();
	enpassant_square = enpassant_history.back();
	enpassant_history.pop_back();
	castling_rights = castling_rights_history.back();
	castling_rights_history.pop_back();
	current_hash = hash_history.back();
	hash_history.pop_back();
	ply--;

	const int piece_type = get_piece_type(move);
	const int from_square = get_from_square(move);
	const int to_square = get_to_square(move);
	const int captured_type = get_captured_type(move);
	const int promoted_type = get_promotion_type(move);

	const bool double_pawn_push = get_double_push_flag(move);
	const bool capture = get_capture_flag(move);
	const bool is_enpassant = get_enpassant_flag(move);
	const bool is_castle = get_castling_flag(move);

	set_bit(bitboards[piece_type], from_square);
	pop_bit(bitboards[piece_type], to_square);

	const bool is_promotion = promoted_type != 15;
	bitboards[(is_promotion)*promoted_type] &= ~(((is_promotion) * 1ULL) << (to_square));
	//branchlessly pop the piece that was promoted. if move was not a promotion the white pawns are and'ed with a bitboard of ones, thus it would not change

	bitboards[(capture)*captured_type] |= (capture) * (1ULL << (to_square + 8 * (is_enpassant) * ((side)-(!side))));
	//branchlessly set the captured piece. if there was no captured piece the white pawn board is or'ed with 0 (not changed)
	const int square_offset = 56 * (side);
	const bool is_kingside = to_square > from_square;
	bitboards[(9 - 6 * (side))] |= (((is_castle) * 1ULL) << ((is_kingside) * (h8 + square_offset) + (!(is_kingside) * (a8 + square_offset))));
	bitboards[(9 - 6 * (side))] &= ~(((is_castle) * 1ULL) << ((is_kingside) * (f8 + square_offset) + (!(is_kingside)) * (d8 + square_offset)));
	//branchlessly set and pop rook bits if move was castling

	occupancies[0] = bitboards[0] | bitboards[1] | bitboards[2] | bitboards[3] | bitboards[4] | bitboards[5];
	occupancies[1] = bitboards[6] | bitboards[7] | bitboards[8] | bitboards[9] | bitboards[10] | bitboards[11];
	occupancies[2] = occupancies[0] | occupancies[1];
	side = !side;
}
void Position::make_nullmove() {
	hash_history.push_back(current_hash);
	no_pawns_or_captures++;
	ply++;
	current_hash ^= (enpassant_square != a8) * keys[773 + (enpassant_square % 8)];
	current_hash ^= keys[772];
	enpassant_square = a8;
	side = !side;
	move_history.push_back(0);
	enpassant_history.push_back(enpassant_square);
	castling_rights_history.push_back(castling_rights);
	no_pawns_or_captures_history.push_back(no_pawns_or_captures);
}
void Position::unmake_nullmove() {
	no_pawns_or_captures_history.pop_back();
	no_pawns_or_captures = no_pawns_or_captures_history.back();
	enpassant_history.pop_back();
	enpassant_square = enpassant_history.back();
	castling_rights_history.pop_back();
	castling_rights = castling_rights_history.back();
	current_hash = hash_history.back();
	hash_history.pop_back();
	move_history.pop_back();
	ply--;
	side = !side;
}