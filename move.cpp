#include "move.h"
std::string uci(const int move) {
	const int from_square = get_from_square(move);
	const int to_square = get_to_square(move);
	std::string ret = square_coordinates[from_square] + square_coordinates[to_square];
	const int promoted_type = get_promotion_type(move);
	if ((promoted_type != 15)) {
		ret += ascii_promotion_symbols[promoted_type];
	}
	return ret;
}
void print_move(int move) {
	const int piece_type = get_piece_type(move);
	const int from_square = get_from_square(move);
	const int to_square = get_to_square(move);
	const int captured_type = get_captured_type(move);
	const int promoted_type = get_promotion_type(move);

	const bool double_pawn_push = get_double_push_flag(move);
	const bool capture = get_capture_flag(move);
	const bool is_enpassant = get_enpassant_flag(move);
	const bool is_castle = get_castling_flag(move);
	char ascii_pieces[] = "PNBRQKpnbrqk";
	std::cout << "\nUci: " << uci(move) << "\n";
	printf("Piece type: %c\n", ascii_pieces[piece_type]);
	printf("Captured type: %c\n", (capture) ? ascii_pieces[captured_type] : '0');
	printf("Promotion: %c\n", (promoted_type != 15) ? ascii_pieces[promoted_type] : '0');
	printf("Is double push: %d\n", double_pawn_push);
	printf("Is castle: %d\n", is_castle);
	printf("Is enpassant: %d\n", is_enpassant);


}
void print_move_bits(int move) {
	std::cout << "\n";
	for (int i = 0; i < 28; i++) {
		std::cout << (bool)(get_bit(move, i));
	}
	std::cout << "\n";
}