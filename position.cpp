#include "position.hpp"

bool Position::is_attacked_by_side(const int sq, const bool color) {
	if (color) {
		return ((get_rook_attacks(occupancies[both], sq) & (bitboards[9] | bitboards[10])) | (get_bishop_attacks(occupancies[both], sq) & (bitboards[8] | bitboards[10])) | (pawn_attacks[!color][sq] & bitboards[6]) | (king_attacks[sq] & bitboards[11]) | (knight_attacks[sq] & bitboards[7]));
	}
	return (get_rook_attacks(occupancies[both], sq) & (bitboards[3] | bitboards[4])) | (get_bishop_attacks(occupancies[both], sq) & (bitboards[2] | bitboards[4])) | (pawn_attacks[!color][sq] & bitboards[0]) | (king_attacks[sq] & bitboards[5]) | (knight_attacks[sq] & bitboards[1]);
}
U64 Position::get_attacks_by(const bool color) {

	int offset = (color) * 6;
	U64 attacks = ((bool)(bitboards[K + offset])) * king_attacks[bitscan(bitboards[K + offset])];
	int type = N + offset;
	U64 tempKnights = bitboards[type];

	while (tempKnights) {
		U64 isolated = tempKnights & twos_complement(tempKnights);
		attacks |= knight_attacks[bitscan(isolated)];
		tempKnights = tempKnights & ones_decrement(tempKnights);
	}
	//knight attacks individually

	attacks |= (!color) * (((bitboards[0] >> 7) & notAFile) | ((bitboards[0] >> 9) & notHFile)) + (color) * (((bitboards[6] << 7) & notHFile) | ((bitboards[6] << 9) & notAFile));
	//setwise pawn attacks

	const U64 rooks_w_queens = bitboards[R + offset] | bitboards[Q + offset];
	U64 rooks = rooks_w_queens;
	U64 flood = rooks_w_queens;
	U64 empty = ~occupancies[both];
	flood |= rooks = (rooks >> 8) & empty;
	flood |= rooks = (rooks >> 8) & empty;
	flood |= rooks = (rooks >> 8) & empty;
	flood |= rooks = (rooks >> 8) & empty;
	flood |= rooks = (rooks >> 8) & empty;
	flood |= (rooks >> 8) & empty;
	attacks |= flood >> 8;

	rooks = rooks_w_queens;
	flood = rooks_w_queens;
	flood |= rooks = (rooks << 8) & empty;
	flood |= rooks = (rooks << 8) & empty;
	flood |= rooks = (rooks << 8) & empty;
	flood |= rooks = (rooks << 8) & empty;
	flood |= rooks = (rooks << 8) & empty;
	flood |= (rooks << 8) & empty;
	attacks |= flood << 8;

	rooks = rooks_w_queens;
	flood = rooks_w_queens;
	empty &= notAFile;
	flood |= rooks = (rooks << 1) & empty;
	flood |= rooks = (rooks << 1) & empty;
	flood |= rooks = (rooks << 1) & empty;
	flood |= rooks = (rooks << 1) & empty;
	flood |= rooks = (rooks << 1) & empty;
	flood |= (rooks << 1) & empty;
	attacks |= (flood << 1) & notAFile;

	const U64 bishops_w_queens = bitboards[B + offset] | bitboards[Q + offset];
	U64 bishops = bishops_w_queens;
	flood = bishops_w_queens;
	flood |= bishops = (bishops >> 7) & empty;
	flood |= bishops = (bishops >> 7) & empty;
	flood |= bishops = (bishops >> 7) & empty;
	flood |= bishops = (bishops >> 7) & empty;
	flood |= bishops = (bishops >> 7) & empty;
	flood |= (bishops >> 7) & empty;
	attacks |= (flood >> 7) & notAFile;

	bishops = bishops_w_queens;
	flood = bishops_w_queens;
	flood |= bishops = (bishops << 9) & empty;
	flood |= bishops = (bishops << 9) & empty;
	flood |= bishops = (bishops << 9) & empty;
	flood |= bishops = (bishops << 9) & empty;
	flood |= bishops = (bishops << 9) & empty;
	flood |= (bishops << 9) & empty;
	attacks |= (flood << 9) & notAFile;


	rooks = rooks_w_queens;
	flood = rooks_w_queens;
	empty = ~occupancies[both] & notHFile;
	flood |= rooks = (rooks >> 1) & empty;
	flood |= rooks = (rooks >> 1) & empty;
	flood |= rooks = (rooks >> 1) & empty;
	flood |= rooks = (rooks >> 1) & empty;
	flood |= rooks = (rooks >> 1) & empty;
	flood |= (rooks >> 1) & empty;
	attacks |= (flood >> 1) & notHFile;

	bishops = bishops_w_queens;
	flood = bishops_w_queens;
	flood |= bishops = (bishops >> 9) & empty;
	flood |= bishops = (bishops >> 9) & empty;
	flood |= bishops = (bishops >> 9) & empty;
	flood |= bishops = (bishops >> 9) & empty;
	flood |= bishops = (bishops >> 9) & empty;
	flood |= (bishops >> 9) & empty;
	attacks |= (flood >> 9) & notHFile;

	bishops = bishops_w_queens;
	flood = bishops_w_queens;
	flood |= bishops = (bishops << 7) & empty;
	flood |= bishops = (bishops << 7) & empty;
	flood |= bishops = (bishops << 7) & empty;
	flood |= bishops = (bishops << 7) & empty;
	flood |= bishops = (bishops << 7) & empty;
	flood |= (bishops << 7) & empty;
	attacks |= (flood << 7) & notHFile;

	return attacks;
}
int Position::get_piece_type_on(const int sq) {
	const int offset = (!side) * 6;
	bool found_piece = false;
	int piece_type = 0;
	bool bit;
	for (int ind = offset; ind < offset + 6; ind++) {
		bit = get_bit(bitboards[ind], sq);
		if (bit) {
			found_piece = bit;
			piece_type = ind;
			break;
		}
	}
	const bool is_enpassant = (sq == enpassant_square);
	return (!found_piece) * ((is_enpassant)*offset + 15 * (!is_enpassant)) + (found_piece)*piece_type;
}
void Position::try_out_move(std::vector<int>& ret, int move) {
	make_move(move);
	if (!is_attacked_by_side(bitscan(bitboards[11 - (side) * 6]), side)) ret.push_back(move);
	//append move if the king is not attacked after playing the move
	unmake_move();
}

void Position::get_legal_moves(std::vector<int>& ret) {
	ret.reserve(256);
	const int kingpos = bitscan(bitboards[K + (side) * 6]);
	const bool in_check = is_attacked_by_side(kingpos, !side);
	const U64 kings_queen_scope = get_queen_attacks(occupancies[both], kingpos);
	const U64 enemy_attacks = get_attacks_by(!side);

	if (in_check) {
		legal_in_check_move_generator(ret, kingpos, kings_queen_scope, enemy_attacks);
		return;
	}
	legal_move_generator(ret, kingpos, kings_queen_scope, enemy_attacks);
}
void Position::legal_move_generator(std::vector<int>& ret, const int kingpos, const U64 kings_queen_scope, const U64 enemy_attacks) {
	get_castles(ret);
	const U64 pinned = get_moves_for_pinned_pieces(ret, kingpos, enemy_attacks);
	U64 enemy_pieces = occupancies[(!side)];
	U64 valid_targets = (~occupancies[both]) | enemy_pieces;
	const int offset = (side) * 6;
	int type = R + offset;
	U64 tempRooks = bitboards[type] & (~pinned);
	int move;
	while (tempRooks) {
		U64 isolated = tempRooks & twos_complement(tempRooks);
		const int sq = bitscan(isolated);
		U64 attacks = get_rook_attacks(occupancies[both], sq) & valid_targets;

		while (attacks) {

			U64 isolated2 = attacks & twos_complement(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), 15, (bool)(get_bit(enemy_pieces, to)), false, false, false);
			ret.push_back(move);

			attacks = attacks & ones_decrement(attacks);
		}
		tempRooks = tempRooks & ones_decrement(tempRooks);
	}

	type = B + offset;
	U64 tempBishops = bitboards[type] & (~pinned);

	while (tempBishops) {
		U64 isolated = tempBishops & twos_complement(tempBishops);
		const int sq = bitscan(isolated);
		U64 attacks = get_bishop_attacks(occupancies[both], sq) & valid_targets;
		;
		while (attacks) {

			U64 isolated2 = attacks & twos_complement(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), 15, (bool)(get_bit(enemy_pieces, to)), false, false, false);
			ret.push_back(move);

			attacks = attacks & ones_decrement(attacks);
		}
		tempBishops = tempBishops & ones_decrement(tempBishops);
	}

	type = Q + offset;
	U64 tempQueens = bitboards[type] & (~pinned);

	while (tempQueens) {
		U64 isolated = tempQueens & twos_complement(tempQueens);
		const int sq = bitscan(isolated);
		U64 attacks = get_queen_attacks(occupancies[both], sq) & valid_targets;

		while (attacks) {

			U64 isolated2 = attacks & twos_complement(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), 15, (bool)(get_bit(enemy_pieces, to)), false, false, false);
			ret.push_back(move);

			attacks = attacks & ones_decrement(attacks);
		}
		tempQueens = tempQueens & ones_decrement(tempQueens);
	}

	type = N + offset;
	U64 tempKnights = bitboards[type] & (~pinned);

	while (tempKnights) {
		U64 isolated = tempKnights & twos_complement(tempKnights);
		const int sq = bitscan(isolated);
		U64 attacks = knight_attacks[sq] & valid_targets;

		while (attacks) {

			U64 isolated2 = attacks & twos_complement(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), 15, (bool)(get_bit(enemy_pieces, to)), false, false, false);
			ret.push_back(move);

			attacks = attacks & ones_decrement(attacks);
		}
		tempKnights = tempKnights & ones_decrement(tempKnights);
	}

	type = K + offset;
	U64 attacks = king_attacks[kingpos] & (~(occupancies[(side)] | enemy_attacks));
	while (attacks) {

		U64 isolated = attacks & twos_complement(attacks);

		const int to = bitscan(isolated);
		move = encode_move(kingpos, to, type, get_piece_type_on(to), 15, (bool)(get_bit(enemy_pieces, to)), false, false, false);
		ret.push_back(move);

		attacks = attacks & ones_decrement(attacks);
	}
	get_legal_pawn_moves(ret, kings_queen_scope, enemy_attacks,pinned);
}
void Position::legal_in_check_move_generator(std::vector<int>& ret, const int kingpos, const U64 kings_queen_scope, const U64 enemy_attacks){
	const U64 pinned = get_pinned_pieces(kingpos, enemy_attacks);
	const U64 checkers = get_checkers(kingpos);
	const bool not_double_check = count_bits(checkers) < 2;
	U64 valid_targets = (not_double_check) * (get_checking_rays(kingpos) | checkers);
	U64 test = (~occupancies[both]) | occupancies[(!side)];
	const int offset = (side) * 6;
	int type = R + offset;
	U64 tempRooks = bitboards[type] & ((~pinned) * (not_double_check));
	int move;
	while (tempRooks) {
		U64 isolated = tempRooks & twos_complement(tempRooks);
		const int sq = bitscan(isolated);
		U64 attacks = get_rook_attacks(occupancies[both], sq) & valid_targets;

		while (attacks) {

			U64 isolated2 = attacks & twos_complement(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), 15, (bool)(get_bit(occupancies[(!side)], to)), false, false, false);
			ret.push_back(move);

			attacks = attacks & ones_decrement(attacks);
		}
		tempRooks = tempRooks & ones_decrement(tempRooks);
	}

	type = B + offset;
	U64 tempBishops = bitboards[type] & ((~pinned) * (not_double_check));

	while (tempBishops) {
		U64 isolated = tempBishops & twos_complement(tempBishops);
		const int sq = bitscan(isolated);
		U64 attacks = get_bishop_attacks(occupancies[both], sq) & valid_targets;

		while (attacks) {

			U64 isolated2 = attacks & twos_complement(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), 15, (bool)(get_bit(occupancies[(!side)], to)), false, false, false);
			ret.push_back(move);

			attacks = attacks & ones_decrement(attacks);
		}
		tempBishops = tempBishops & ones_decrement(tempBishops);
	}

	type = Q + offset;
	U64 tempQueens = bitboards[type] & ((~pinned) * (not_double_check));

	while (tempQueens) {
		U64 isolated = tempQueens & twos_complement(tempQueens);
		const int sq = bitscan(isolated);
		U64 attacks = get_queen_attacks(occupancies[both], sq) & valid_targets;

		while (attacks) {

			U64 isolated2 = attacks & twos_complement(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), 15, (bool)(get_bit(occupancies[(!side)], to)), false, false, false);
			ret.push_back(move);

			attacks = attacks & ones_decrement(attacks);
		}
		tempQueens = tempQueens & ones_decrement(tempQueens);
	}

	type = N + offset;
	U64 tempKnights = bitboards[type] & ((~pinned) * (not_double_check));

	while (tempKnights) {
		U64 isolated = tempKnights & twos_complement(tempKnights);
		const int sq = bitscan(isolated);
		U64 attacks = knight_attacks[sq] & valid_targets;

		while (attacks) {

			U64 isolated2 = attacks & twos_complement(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), 15, (bool)(get_bit(occupancies[(!side)], to)), false, false, false);
			ret.push_back(move);

			attacks = attacks & ones_decrement(attacks);
		}
		tempKnights = tempKnights & ones_decrement(tempKnights);
	}

	type = K + offset;
	pop_bit(occupancies[both], kingpos);
	const U64 enemy_attacks_without_king = get_attacks_by(!side);
	set_bit(occupancies[both], kingpos);
	U64 attacks = king_attacks[kingpos] & (~(enemy_attacks_without_king | occupancies[(side)]));
	while (attacks) {

		U64 isolated = attacks & twos_complement(attacks);

		const int to = bitscan(isolated);
		move = encode_move(kingpos, to, type, get_piece_type_on(to), 15, (bool)(get_bit(occupancies[(!side)], to)), false, false, false);
		ret.push_back(move);

		attacks = attacks & ones_decrement(attacks);
	}
	in_check_get_legal_pawn_moves(ret, kings_queen_scope | knight_attacks[kingpos], enemy_attacks, pinned, (not_double_check)*checkers, (not_double_check)*valid_targets);
}

void Position::get_legal_captures(std::vector<int>& ret) {
	ret.reserve(256);
	const int kingpos = bitscan(bitboards[K + (side) * 6]);
	const bool in_check = is_attacked_by_side(kingpos, !side);
	const U64 kings_queen_scope = get_queen_attacks(occupancies[both], kingpos);
	const U64 enemy_attacks = get_attacks_by(!side);
	if (in_check) {
		legal_in_check_capture_gen(ret, kings_queen_scope, enemy_attacks);
		return;
	}
	legal_capture_gen(ret, kings_queen_scope, enemy_attacks);
}
void Position::legal_capture_gen(std::vector<int>& ret, const U64 kings_queen_scope, const U64& enemy_attacks) {
	const int offset = (side) * 6;
	const int kingpos = bitscan(bitboards[K + offset]);
	const U64 pinned = get_captures_for_pinned_pieces(ret, kingpos, enemy_attacks);
	const U64 enemy_pieces = occupancies[(!side)];
	int type = R + offset;
	U64 tempRooks = bitboards[type] & (~pinned);
	int move;
	while (tempRooks) {
		U64 isolated = tempRooks & twos_complement(tempRooks);
		const int sq = bitscan(isolated);
		U64 attacks = get_rook_attacks(occupancies[both], sq) & enemy_pieces;

		while (attacks) {

			U64 isolated2 = attacks & twos_complement(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), 15, true, false, false, false);
			ret.push_back(move);

			attacks = attacks & ones_decrement(attacks);
		}
		tempRooks = tempRooks & ones_decrement(tempRooks);
	}

	type = B + offset;
	U64 tempBishops = bitboards[type] & (~pinned);

	while (tempBishops) {
		U64 isolated = tempBishops & twos_complement(tempBishops);
		const int sq = bitscan(isolated);
		U64 attacks = get_bishop_attacks(occupancies[both], sq) & enemy_pieces;
		;
		while (attacks) {

			U64 isolated2 = attacks & twos_complement(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), 15, true, false, false, false);
			ret.push_back(move);

			attacks = attacks & ones_decrement(attacks);
		}
		tempBishops = tempBishops & ones_decrement(tempBishops);
	}

	type = Q + offset;
	U64 tempQueens = bitboards[type] & (~pinned);

	while (tempQueens) {
		U64 isolated = tempQueens & twos_complement(tempQueens);
		const int sq = bitscan(isolated);
		U64 attacks = get_queen_attacks(occupancies[both], sq) & enemy_pieces;

		while (attacks) {

			U64 isolated2 = attacks & twos_complement(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), 15, true, false, false, false);
			ret.push_back(move);

			attacks = attacks & ones_decrement(attacks);
		}
		tempQueens = tempQueens & ones_decrement(tempQueens);
	}

	type = N + offset;
	U64 tempKnights = bitboards[type] & (~pinned);

	while (tempKnights) {
		U64 isolated = tempKnights & twos_complement(tempKnights);
		const int sq = bitscan(isolated);
		U64 attacks = knight_attacks[sq] & enemy_pieces;

		while (attacks) {

			U64 isolated2 = attacks & twos_complement(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), 15, true, false, false, false);
			ret.push_back(move);

			attacks = attacks & ones_decrement(attacks);
		}
		tempKnights = tempKnights & ones_decrement(tempKnights);
	}
	type = K + offset;
	U64 tempKing = bitboards[type];

	U64 attacks = king_attacks[kingpos] & (enemy_pieces & (~enemy_attacks));

	while (attacks) {

		U64 isolated2 = attacks & twos_complement(attacks);

		const int to = bitscan(isolated2);
		move = encode_move(kingpos, to, type, get_piece_type_on(to), 15, true, false, false, false);
		ret.push_back(move);

		attacks = attacks & ones_decrement(attacks);
	}
	get_pawn_captures(ret, kings_queen_scope, enemy_attacks,pinned);
}
void Position::legal_in_check_capture_gen(std::vector<int>& ret, const U64 kings_queen_scope, const U64& enemy_attacks) {
	const int offset = (side) * 6;
	const int kingpos = bitscan(bitboards[K + offset]);
	const U64 pinned = get_pinned_pieces(kingpos, enemy_attacks);
	const U64 checkers = get_checkers(kingpos);
	const bool not_double_check = count_bits(checkers) < 2;

	int type = R + offset;
	U64 tempRooks = bitboards[type] & ((~pinned) * (not_double_check));
	int move;
	while (tempRooks) {
		U64 isolated = tempRooks & twos_complement(tempRooks);
		const int sq = bitscan(isolated);
		U64 attacks = get_rook_attacks(occupancies[both], sq) & checkers;

		while (attacks) {

			U64 isolated2 = attacks & twos_complement(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), 15, true, false, false, false);
			try_out_move(ret, move);

			attacks = attacks & ones_decrement(attacks);
		}
		tempRooks = tempRooks & ones_decrement(tempRooks);
	}

	type = B + offset;
	U64 tempBishops = bitboards[type] & ((~pinned) * (not_double_check));

	while (tempBishops) {
		U64 isolated = tempBishops & twos_complement(tempBishops);
		const int sq = bitscan(isolated);
		U64 attacks = get_bishop_attacks(occupancies[both], sq) & checkers;
		;
		while (attacks) {

			U64 isolated2 = attacks & twos_complement(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), 15, true, false, false, false);
			try_out_move(ret, move);

			attacks = attacks & ones_decrement(attacks);
		}
		tempBishops = tempBishops & ones_decrement(tempBishops);
	}

	type = Q + offset;
	U64 tempQueens = bitboards[type] & ((~pinned) * (not_double_check));

	while (tempQueens) {
		U64 isolated = tempQueens & twos_complement(tempQueens);
		const int sq = bitscan(isolated);
		U64 attacks = get_queen_attacks(occupancies[both], sq) & checkers;

		while (attacks) {

			U64 isolated2 = attacks & twos_complement(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), 15, true, false, false, false);
			try_out_move(ret, move);

			attacks = attacks & ones_decrement(attacks);
		}
		tempQueens = tempQueens & ones_decrement(tempQueens);
	}

	type = N + offset;
	U64 tempKnights = bitboards[type] & ((~pinned) * (not_double_check));

	while (tempKnights) {
		U64 isolated = tempKnights & twos_complement(tempKnights);
		const int sq = bitscan(isolated);
		U64 attacks = knight_attacks[sq] & checkers;

		while (attacks) {

			U64 isolated2 = attacks & twos_complement(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), 15, true, false, false, false);
			try_out_move(ret, move);

			attacks = attacks & ones_decrement(attacks);
		}
		tempKnights = tempKnights & ones_decrement(tempKnights);
	}
	type = K + offset;
	U64 tempKing = bitboards[type];

	//IN_CHECK_VALID = (not_double_check) * (get_checking_rays(kingpos) | checkers);
	type = K + offset;
	pop_bit(occupancies[both], kingpos);
	const U64 enemy_attacks_without_king = get_attacks_by(!side);
	set_bit(occupancies[both], kingpos);
	U64 attacks = (king_attacks[kingpos] & occupancies[(!side)]) & (~(enemy_attacks_without_king | occupancies[(side)]));
	while (attacks) {

		U64 isolated = attacks & twos_complement(attacks);

		const int to = bitscan(isolated);
		move = encode_move(kingpos, to, type, get_piece_type_on(to), 15, true, false, false, false);
		ret.push_back(move);

		attacks = attacks & ones_decrement(attacks);
	}
	in_check_get_pawn_captures(ret, kings_queen_scope, enemy_attacks,pinned, (not_double_check)*checkers);
}

void Position::get_pawn_captures(std::vector<int>& ptr, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned) {
	if (side) {
		legal_bpawn_captures(ptr, kings_queen_scope, enemy_attacks, pinned);
		return;
	}
	legal_wpawn_captures(ptr, kings_queen_scope, enemy_attacks, pinned);
}
void Position::in_check_get_pawn_captures(std::vector<int>& ptr, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets) {
	if (side) {
		in_check_legal_bpawn_captures(ptr, kings_queen_scope, enemy_attacks,pinned, targets);
		return;
	}
	in_check_legal_wpawn_captures(ptr, kings_queen_scope, enemy_attacks, pinned, targets);
}

void Position::get_legal_pawn_moves(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned) {
	if (side) {
		legal_bpawn_pushes(ret, kings_queen_scope, enemy_attacks, pinned);
		legal_bpawn_captures(ret, kings_queen_scope, enemy_attacks, pinned);
		return;
	}
	legal_wpawn_pushes(ret, kings_queen_scope, enemy_attacks, pinned);
	legal_wpawn_captures(ret, kings_queen_scope, enemy_attacks, pinned);
}
void Position::legal_bpawn_pushes(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned) {
	U64 valid_targets = ~occupancies[both];
	U64 promoters = bitboards[6] & (~pinned) & secondRank;
	U64 push_promotions = (promoters << 8) & valid_targets;
	U64 pushes = ((bitboards[6] & ~(pinned | promoters)) << 8) & valid_targets;
	U64 doublePushes = ((pushes << 8) & 0xFF000000ULL) & valid_targets;
	int move;
	while (push_promotions) {
		const U64 isolated = push_promotions & twos_complement(push_promotions);
		int ind = bitscan(isolated);
		move = encode_move(ind - 8, ind, p, 15, n, false, false, false, true);
		ret.push_back(move);
		set_promotion_type(move, b);
		ret.push_back(move);
		set_promotion_type(move, r);
		ret.push_back(move);
		set_promotion_type(move, q);
		ret.push_back(move);
		push_promotions = push_promotions & ones_decrement(push_promotions);
	}
	while (pushes) {
		U64 isolated = pushes & twos_complement(pushes);
		int ind = bitscan(isolated);
		move = encode_move(ind - 8, ind, p, 15, 15, false, false, false, false);
		ret.push_back(move);
		pushes = pushes & ones_decrement(pushes);
	}
	while (doublePushes) {
		U64 isolated = doublePushes & twos_complement(doublePushes);
		int ind = bitscan(isolated);
		move = encode_move(ind - 16, ind, p, 15, 15, false, true, false, false);
		ret.push_back(move);
		doublePushes = doublePushes & ones_decrement(doublePushes);
	}
}
void Position::legal_wpawn_pushes(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned) {
	U64 valid_targets = ~occupancies[both];
	U64 promoters = bitboards[0] & (~pinned) & seventhRank;
	U64 push_promotions = (promoters >> 8) & valid_targets;
	U64 pushes = ((bitboards[0] & ~(pinned | promoters)) >> 8) & valid_targets;
	U64 doublePushes = ((pushes >> 8) & 0xFF00000000ULL) & valid_targets;
	int move;
	while (push_promotions) {
		const U64 isolated = push_promotions & twos_complement(push_promotions);
		int ind = bitscan(isolated);
		move = encode_move(ind + 8, ind, P, 15, N, false, false, false, true);
		ret.push_back(move);
		set_promotion_type(move, B);
		ret.push_back(move);
		set_promotion_type(move, R);
		ret.push_back(move);
		set_promotion_type(move, Q);
		ret.push_back(move);
		push_promotions = push_promotions & ones_decrement(push_promotions);
	}
	while (pushes) {
		U64 isolated = pushes & twos_complement(pushes);
		int ind = bitscan(isolated);

		move = encode_move(ind + 8, ind, P, 15, 15, false, false, false, false);
		ret.push_back(move);
		pushes = pushes & ones_decrement(pushes);
	}
	while (doublePushes) {
		U64 isolated = doublePushes & twos_complement(doublePushes);
		int ind = bitscan(isolated);
		move = encode_move(ind + 16, ind, P, 15, 15, false, true, false, false);
		ret.push_back(move);
		doublePushes = doublePushes & ones_decrement(doublePushes);
	}
}
void Position::legal_bpawn_captures(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned) {
	const U64 targets = occupancies[white];
	U64 promoters = bitboards[6] & (~pinned) & secondRank;
	U64 captures = ((bitboards[6] & ~(pinned | promoters)) << 7) & notHFile & targets;

	U64 enpassant = 0ULL;
	set_bit(enpassant, enpassant_square);
	const bool left_enpassant = ((enpassant >> 7) & notAFile) & (bitboards[6] & ~(pinned | promoters));
	const bool right_enpassant = ((enpassant >> 9) & notHFile) & (bitboards[6] & ~(pinned | promoters));
	if (left_enpassant) {
		int move = encode_move(enpassant_square - 7, enpassant_square, p, P, 15, true, false, false, true);
		try_out_move(ret, move);
	}
	if (right_enpassant) {
		int move = encode_move(enpassant_square - 9, enpassant_square, p, P, 15, true, false, false, true);
		try_out_move(ret, move);
	}

	while (captures) {
		U64 isolated = captures & twos_complement(captures);
		const int ind = bitscan(isolated);
		int move = encode_move(ind - 7, ind, p, get_piece_type_on(ind), 15, true, false, false, false);
		ret.push_back(move);
		captures = captures & ones_decrement(captures);
	}
	captures = ((bitboards[6] & ~(pinned | promoters)) << 9) & notAFile & targets;
	while (captures) {
		U64 isolated = captures & twos_complement(captures);
		const int ind = bitscan(isolated);
		int move = encode_move(ind - 9, ind, p, get_piece_type_on(ind), 15, true, false, false, false);
		ret.push_back(move);
		captures = captures & ones_decrement(captures);
	}

	U64 promotion_captures = ((promoters) << 7) & notHFile & targets;
	while (promotion_captures) {
		U64 isolated = promotion_captures & twos_complement(promotion_captures);
		const int ind = bitscan(isolated);
		int move = encode_move(ind - 7, ind, p, get_piece_type_on(ind), n, true, false, false, false);
		ret.push_back(move);
		set_promotion_type(move, b);
		ret.push_back(move);
		set_promotion_type(move, r);
		ret.push_back(move);
		set_promotion_type(move, q);
		ret.push_back(move);
		promotion_captures = promotion_captures & ones_decrement(promotion_captures);
	}
	promotion_captures = ((promoters) << 9) & notAFile & targets;
	while (promotion_captures) {
		U64 isolated = promotion_captures & twos_complement(promotion_captures);
		const int ind = bitscan(isolated);
		int move = encode_move(ind - 9, ind, p, get_piece_type_on(ind), n, true, false, false, false);
		ret.push_back(move);
		set_promotion_type(move, b);
		ret.push_back(move);
		set_promotion_type(move, r);
		ret.push_back(move);
		set_promotion_type(move, q);
		ret.push_back(move);
		promotion_captures = promotion_captures & ones_decrement(promotion_captures);
	}
}
void Position::legal_wpawn_captures(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned) {
	U64 promoters = bitboards[0] & (~pinned) & seventhRank;
	const U64 targets = occupancies[black];
	U64 captures = ((bitboards[0] & ~(pinned | promoters)) >> 7) & notAFile & targets;


	U64 enpassant = 0ULL;
	enpassant = (enpassant_square != a8) * (1ULL << enpassant_square);
	const bool left_enpassant = ((enpassant << 7) & notHFile) & (bitboards[0] & ~(pinned | promoters));
	const bool right_enpassant = ((enpassant << 9) & notAFile) & (bitboards[0] & ~(pinned | promoters));
	if (left_enpassant) {
		int move = encode_move(enpassant_square + 7, enpassant_square, P, p, 15, true, false, false, true);
		try_out_move(ret, move);
	}
	if (right_enpassant) {
		int move = encode_move(enpassant_square + 9, enpassant_square, P, p, 15, true, false, false, true);
		try_out_move(ret, move);
	}

	while (captures) {
		U64 isolated = captures & twos_complement(captures);
		const int ind = bitscan(isolated);
		int move = encode_move(ind + 7, ind, P, get_piece_type_on(ind), 15, true, false, false, false);
		ret.push_back(move);
		captures = captures & ones_decrement(captures);
	}
	captures = ((bitboards[0] & ~(pinned | promoters)) >> 9) & notHFile & targets;
	while (captures) {
		U64 isolated = captures & twos_complement(captures);
		const int ind = bitscan(isolated);
		int move = encode_move(ind + 9, ind, P, get_piece_type_on(ind), 15, true, false, false, false);
		ret.push_back(move);
		captures = captures & ones_decrement(captures);
	}
	U64 promotion_captures = ((promoters) >> 7) & notAFile & targets;
	while (promotion_captures) {
		U64 isolated = promotion_captures & twos_complement(promotion_captures);
		const int ind = bitscan(isolated);
		int move = encode_move(ind + 7, ind, P, get_piece_type_on(ind), N, true, false, false, false);
		ret.push_back(move);
		set_promotion_type(move, B);
		ret.push_back(move);
		set_promotion_type(move, R);
		ret.push_back(move);
		set_promotion_type(move, Q);
		ret.push_back(move);
		promotion_captures = promotion_captures & ones_decrement(promotion_captures);
	}
	promotion_captures = ((promoters) >> 9) & notHFile & targets;
	while (promotion_captures) {
		U64 isolated = promotion_captures & twos_complement(promotion_captures);
		const int ind = bitscan(isolated);
		int move = encode_move(ind + 9, ind, P, get_piece_type_on(ind), N, true, false, false, false);
		ret.push_back(move);
		set_promotion_type(move, B);
		ret.push_back(move);
		set_promotion_type(move, R);
		ret.push_back(move);
		set_promotion_type(move, Q);
		ret.push_back(move);
		promotion_captures = promotion_captures & ones_decrement(promotion_captures);
	}
}

void Position::in_check_get_legal_pawn_moves(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, const U64 in_check_valid) {
	if (side) {
		in_check_legal_bpawn_pushes(ret, kings_queen_scope, enemy_attacks, pinned, targets, in_check_valid);
		in_check_legal_bpawn_captures(ret, kings_queen_scope, enemy_attacks, pinned, targets);
		return;
	}
	in_check_legal_wpawn_pushes(ret, kings_queen_scope, enemy_attacks, pinned, targets, in_check_valid);
	in_check_legal_wpawn_captures(ret, kings_queen_scope, enemy_attacks, pinned, targets);
}
void Position::in_check_legal_bpawn_pushes(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, const U64 in_check_valid) {
	const U64 valid_targets = ~occupancies[both];
	U64 promoters = bitboards[6] & (~pinned) & secondRank;
	U64 push_promotions = (promoters << 8) & in_check_valid & valid_targets;
	U64 pushes = ((bitboards[6] & ~(pinned | promoters)) << 8) & valid_targets;
	U64 doublePushes = ((pushes << 8) & 0xFF000000ULL) & in_check_valid & valid_targets;
	pushes = pushes & in_check_valid & valid_targets;
	int move;
	while (push_promotions) {
		const U64 isolated = push_promotions & twos_complement(push_promotions);
		int ind = bitscan(isolated);
		move = encode_move(ind - 8, ind, p, 15, n, false, false, false, false);
		ret.push_back(move);
		set_promotion_type(move, b);
		ret.push_back(move);
		set_promotion_type(move, r);
		ret.push_back(move);
		set_promotion_type(move, q);
		ret.push_back(move);
		push_promotions = push_promotions & ones_decrement(push_promotions);
	}
	while (pushes) {
		U64 isolated = pushes & twos_complement(pushes);
		int ind = bitscan(isolated);
		move = encode_move(ind - 8, ind, p, 15, 15, false, false, false, false);
		ret.push_back(move);
		pushes = pushes & ones_decrement(pushes);
	}
	while (doublePushes) {
		U64 isolated = doublePushes & twos_complement(doublePushes);
		int ind = bitscan(isolated);
		move = encode_move(ind - 16, ind, p, 15, 15, false, true, false, false);
		ret.push_back(move);
		doublePushes = doublePushes & ones_decrement(doublePushes);
	}
}
void Position::in_check_legal_wpawn_pushes(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, const U64 in_check_valid) {
	U64 valid_targets = ~occupancies[both];
	U64 promoters = bitboards[0] & (~pinned) & seventhRank;
	U64 push_promotions = (promoters >> 8) & in_check_valid & valid_targets;
	U64 pushes = ((bitboards[0] & ~(pinned | promoters)) >> 8) & valid_targets;
	U64 doublePushes = ((pushes >> 8) & 0xFF00000000ULL) & in_check_valid & valid_targets;
	pushes = pushes & in_check_valid & valid_targets;
	int move;
	while (push_promotions) {
		const U64 isolated = push_promotions & twos_complement(push_promotions);
		int ind = bitscan(isolated);
		move = encode_move(ind + 8, ind, P, 15, N, false, false, false, false);
		ret.push_back(move);
		set_promotion_type(move, B);
		ret.push_back(move);
		set_promotion_type(move, R);
		ret.push_back(move);
		set_promotion_type(move, Q);
		ret.push_back(move);
		push_promotions = push_promotions & ones_decrement(push_promotions);
	}
	while (pushes) {
		U64 isolated = pushes & twos_complement(pushes);
		int ind = bitscan(isolated);

		move = encode_move(ind + 8, ind, P, 15, 15, false, false, false, false);
		ret.push_back(move);
		pushes = pushes & ones_decrement(pushes);
	}
	while (doublePushes) {
		U64 isolated = doublePushes & twos_complement(doublePushes);
		int ind = bitscan(isolated);
		move = encode_move(ind + 16, ind, P, 15, 15, false, true, false, false);
		ret.push_back(move);
		doublePushes = doublePushes & ones_decrement(doublePushes);
	}
}
void Position::in_check_legal_bpawn_captures(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets) {
	U64 promoters = bitboards[6] & (~pinned) & secondRank;
	U64 captures = ((bitboards[6] & (~(pinned | promoters))) << 7) & notHFile & targets;

	U64 enpassant = 0ULL;
	set_bit(enpassant, enpassant_square);
	const bool left_enpassant = ((enpassant >> 7) & notAFile) & (bitboards[6] & ~(pinned | promoters));
	const bool right_enpassant = ((enpassant >> 9) & notHFile) & (bitboards[6] & ~(pinned | promoters));
	if (left_enpassant) {
		int move = encode_move(enpassant_square - 7, enpassant_square, p, P, 15, true, false, false, true);
		try_out_move(ret, move);
	}
	if (right_enpassant) {
		int move = encode_move(enpassant_square - 9, enpassant_square, p, P, 15, true, false, false, true);
		try_out_move(ret, move);
	}

	while (captures) {
		U64 isolated = captures & twos_complement(captures);
		const int ind = bitscan(isolated);
		int move = encode_move(ind - 7, ind, p, get_piece_type_on(ind), 15, true, false, false, false);
		ret.push_back(move);
		captures = captures & ones_decrement(captures);
	}
	captures = ((bitboards[6] & ~(pinned | promoters)) << 9) & notAFile & targets;
	while (captures) {
		U64 isolated = captures & twos_complement(captures);
		const int ind = bitscan(isolated);
		int move = encode_move(ind - 9, ind, p, get_piece_type_on(ind), 15, true, false, false, false);
		ret.push_back(move);
		captures = captures & ones_decrement(captures);
	}

	U64 promotion_captures = ((promoters) << 7) & notHFile & targets;
	while (promotion_captures) {
		U64 isolated = promotion_captures & twos_complement(promotion_captures);
		const int ind = bitscan(isolated);
		int move = encode_move(ind - 7, ind, p, get_piece_type_on(ind), n, true, false, false, false);
		ret.push_back(move);
		set_promotion_type(move, b);
		ret.push_back(move);
		set_promotion_type(move, r);
		ret.push_back(move);
		set_promotion_type(move, q);
		ret.push_back(move);
		promotion_captures = promotion_captures & ones_decrement(promotion_captures);
	}
	promotion_captures = ((promoters) << 9) & notAFile & targets;
	while (promotion_captures) {
		U64 isolated = promotion_captures & twos_complement(promotion_captures);
		const int ind = bitscan(isolated);
		int move = encode_move(ind - 9, ind, p, get_piece_type_on(ind), n, true, false, false, false);
		ret.push_back(move);
		set_promotion_type(move, b);
		ret.push_back(move);
		set_promotion_type(move, r);
		ret.push_back(move);
		set_promotion_type(move, q);
		ret.push_back(move);
		promotion_captures = promotion_captures & ones_decrement(promotion_captures);
	}
}
void Position::in_check_legal_wpawn_captures(std::vector<int>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets) {
	U64 promoters = bitboards[0] & (~pinned) & seventhRank;
	U64 captures = ((bitboards[0] & (~(pinned | promoters))) >> 7) & notAFile & targets;

	U64 enpassant = (enpassant_square != a8) * (1ULL << enpassant_square);
	const bool left_enpassant = ((enpassant << 7) & notHFile) & (bitboards[0] & ~(pinned | promoters));
	const bool right_enpassant = ((enpassant << 9) & notAFile) & (bitboards[0] & ~(pinned | promoters));
	if (left_enpassant) {
		int move = encode_move(enpassant_square + 7, enpassant_square, P, p, 15, true, false, false, true);
		try_out_move(ret, move);
	}
	if (right_enpassant) {
		int move = encode_move(enpassant_square + 9, enpassant_square, P, p, 15, true, false, false, true);
		try_out_move(ret, move);
	}

	while (captures) {
		U64 isolated = captures & twos_complement(captures);
		const int ind = bitscan(isolated);
		int move = encode_move(ind + 7, ind, P, get_piece_type_on(ind), 15, true, false, false, false);
		ret.push_back(move);
		captures = captures & ones_decrement(captures);
	}
	captures = ((bitboards[0] & ~(pinned | promoters)) >> 9) & notHFile & targets;
	while (captures) {
		U64 isolated = captures & twos_complement(captures);
		const int ind = bitscan(isolated);
		int move = encode_move(ind + 9, ind, P, get_piece_type_on(ind), 15, true, false, false, false);
		ret.push_back(move);
		captures = captures & ones_decrement(captures);
	}
	U64 promotion_captures = ((promoters) >> 7) & notAFile & targets;
	while (promotion_captures) {
		U64 isolated = promotion_captures & twos_complement(promotion_captures);
		const int ind = bitscan(isolated);
		int move = encode_move(ind + 7, ind, P, get_piece_type_on(ind), N, true, false, false, false);
		ret.push_back(move);
		set_promotion_type(move, B);
		ret.push_back(move);
		set_promotion_type(move, R);
		ret.push_back(move);
		set_promotion_type(move, Q);
		ret.push_back(move);
		promotion_captures = promotion_captures & ones_decrement(promotion_captures);
	}
	promotion_captures = ((promoters) >> 9) & notHFile & targets;
	while (promotion_captures) {
		U64 isolated = promotion_captures & twos_complement(promotion_captures);
		const int ind = bitscan(isolated);
		int move = encode_move(ind + 9, ind, P, get_piece_type_on(ind), N, true, false, false, false);
		ret.push_back(move);
		set_promotion_type(move, B);
		ret.push_back(move);
		set_promotion_type(move, R);
		ret.push_back(move);
		set_promotion_type(move, Q);
		ret.push_back(move);
		promotion_captures = promotion_captures & ones_decrement(promotion_captures);
	}
}

void Position::get_castles(std::vector<int>& const ptr) {
	const U64 empty = ~(occupancies[both]);

	const int king_index = 5 + (side) * 6;
	const int kingpos = bitscan(bitboards[king_index]);
	const bool king_on_valid_square = (side) ? (kingpos == e8) : (kingpos == e1);

	const int enemy_color = (side) ? white : black;

	const bool king_in_valid_state = king_on_valid_square && (!is_attacked_by_side(kingpos, enemy_color));
	bool queenside = king_in_valid_state && (get_bit(castling_rights, 1 + 2 * (side)));
	bool kingside = king_in_valid_state && (get_bit(castling_rights, 2 * (side)));

	const int square_offset = 56 * (side);

	queenside &= (((bitboards[king_index]) >> 1) & empty) && (!is_attacked_by_side(d1 - square_offset, enemy_color));
	queenside &= (((bitboards[king_index]) >> 2) & empty) && (!is_attacked_by_side(c1 - square_offset, enemy_color));
	queenside &= (bool)(((bitboards[king_index]) >> 3) & empty);

	int move = encode_move(kingpos, kingpos - 2, king_index, 15, 15, false, false, true, false);

	if (queenside) {
		ptr.push_back(move);
	}

	kingside &= (((bitboards[king_index]) << 1) & empty) && (!is_attacked_by_side(f1 - square_offset, enemy_color));
	kingside &= (((bitboards[king_index]) << 2) & empty) && (!is_attacked_by_side(g1 - square_offset, enemy_color));
	set_to_square(move, kingpos + 2);
	if (kingside) {
		ptr.push_back(move);
	}
}
U64 Position::get_pinned_pieces(const int& kingpos, const U64& enemy_attacks) {
	U64 potentially_pinned_by_bishops = enemy_attacks & get_bishop_attacks(occupancies[both], kingpos) & occupancies[(side)];
	U64 potentially_pinned_by_rooks = enemy_attacks & get_rook_attacks(occupancies[both], kingpos) & occupancies[(side)];
	U64 pinned_pieces = 0ULL;
	const int offset = 6 * (!side);
	while (potentially_pinned_by_bishops) {
		const U64 isolated = potentially_pinned_by_bishops & twos_complement(potentially_pinned_by_bishops);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_pinner = get_bishop_attacks(occupancies[both], kingpos) & (bitboards[B + offset] | bitboards[Q + offset]);
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		pinned_pieces |= (((bool)(pot_pinner)) & ((bool)(isolated & get_bishop_attacks(occupancies[both], static_cast<unsigned long long>(bitscan(pot_pinner)))))) * isolated;
		potentially_pinned_by_bishops = potentially_pinned_by_bishops & ones_decrement(potentially_pinned_by_bishops);
	}
	while (potentially_pinned_by_rooks) {
		const U64 isolated = potentially_pinned_by_rooks & twos_complement(potentially_pinned_by_rooks);
		occupancies[both] &= ~isolated;//pop bit of piece from occupancies
		U64 pot_pinner = get_rook_attacks(occupancies[both], kingpos) & (bitboards[R + offset] | bitboards[Q + offset]);
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		pinned_pieces |= (((bool)(pot_pinner)) & ((bool)(isolated & get_rook_attacks(occupancies[both], static_cast<unsigned long long>(bitscan(pot_pinner)))))) * isolated;

		potentially_pinned_by_rooks = potentially_pinned_by_rooks & ones_decrement(potentially_pinned_by_rooks);
	}
	return pinned_pieces;
}
U64 Position::get_moves_for_pinned_pieces(std::vector<int>& ret, const int kingpos, const U64 enemy_attacks) {
	const int piece_offset = 6 * (side);
	const U64 bishop_attacks = get_bishop_attacks(occupancies[both], kingpos);
	const U64 rook_attacks = get_rook_attacks(occupancies[both], kingpos);
	U64 bishops_pot_pinned_by_bishops = enemy_attacks & bishop_attacks & bitboards[B + piece_offset];
	U64 bishops_pot_pinned_by_rooks = enemy_attacks & rook_attacks & bitboards[B + piece_offset];

	U64 queens_pot_pinned_by_bishops = enemy_attacks & bishop_attacks & bitboards[Q + piece_offset];
	U64 queens_pot_pinned_by_rooks = enemy_attacks & rook_attacks & bitboards[Q + piece_offset];

	U64 rooks_pot_pinned_by_bishops = enemy_attacks & bishop_attacks & bitboards[R + piece_offset];
	U64 rooks_pot_pinned_by_rooks = enemy_attacks & rook_attacks & bitboards[R + piece_offset];

	U64 pawns_pot_pinned_by_bishops = enemy_attacks & bishop_attacks & bitboards[P + piece_offset];
	U64 pawns_pot_pinned_by_rooks = enemy_attacks & rook_attacks & bitboards[P + piece_offset];

	U64 knights_pot_pinned_by_bishops = enemy_attacks & bishop_attacks & bitboards[N + piece_offset];
	U64 knights_pot_pinned_by_rooks = enemy_attacks & rook_attacks & bitboards[N + piece_offset];

	U64 pinned_pieces = 0ULL;

	const int offset = 6 * (!side);

	const U64 rook_and_queen_mask=bitboards[R + offset] | bitboards[Q + offset];
	const U64 bishop_and_queen_mask= bitboards[B + offset] | bitboards[Q + offset];

	int type = B + piece_offset;
	while (bishops_pot_pinned_by_bishops) {
		const U64 isolated = bishops_pot_pinned_by_bishops & twos_complement(bishops_pot_pinned_by_bishops);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_bishop_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & bishop_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			const int from = bitscan(isolated);
			const int to = bitscan(pinner);
			int move = encode_move(from, to, type, get_piece_type_on(to), 15, true, false, false, false);
			ret.push_back(move);
			U64 attacks = (pot_attacks & get_bishop_attacks(occupancies[both], to)) & (~isolated);
			while (attacks) {
				const U64 isolated2 = attacks & twos_complement(attacks);
				move = encode_move(from, bitscan(isolated2), type, 15, 15, false, false, false, false);
				ret.push_back(move);
				attacks = attacks & ones_decrement(attacks);
			}
		}
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		bishops_pot_pinned_by_bishops = bishops_pot_pinned_by_bishops & ones_decrement(bishops_pot_pinned_by_bishops);
	}
	while (bishops_pot_pinned_by_rooks) {
		const U64 isolated = bishops_pot_pinned_by_rooks & twos_complement(bishops_pot_pinned_by_rooks);
		occupancies[both] &= ~isolated;//pop bit of piece from occupancies
		pinned_pieces |= ((bool)(get_rook_attacks(occupancies[both], kingpos) & rook_and_queen_mask)) * isolated;
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		bishops_pot_pinned_by_rooks = bishops_pot_pinned_by_rooks & ones_decrement(bishops_pot_pinned_by_rooks);
	}

	type = Q + piece_offset;
	while (queens_pot_pinned_by_bishops) {
		const U64 isolated = queens_pot_pinned_by_bishops & twos_complement(queens_pot_pinned_by_bishops);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_bishop_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & bishop_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			const int from = bitscan(isolated);
			const int to = bitscan(pinner);
			int move = encode_move(from, to, type, get_piece_type_on(to), 15, true, false, false, false);
			ret.push_back(move);
			U64 attacks = (pot_attacks & get_bishop_attacks(occupancies[both], to)) & (~isolated);
			while (attacks) {
				const U64 isolated2 = attacks & twos_complement(attacks);
				move = encode_move(from, bitscan(isolated2), type, 15, 15, false, false, false, false);
				ret.push_back(move);
				attacks = attacks & ones_decrement(attacks);
			}
		}
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		queens_pot_pinned_by_bishops = queens_pot_pinned_by_bishops & ones_decrement(queens_pot_pinned_by_bishops);
	}

	while (queens_pot_pinned_by_rooks) {
		const U64 isolated = queens_pot_pinned_by_rooks & twos_complement(queens_pot_pinned_by_rooks);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_rook_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & rook_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			const int from = bitscan(isolated);
			const int to = bitscan(pinner);
			int move = encode_move(from, to, type, get_piece_type_on(to), 15, true, false, false, false);
			ret.push_back(move);
			U64 attacks = (pot_attacks & get_rook_attacks(occupancies[both], to)) & (~isolated);
			while (attacks) {
				const U64 isolated2 = attacks & twos_complement(attacks);
				move = encode_move(from, bitscan(isolated2), type, 15, 15, false, false, false, false);
				ret.push_back(move);
				attacks = attacks & ones_decrement(attacks);
			}
		}
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		queens_pot_pinned_by_rooks = queens_pot_pinned_by_rooks & ones_decrement(queens_pot_pinned_by_rooks);
	}
	type = R + piece_offset;
	while (rooks_pot_pinned_by_rooks) {
		const U64 isolated = rooks_pot_pinned_by_rooks & twos_complement(rooks_pot_pinned_by_rooks);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_rook_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & rook_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			const int from = bitscan(isolated);
			const int to = bitscan(pinner);
			int move = encode_move(from, to, type, get_piece_type_on(to), 15, true, false, false, false);
			ret.push_back(move);
			U64 attacks = (pot_attacks & get_rook_attacks(occupancies[both], to)) & (~isolated);
			while (attacks) {
				const U64 isolated2 = attacks & twos_complement(attacks);
				move = encode_move(from, bitscan(isolated2), type, 15, 15, false, false, false, false);
				ret.push_back(move);
				attacks = attacks & ones_decrement(attacks);
			}
		}
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		rooks_pot_pinned_by_rooks = rooks_pot_pinned_by_rooks & ones_decrement(rooks_pot_pinned_by_rooks);
	}

	while (rooks_pot_pinned_by_bishops) {
		const U64 isolated = rooks_pot_pinned_by_bishops & twos_complement(rooks_pot_pinned_by_bishops);
		occupancies[both] &= ~isolated;//pop bit of piece from occupancies
		pinned_pieces |= ((bool)(get_bishop_attacks(occupancies[both], kingpos) & bishop_and_queen_mask)) * isolated;
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		rooks_pot_pinned_by_bishops = rooks_pot_pinned_by_bishops & ones_decrement(rooks_pot_pinned_by_bishops);
	}
	type = P + piece_offset;
	U64 enpassant = 0ULL;
	enpassant = (enpassant_square != a8) * (enpassant | (1ULL << enpassant_square));
	while (pawns_pot_pinned_by_bishops) {
		const U64 isolated = pawns_pot_pinned_by_bishops & twos_complement(pawns_pot_pinned_by_bishops);
		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_bishop_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & bishop_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			const int from = bitscan(isolated);
			const int to = bitscan(pinner);
			U64 attacks = pawn_attacks[(side)][from];
			const bool is_enpassant = attacks & enpassant;
			if (is_enpassant) {
				int move = encode_move(from, enpassant_square, type, get_piece_type_on(P + offset), 15, true, false, false, true);
				ret.push_back(move);
			}
			else if (attacks & pinner) {
				int move = encode_move(from, to, type, get_piece_type_on(to), 15, true, false, false, false);
				if ((side && (to < a7)) || ((!side) && (to > h2))) {
					set_promotion_type(move, N + piece_offset);
					ret.push_back(move);
					set_promotion_type(move, B + piece_offset);
					ret.push_back(move);
					set_promotion_type(move, R + piece_offset);
					ret.push_back(move);
					set_promotion_type(move, Q + piece_offset);
				}
				ret.push_back(move);
			}
		}
		occupancies[both] |= isolated;//reset bit of piece from occupancies

		pawns_pot_pinned_by_bishops = pawns_pot_pinned_by_bishops & ones_decrement(pawns_pot_pinned_by_bishops);
	}

	while (pawns_pot_pinned_by_rooks) {
		const U64 isolated = pawns_pot_pinned_by_rooks & twos_complement(pawns_pot_pinned_by_rooks);
		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_rook_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & rook_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			const int from = bitscan(isolated);
			const int to = bitscan(pinner);
			U64 valid_targets = (~occupancies[both]) & pot_attacks;
			const int push_target = from + 8 * ((side)-(!side));
			const int double_push_target = from + 16 * ((side)-(!side));
			if (get_bit(valid_targets, push_target)) {
				int move = encode_move(from, push_target, type, 15, 15, false, false, false, false);
				if ((push_target < a7) || (push_target > h2)) {
					set_promotion_type(move, N + piece_offset);
					ret.push_back(move);
					set_promotion_type(move, B + piece_offset);
					ret.push_back(move);
					set_promotion_type(move, R + piece_offset);
					ret.push_back(move);
					set_promotion_type(move, Q + piece_offset);
				}
				ret.push_back(move);
			}
			if ((double_push_target < a3) && (double_push_target > h6)) {
				if (get_bit(valid_targets & (((!side) * 0xFF00000000ULL) | ((side) * 0xFF000000ULL)), double_push_target)) {
					int move = encode_move(from, double_push_target, type, 15, 15, false, true, false, false);
					ret.push_back(move);
				}
			}
		}
		occupancies[both] |= isolated;//reset bit of piece from occupancies

		pawns_pot_pinned_by_rooks = pawns_pot_pinned_by_rooks & ones_decrement(pawns_pot_pinned_by_rooks);
	}

	while (knights_pot_pinned_by_bishops) {
		const U64 isolated = knights_pot_pinned_by_bishops & twos_complement(knights_pot_pinned_by_bishops);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		pinned_pieces |= ((bool)(get_bishop_attacks(occupancies[both], kingpos) & bishop_and_queen_mask)) * isolated;
		occupancies[both] |= isolated;//reset bit of piece from occupancies

		knights_pot_pinned_by_bishops = knights_pot_pinned_by_bishops & ones_decrement(knights_pot_pinned_by_bishops);
	}

	while (knights_pot_pinned_by_rooks) {
		const U64 isolated = knights_pot_pinned_by_rooks & twos_complement(knights_pot_pinned_by_rooks);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancies
		pinned_pieces |= ((bool)(get_rook_attacks(occupancies[both], kingpos) & rook_and_queen_mask)) * isolated;
		occupancies[both] |= isolated;//reset bit of piece from occupancies

		knights_pot_pinned_by_rooks = knights_pot_pinned_by_rooks & ones_decrement(knights_pot_pinned_by_rooks);
	}
	return pinned_pieces;
}
U64 Position::get_captures_for_pinned_pieces(std::vector<int>& ret, const int kingpos, const U64 enemy_attacks) {
	const int piece_offset = 6 * (side);
	const U64 bishop_attacks = get_bishop_attacks(occupancies[both], kingpos);
	const U64 rook_attacks = get_rook_attacks(occupancies[both], kingpos);
	U64 bishops_pot_pinned_by_bishops = enemy_attacks & bishop_attacks & bitboards[B + piece_offset];
	U64 bishops_pot_pinned_by_rooks = enemy_attacks & rook_attacks & bitboards[B + piece_offset];

	U64 queens_pot_pinned_by_bishops = enemy_attacks & bishop_attacks & bitboards[Q + piece_offset];
	U64 queens_pot_pinned_by_rooks = enemy_attacks & rook_attacks & bitboards[Q + piece_offset];

	U64 rooks_pot_pinned_by_bishops = enemy_attacks & bishop_attacks & bitboards[R + piece_offset];
	U64 rooks_pot_pinned_by_rooks = enemy_attacks & rook_attacks & bitboards[R + piece_offset];

	U64 pawns_pot_pinned_by_bishops = enemy_attacks & bishop_attacks & bitboards[P + piece_offset];
	U64 pawns_pot_pinned_by_rooks = enemy_attacks & rook_attacks & bitboards[P + piece_offset];

	U64 knights_pot_pinned_by_bishops = enemy_attacks & bishop_attacks & bitboards[N + piece_offset];
	U64 knights_pot_pinned_by_rooks = enemy_attacks & rook_attacks & bitboards[N + piece_offset];

	U64 pinned_pieces = 0ULL;

	const int offset = 6 * (!side);
	const U64 bishop_and_queen_mask = bitboards[B + offset] | bitboards[Q + offset];
	const U64 rook_and_queen_mask = bitboards[R + offset] | bitboards[Q + offset];
	int type = B + piece_offset;
	while (bishops_pot_pinned_by_bishops) {
		const U64 isolated = bishops_pot_pinned_by_bishops & twos_complement(bishops_pot_pinned_by_bishops);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_bishop_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & bishop_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			const int from = bitscan(isolated);
			const int to = bitscan(pinner);
			int move = encode_move(from, to, type, get_piece_type_on(to), 15, true, false, false, false);
			ret.push_back(move);
		}
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		bishops_pot_pinned_by_bishops = bishops_pot_pinned_by_bishops & ones_decrement(bishops_pot_pinned_by_bishops);
	}
	while (bishops_pot_pinned_by_rooks) {
		const U64 isolated = bishops_pot_pinned_by_rooks & twos_complement(bishops_pot_pinned_by_rooks);
		occupancies[both] &= ~isolated;//pop bit of piece from occupancies
		pinned_pieces |= ((bool)(get_rook_attacks(occupancies[both], kingpos) & rook_and_queen_mask)) * isolated;
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		bishops_pot_pinned_by_rooks = bishops_pot_pinned_by_rooks & ones_decrement(bishops_pot_pinned_by_rooks);
	}

	type = Q + piece_offset;
	while (queens_pot_pinned_by_bishops) {
		const U64 isolated = queens_pot_pinned_by_bishops & twos_complement(queens_pot_pinned_by_bishops);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_bishop_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & bishop_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			const int from = bitscan(isolated);
			const int to = bitscan(pinner);
			int move = encode_move(from, to, type, get_piece_type_on(to), 15, true, false, false, false);
			ret.push_back(move);
		}
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		queens_pot_pinned_by_bishops = queens_pot_pinned_by_bishops & ones_decrement(queens_pot_pinned_by_bishops);
	}
	while (queens_pot_pinned_by_rooks) {
		const U64 isolated = queens_pot_pinned_by_rooks & twos_complement(queens_pot_pinned_by_rooks);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_rook_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & rook_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			const int from = bitscan(isolated);
			const int to = bitscan(pinner);
			int move = encode_move(from, to, type, get_piece_type_on(to), 15, true, false, false, false);
			ret.push_back(move);
		}
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		queens_pot_pinned_by_rooks = queens_pot_pinned_by_rooks & ones_decrement(queens_pot_pinned_by_rooks);
	}
	type = R + piece_offset;
	while (rooks_pot_pinned_by_rooks) {
		const U64 isolated = rooks_pot_pinned_by_rooks & twos_complement(rooks_pot_pinned_by_rooks);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_rook_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & rook_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			const int from = bitscan(isolated);
			const int to = bitscan(pinner);
			int move = encode_move(from, to, type, get_piece_type_on(to), 15, true, false, false, false);
			ret.push_back(move);
		}
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		rooks_pot_pinned_by_rooks = rooks_pot_pinned_by_rooks & ones_decrement(rooks_pot_pinned_by_rooks);
	}
	while (rooks_pot_pinned_by_bishops) {
		const U64 isolated = rooks_pot_pinned_by_bishops & twos_complement(rooks_pot_pinned_by_bishops);
		occupancies[both] &= ~isolated;//pop bit of piece from occupancies
		pinned_pieces |= ((bool)(get_bishop_attacks(occupancies[both], kingpos) & bishop_and_queen_mask)) * isolated;
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		rooks_pot_pinned_by_bishops = rooks_pot_pinned_by_bishops & ones_decrement(rooks_pot_pinned_by_bishops);
	}
	type = P + piece_offset;
	U64 enpassant = 0ULL;
	enpassant = (enpassant_square != a8) * (enpassant | (1ULL << enpassant_square));
	while (pawns_pot_pinned_by_bishops) {
		const U64 isolated = pawns_pot_pinned_by_bishops & twos_complement(pawns_pot_pinned_by_bishops);
		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_bishop_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & bishop_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			const int from = bitscan(isolated);
			const int to = bitscan(pinner);
			U64 attacks = pawn_attacks[(side)][from];
			const bool is_enpassant = attacks & enpassant;
			if (is_enpassant) {
				int move = encode_move(from, enpassant_square, type, get_piece_type_on(P + offset), 15, true, false, false, true);
				ret.push_back(move);
			}
			else if (attacks & pinner) {
				int move = encode_move(from, to, type, get_piece_type_on(to), 15, true, false, false, false);
				if ((side && (to < a7)) || ((!side) && (to > h2))) {
					set_promotion_type(move, N + piece_offset);
					ret.push_back(move);
					set_promotion_type(move, B + piece_offset);
					ret.push_back(move);
					set_promotion_type(move, R + piece_offset);
					ret.push_back(move);
					set_promotion_type(move, Q + piece_offset);
				}
				ret.push_back(move);
			}
		}
		occupancies[both] |= isolated;//reset bit of piece from occupancies

		pawns_pot_pinned_by_bishops = pawns_pot_pinned_by_bishops & ones_decrement(pawns_pot_pinned_by_bishops);
	}
	while (pawns_pot_pinned_by_rooks) {
		const U64 isolated = pawns_pot_pinned_by_rooks & twos_complement(pawns_pot_pinned_by_rooks);
		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_rook_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & rook_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		occupancies[both] |= isolated;//reset bit of piece from occupancies

		pawns_pot_pinned_by_rooks = pawns_pot_pinned_by_rooks & ones_decrement(pawns_pot_pinned_by_rooks);
	}


	while (knights_pot_pinned_by_bishops) {
		const U64 isolated = knights_pot_pinned_by_bishops & twos_complement(knights_pot_pinned_by_bishops);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		pinned_pieces |= ((bool)(get_bishop_attacks(occupancies[both], kingpos) & bishop_and_queen_mask)) * isolated;
		occupancies[both] |= isolated;//reset bit of piece from occupancies

		knights_pot_pinned_by_bishops = knights_pot_pinned_by_bishops & ones_decrement(knights_pot_pinned_by_bishops);
	}
	while (knights_pot_pinned_by_rooks) {
		const U64 isolated = knights_pot_pinned_by_rooks & twos_complement(knights_pot_pinned_by_rooks);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancies
		pinned_pieces |= ((bool)(get_rook_attacks(occupancies[both], kingpos) & rook_and_queen_mask)) * isolated;
		occupancies[both] |= isolated;//reset bit of piece from occupancies

		knights_pot_pinned_by_rooks = knights_pot_pinned_by_rooks & ones_decrement(knights_pot_pinned_by_rooks);
	}
	return pinned_pieces;
}
U64 Position::get_checkers(const int kingpos) {
	const int offset = 6 * (!side);
	return (get_bishop_attacks(occupancies[both], kingpos) & (bitboards[B + offset] | bitboards[Q + offset])) | (get_rook_attacks(occupancies[both], kingpos) & (bitboards[R + offset] | bitboards[Q + offset])) | (knight_attacks[kingpos] & bitboards[N + offset]) | (pawn_attacks[(side)][kingpos] & bitboards[P + offset]);
}
U64 Position::get_checking_rays(const int kingpos) {
	const int offset = 6 * (!side);
	const U64 kings_rook_scope = get_rook_attacks(occupancies[both], kingpos);
	const U64 kings_bishop_scope = get_bishop_attacks(occupancies[both], kingpos);
	const U64 checking_rooks = kings_rook_scope & (bitboards[R + offset] | bitboards[Q + offset]);
	const U64 checking_bishops = kings_bishop_scope & (bitboards[B + offset] | bitboards[Q + offset]);

	return (((bool)(checking_rooks)) * (checking_rooks | (get_rook_attacks(occupancies[both], bitscan(checking_rooks)) & kings_rook_scope))) | (((bool)(checking_bishops)) * (checking_bishops | (get_bishop_attacks(occupancies[both], bitscan(checking_bishops)) & kings_bishop_scope)));
}

Position::Position() {
	bitboards = std::array<U64, 12>{ { 71776119061217280ULL, 4755801206503243776ULL, 2594073385365405696ULL, 9295429630892703744ULL, 576460752303423488ULL, 1152921504606846976ULL, 65280ULL, 66ULL, 36ULL, 129ULL, 8ULL, 16ULL } };
	occupancies = std::array<U64, 3>{ { 18446462598732840960ULL, 65535ULL, 18446462598732906495ULL} };
	side = white;
	ply = 1;
	enpassant_square = a8;
	castling_rights = 15;
	no_pawns_or_captures = 0;
	current_hash = get_hash();
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
Position::Position(const std::string& fen) {
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
		if (strList[1] == "w") {
			side = white;
		}
		else if (strList[1] == "b") {
			side = black;
		}
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
	ply = 2 * stoi(fen) - (side == white);
	occupancies[0] = bitboards[0] | bitboards[1] | bitboards[2] | bitboards[3] | bitboards[4] | bitboards[5];
	occupancies[1] = bitboards[6] | bitboards[7] | bitboards[8] | bitboards[9] | bitboards[10] | bitboards[11];
	occupancies[2] = occupancies[0] | occupancies[1];
	no_pawns_or_captures_history.push_back(no_pawns_or_captures);
	//castling_rights_history.push_back(0);
	castling_rights_history.push_back(castling_rights);
	//enpassant_history.push_back(a8);
	enpassant_history.push_back(enpassant_square);
	current_hash = get_hash();
	hash_history.push_back(current_hash);
}
void Position::print() const {
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
	printf("    current game turn: %d\n", (int)ply / 2 + (side == white));
	std::cout << "    current hash: " << current_hash << "\n";
}
U64 Position::get_hash() const {

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
void Position::update_hash(const int move) {
	const bool capture = get_capture_flag(move);
	const bool is_enpassant = get_enpassant_flag(move);
	const bool is_castle = get_castling_flag(move);
	const bool is_double_push = get_double_push_flag(move);

	const int piece_type = get_piece_type(move);
	const int from_square = get_from_square(move);
	const int to_square = get_to_square(move);
	const int captured_type = (capture)*get_captured_type(move);
	const int promoted_type = get_promotion_type(move);


	const int offset = piece_type * 64;

	current_hash ^= keys[offset + from_square];
	current_hash ^= (capture) * ((is_enpassant) ? keys[to_square + (side) * (6 * 64 + 8) + (!side) * (-8)] : keys[captured_type * 64 + to_square]);
	current_hash ^= keys[offset + to_square];

	const int rook_source = (is_castle) * ((piece_type == k) * ((to_square == g8) * h8 + (to_square == c8) * a8) + (piece_type == K) * ((to_square == g1) * h1 + (to_square == c1) * a1));
	const int rook_target = (is_castle) * (from_square + ((to_square == g8) | (to_square == g1)) - ((to_square == c8) | (to_square == c1)));
	current_hash ^= (is_castle) ? keys[offset - 128 + rook_source] : 0;
	current_hash ^= (is_castle) ? keys[offset - 128 + rook_target] : 0;

	//const auto size = static_cast<std::vector<int, std::allocator<int>>::size_type>(enpassant_history.size() - 2);
	const int different_rights = castling_rights ^ castling_rights_history.back();
	current_hash ^= (get_bit(different_rights, 0)) * keys[12 * 64];
	current_hash ^= (get_bit(different_rights, 1)) * keys[12 * 64 + 1];
	current_hash ^= (get_bit(different_rights, 2)) * keys[12 * 64 + 2];
	current_hash ^= (get_bit(different_rights, 3)) * keys[12 * 64 + 3];
	current_hash ^= keys[772];

	const int old_enpassant_square = enpassant_history.back();
	current_hash ^= ((old_enpassant_square != a8) && (old_enpassant_square != 64)) * keys[773 + (old_enpassant_square % 8)];
	//undo old enpassant key
	current_hash ^= (is_double_push)*keys[773 + (from_square % 8)];

	current_hash = current_hash % 4294967296;
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
	pop_bit(castling_rights, (bK || (bR && (from_square == a8)) || (to_square == a8)) ? 3 : 4);
	pop_bit(castling_rights, (bK || (bR && (from_square == h8)) || (to_square == h8)) ? 2 : 4);
	const bool wK = (piece_type == K);
	const bool wR = (piece_type == R);
	pop_bit(castling_rights, (wK || (wR && (from_square == a1)) || (to_square == a1)) ? 1 : 4);
	pop_bit(castling_rights, (wK || (wR && (from_square == h1)) || (to_square == h1)) ? 0 : 4);
	//branchlessly pop castle right bits

	const int square_offset = 56 * (side);
	const bool is_kingside = to_square > from_square;
	const int rook_source = (is_kingside) ? (h1 - square_offset) : (a1 - square_offset);
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

	update_hash(move);
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
	bitboards[(9 - 6 * (side))] |= (((is_castle) * 1ULL) << ((is_kingside) ? (h8 + square_offset) : (a8 + square_offset)));
	bitboards[(9 - 6 * (side))] &= ~(((is_castle) * 1ULL) << ((is_kingside) ? (f8 + square_offset) : (d8 + square_offset)));
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