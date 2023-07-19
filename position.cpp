#include "position.hpp"
invalid_move_exception::invalid_move_exception(const Position t_pos, const unsigned int t_move) {
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
inline bool Position::is_attacked_by_side(const int sq, const bool color) {
	return (color) ? ((get_rook_attacks(occupancies[both], sq) & (bitboards[9] | bitboards[10])) | (get_bishop_attacks(occupancies[both], sq) & (bitboards[8] | bitboards[10])) | (pawn_attacks[!color][sq] & bitboards[6]) | (king_attacks[sq] & bitboards[11]) | (knight_attacks[sq] & bitboards[7])) :
		((get_rook_attacks(occupancies[both], sq) & (bitboards[3] | bitboards[4])) | (get_bishop_attacks(occupancies[both], sq) & (bitboards[2] | bitboards[4])) | (pawn_attacks[!color][sq] & bitboards[0]) | (king_attacks[sq] & bitboards[5]) | (knight_attacks[sq] & bitboards[1]));
}
inline U64 Position::get_attacks_by(const bool color) {

	int offset = (color) * 6;
	U64 attacks = ((bool)(bitboards[K + offset])) * king_attacks[bitscan(bitboards[K + offset])];
	int type = N + offset;
	U64 tempKnights = bitboards[type];

	while (tempKnights) {
		U64 isolated = _blsi_u64(tempKnights);
		attacks |= knight_attacks[bitscan(isolated)];
		tempKnights = _blsr_u64(tempKnights);
	}
	//knight attacks individually
	attacks |= (color) ? (((bitboards[p] << 7) & notHFile) | ((bitboards[p] << 9) & notAFile)) : (((bitboards[P] >> 7) & notAFile) | ((bitboards[P] >> 9) & notHFile));
	//setwise pawn attacks

	U64 rooks_w_queens = bitboards[R + offset] | bitboards[Q + offset];
	while (rooks_w_queens) {
		U64 isolated = _blsi_u64(rooks_w_queens);
		attacks |= get_rook_attacks(occupancies[both], bitscan(isolated));
		rooks_w_queens = _blsr_u64(rooks_w_queens);
	}
	U64 bishops_w_queens = bitboards[B + offset] | bitboards[Q + offset];
	while (bishops_w_queens) {
		U64 isolated = _blsi_u64(bishops_w_queens);
		attacks |= get_bishop_attacks(occupancies[both], bitscan(isolated));
		bishops_w_queens = _blsr_u64(bishops_w_queens);
	}

	return attacks;
}
inline int Position::get_piece_type_on(const int sq)const {
	return square_board[sq];
}
void Position::try_out_move(std::array<unsigned int,128>& ret, unsigned int move, int& ind) {
	make_move(move);
	if (!is_attacked_by_side(bitscan(bitboards[11 - (side) * 6]), side)) ret[ind++]=move;
	//append move if the king is not attacked after playing the move
	unmake_move();
}

int Position::get_legal_moves(std::array<unsigned int,128>& ret) {
	const int kingpos = bitscan(bitboards[K + (side) * 6]);
	const bool in_check = is_attacked_by_side(kingpos, !side);
	const U64 kings_queen_scope = get_queen_attacks(occupancies[both], kingpos);
	const U64 enemy_attacks = get_attacks_by(!side);
	int ind = 0;
	(in_check) ? (legal_in_check_move_generator(ret, kingpos, kings_queen_scope, enemy_attacks,ind)) : (legal_move_generator(ret, kingpos, kings_queen_scope, enemy_attacks,ind));
	return ind;
}
void Position::legal_move_generator(std::array<unsigned int,128>& ret, const int kingpos, const U64 kings_queen_scope, const U64 enemy_attacks, int& ind) {
	get_castles(ret, enemy_attacks, ind);
	const U64 pinned = get_moves_for_pinned_pieces(ret, kingpos, enemy_attacks, ind);
	const U64 not_pinned = ~pinned;
	const U64 enemy_pieces = occupancies[(!side)];
	const U64 valid_targets = (~occupancies[both]) | enemy_pieces;
	unsigned int move;
	int type = (side) ? n : N;
	__m256i _pieces = _mm256_loadu_epi64((__m256i*) & bitboards[type]);
	__m256i _not_pinned = _mm256_set1_epi64x(not_pinned);
	_pieces = _mm256_and_si256(_pieces, _not_pinned);
	U64 tempKnights = _mm256_extract_epi64(_pieces, 0);
	U64 tempBishops = _mm256_extract_epi64(_pieces, 1);
	U64 tempRooks = _mm256_extract_epi64(_pieces, 2);
	U64 tempQueens = _mm256_extract_epi64(_pieces, 3);
	while (tempKnights) {
		const U64 isolated = _blsi_u64(tempKnights);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		U64 attacks = knight_attacks[sq] & valid_targets;

		while (attacks) {
			const U64 isolated2 = _blsi_u64(attacks);

			unsigned long to = 0UL;
			_BitScanForward64(&to, isolated2);
			//move = 15728640;
			//move |= sq;
			//move |= to << 6;
			//move |= type << 12;
			//move |= get_piece_type_on(to) << 16;
			//move |= (int)_bittest64((long long*)&enemy_pieces, to) << 24;
			move = encode_move(sq, to, type, get_piece_type_on(to), no_piece, (int)_bittest64((long long*)&enemy_pieces,to), false, false, false);
			ret[ind++] = move;

			attacks = _blsr_u64(attacks);
		}
		tempKnights = _blsr_u64(tempKnights);
	}

	type++;

	while (tempBishops) {
		const U64 isolated = _blsi_u64(tempBishops);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		U64 attacks = get_bishop_attacks(occupancies[both], sq) & valid_targets;

		while (attacks) {
			const U64 isolated2 = _blsi_u64(attacks);

			unsigned long to = 0UL;
			_BitScanForward64(&to, isolated2);
			//move = 15728640;
			//move |= sq;
			//move |= to << 6;
			//move |= type << 12;
			//move |= get_piece_type_on(to) << 16;
			//move |= (int)_bittest64((long long*)&enemy_pieces, to) << 24;
			move = encode_move(sq, to, type, get_piece_type_on(to), no_piece, (int)_bittest64((long long*)&enemy_pieces,to), false, false, false);
			ret[ind++] = move;

			attacks = _blsr_u64(attacks);
		}
		tempBishops = _blsr_u64(tempBishops);
	}

	type++;

	while (tempRooks) {
		const U64 isolated = _blsi_u64(tempRooks);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		U64 attacks = get_rook_attacks(occupancies[both], sq) & valid_targets;

		while (attacks) {
			const U64 isolated2 = _blsi_u64(attacks);

			unsigned long to = 0UL;
			_BitScanForward64(&to, isolated2);
			//move = 15728640;
			//move |= sq;
			//move |= to << 6;
			//move |= type << 12;
			//move |= get_piece_type_on(to) << 16;
			//move |= (int)_bittest64((long long*)&enemy_pieces, to) << 24;
			move = encode_move(sq, to, type, get_piece_type_on(to), no_piece, (int)_bittest64((long long*)&enemy_pieces,to), false, false, false);
			ret[ind++] = move;

			attacks = _blsr_u64(attacks);
		}
		tempRooks = _blsr_u64(tempRooks);
	}

	type++;

	while (tempQueens) {
		const U64 isolated = _blsi_u64(tempQueens);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		U64 attacks = get_queen_attacks(occupancies[both], sq) & valid_targets;

		while (attacks) {
			const U64 isolated2 = _blsi_u64(attacks);

			unsigned long to = 0UL;
			_BitScanForward64(&to, isolated2);
			//move = 15728640;
			//move |= sq;
			//move |= to << 6;
			//move |= type << 12;
			//move |= get_piece_type_on(to) << 16;
			//move |= (int)_bittest64((long long*)&enemy_pieces, to) << 24;
			move = encode_move(sq, to, type, get_piece_type_on(to), no_piece, (int)_bittest64((long long*)&enemy_pieces,to), false, false, false);
			ret[ind++] = move;

			attacks = _blsr_u64(attacks);
		}
		tempQueens = _blsr_u64(tempQueens);
	}


	type++;
	U64 attacks = king_attacks[kingpos] & (~(occupancies[(side)] | enemy_attacks));
	while (attacks) {

		const U64 isolated = _blsi_u64(attacks);

		unsigned long to = 0UL;
		_BitScanForward64(&to, isolated);
		//move = 15728640;
		//move |= kingpos;
		//move |= to << 6;
		//move |= type << 12;
		//move |= get_piece_type_on(to) << 16;
		//move |= (int)_bittest64((long long*)&enemy_pieces, to) << 24;
		move = encode_move(kingpos, to, type, get_piece_type_on(to), no_piece, (int)_bittest64((long long*)&enemy_pieces,to), false, false, false);
		ret[ind++] = move;

		attacks = _blsr_u64(attacks);
	}
	get_legal_pawn_moves(ret, kings_queen_scope, enemy_attacks, pinned,ind);
}
void Position::legal_in_check_move_generator(std::array<unsigned int, 128>& ret, const int kingpos, const U64 kings_queen_scope, const U64 enemy_attacks, int& ind){
	const U64 pinned = get_pinned_pieces(kingpos, enemy_attacks);
	const U64 enemy_pieces = occupancies[(!side)];
	const U64 checkers = get_checkers(kingpos);
	const bool not_double_check = count_bits(checkers) < 2;
	const U64 valid_targets = (not_double_check) * (get_checking_rays(kingpos) | checkers);
	const U64 valid_pieces = (~pinned) * (not_double_check);
	unsigned int move;
	int type = (side) ? n : N;
	__m256i _pieces = _mm256_loadu_epi64((__m256i*) & bitboards[type]);
	__m256i _valid_pieces = _mm256_set1_epi64x(valid_pieces);
	_pieces = _mm256_and_si256(_pieces, _valid_pieces);
	U64 tempKnights = _mm256_extract_epi64(_pieces, 0);
	U64 tempBishops = _mm256_extract_epi64(_pieces, 1);
	U64 tempRooks = _mm256_extract_epi64(_pieces, 2);
	U64 tempQueens = _mm256_extract_epi64(_pieces, 3);

	while (tempKnights) {
		const U64 isolated = _blsi_u64(tempKnights);
		const int sq = bitscan(isolated);
		U64 attacks = knight_attacks[sq] & valid_targets;

		while (attacks) {

			const U64 isolated2 = _blsi_u64(attacks);

			unsigned long to = 0UL;
			_BitScanForward64(&to, isolated2);
			//move = 15728640;
			//move |= sq;
			//move |= to << 6;
			//move |= type << 12;
			//move |= get_piece_type_on(to) << 16;
			//move |= (int)_bittest64((long long*)&enemy_pieces, to) << 24;
			move = encode_move(sq, to, type, get_piece_type_on(to), no_piece, (int)_bittest64((long long*)&enemy_pieces,to), false, false, false);
			ret[ind++]=move;

			attacks = _blsr_u64(attacks);
		}
		tempKnights = _blsr_u64(tempKnights);
	}

	type++;

	while (tempBishops) {
		const U64 isolated = _blsi_u64(tempBishops);
		const int sq = bitscan(isolated);
		U64 attacks = get_bishop_attacks(occupancies[both], sq) & valid_targets;

		while (attacks) {

			const U64 isolated2 = _blsi_u64(attacks);

			unsigned long to = 0UL;
			_BitScanForward64(&to, isolated2);
			//move = 15728640;
			//move |= sq;
			//move |= to << 6;
			//move |= type << 12;
			//move |= get_piece_type_on(to) << 16;
			//move |= (int)_bittest64((long long*)&enemy_pieces, to) << 24;
			move = encode_move(sq, to, type, get_piece_type_on(to), no_piece, (int)_bittest64((long long*)&enemy_pieces,to), false, false, false);
			ret[ind++]=move;

			attacks = _blsr_u64(attacks);
		}
		tempBishops = _blsr_u64(tempBishops);
	}

	type++;

	while (tempRooks) {
		const U64 isolated = _blsi_u64(tempRooks);
		const int sq = bitscan(isolated);
		U64 attacks = get_rook_attacks(occupancies[both], sq) & valid_targets;

		while (attacks) {

			const U64 isolated2 = _blsi_u64(attacks);

			unsigned long to = 0UL;
			_BitScanForward64(&to, isolated2);
			//move = 15728640;
			//move |= sq;
			//move |= to << 6;
			//move |= type << 12;
			//move |= get_piece_type_on(to) << 16;
			//move |= (int)_bittest64((long long*)&enemy_pieces, to) << 24;
			move = encode_move(sq, to, type, get_piece_type_on(to), no_piece, (int)_bittest64((long long*)&enemy_pieces,to), false, false, false);
			ret[ind++]=move;

			attacks = _blsr_u64(attacks);
		}
		tempRooks = _blsr_u64(tempRooks);
	}

	type++;

	while (tempQueens) {
		const U64 isolated = _blsi_u64(tempQueens);
		const int sq = bitscan(isolated);
		U64 attacks = get_queen_attacks(occupancies[both], sq) & valid_targets;

		while (attacks) {

			const U64 isolated2 = _blsi_u64(attacks);

			unsigned long to = 0UL;
			_BitScanForward64(&to, isolated2);
			//move = 15728640;
			//move |= sq;
			//move |= to << 6;
			//move |= type << 12;
			//move |= get_piece_type_on(to) << 16;
			//move |= (int)_bittest64((long long*)&enemy_pieces, to) << 24;
			move = encode_move(sq, to, type, get_piece_type_on(to), no_piece, (int)_bittest64((long long*)&enemy_pieces,to), false, false, false);
			ret[ind++]=move;

			attacks = _blsr_u64(attacks);
		}
		tempQueens = _blsr_u64(tempQueens);
	}

	type++;
	pop_bit(occupancies[both], kingpos);
	const U64 enemy_attacks_without_king = get_attacks_by(!side);
	set_bit(occupancies[both], kingpos);
	U64 attacks = king_attacks[kingpos] & (~(enemy_attacks_without_king | occupancies[(side)]));
	while (attacks) {

		const U64 isolated = _blsi_u64(attacks);

		unsigned long to = 0UL;
		_BitScanForward64(&to, isolated);
		//move = 15728640;
		//move |= kingpos;
		//move |= to << 6;
		//move |= type << 12;
		//move |= get_piece_type_on(to) << 16;
		//move |= (int)_bittest64((long long*)&enemy_pieces, to) << 24;
		move = encode_move(kingpos, to, type, get_piece_type_on(to), no_piece, (int)_bittest64((long long*)&enemy_pieces,to), false, false, false);
		ret[ind++]=move;

		attacks = _blsr_u64(attacks);
	}
	in_check_get_legal_pawn_moves(ret, kings_queen_scope | knight_attacks[kingpos], enemy_attacks, pinned, (not_double_check)*checkers, (not_double_check)*valid_targets, ind);
}

int Position::get_legal_captures(std::array<unsigned int,128>& ret) {
	const int kingpos = bitscan(bitboards[K + (side) * 6]);
	const bool in_check = is_attacked_by_side(kingpos, !side);
	const U64 kings_queen_scope = get_queen_attacks(occupancies[both], kingpos);
	const U64 enemy_attacks = get_attacks_by(!side);
	int ind = 0;
	(in_check) ? (legal_in_check_capture_gen(ret, kings_queen_scope, enemy_attacks, ind)) : (legal_capture_gen(ret, kings_queen_scope, enemy_attacks, ind));
	return ind;
}
void Position::legal_capture_gen(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, int& ind) {
	const int kingpos = bitscan(bitboards[K + (side) * 6]);
	const U64 pinned = get_captures_for_pinned_pieces(ret, kingpos, enemy_attacks, ind);
	const U64 enemy_pieces = occupancies[(!side)];
	const U64 not_pinned = ~pinned;
	unsigned int move;
	int type = (side) ? n : N;
	U64 tempKnights = bitboards[type] & not_pinned;

	while (tempKnights) {
		const U64 isolated = _blsi_u64(tempKnights);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		U64 attacks = knight_attacks[sq] & enemy_pieces;

		while (attacks) {

			const U64 isolated2 = _blsi_u64(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
			ret[ind++] = move;

			attacks = _blsr_u64(attacks);
		}
		tempKnights = _blsr_u64(tempKnights);
	}
	type++;
	U64 tempBishops = bitboards[type] & not_pinned;

	while (tempBishops) {
		const U64 isolated = _blsi_u64(tempBishops);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		U64 attacks = get_bishop_attacks(occupancies[both], sq) & enemy_pieces;
		;
		while (attacks) {

			const U64 isolated2 = _blsi_u64(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
			ret[ind++] = move;

			attacks = _blsr_u64(attacks);
		}
		tempBishops = _blsr_u64(tempBishops);
	}

	type++;
	U64 tempRooks = bitboards[type] & not_pinned;
	while (tempRooks) {
		const U64 isolated = _blsi_u64(tempRooks);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		U64 attacks = get_rook_attacks(occupancies[both], sq) & enemy_pieces;

		while (attacks) {

			const U64 isolated2 = _blsi_u64(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
			ret[ind++] = move;

			attacks = _blsr_u64(attacks);
		}
		tempRooks = _blsr_u64(tempRooks);
	}

	type++;
	U64 tempQueens = bitboards[type] & not_pinned;

	while (tempQueens) {
		const U64 isolated = _blsi_u64(tempQueens);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		U64 attacks = get_queen_attacks(occupancies[both], sq) & enemy_pieces;

		while (attacks) {

			const U64 isolated2 = _blsi_u64(attacks);

			const int to = bitscan(isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
			ret[ind++] = move;

			attacks = _blsr_u64(attacks);
		}
		tempQueens = _blsr_u64(tempQueens);
	}

	type++;
	U64 tempKing = bitboards[type];

	U64 attacks = king_attacks[kingpos] & (enemy_pieces & (~enemy_attacks));

	while (attacks) {

		const U64 isolated2 = _blsi_u64(attacks);

		const int to = bitscan(isolated2);
		move = encode_move(kingpos, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
		ret[ind++] = move;

		attacks = _blsr_u64(attacks);
	}
	get_pawn_captures(ret, kings_queen_scope, enemy_attacks, pinned, ind);
}
void Position::legal_in_check_capture_gen(std::array<unsigned int, 128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, int& ind) {
	const int kingpos = bitscan(bitboards[K + (side) * 6]);
	const U64 pinned = get_pinned_pieces(kingpos, enemy_attacks);
	const U64 checkers = get_checkers(kingpos);
	const bool not_double_check = count_bits(checkers) < 2;
	const U64 valid_pieces = (~pinned) * (not_double_check);
	unsigned int move;

	int type = (side) ? n : N;
	U64 tempKnights = bitboards[type] & valid_pieces;

	while (tempKnights) {
		U64 isolated = _blsi_u64(tempKnights);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		U64 attacks = knight_attacks[sq] & checkers;

		while (attacks) {

			const U64 isolated2 = _blsi_u64(attacks);

			unsigned long to = 0UL;
			_BitScanForward64(&to, isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
			try_out_move(ret, move, ind);

			attacks = _blsr_u64(attacks);
		}
		tempKnights = _blsr_u64(tempKnights);
	}

	type++;
	U64 tempBishops = bitboards[type] & valid_pieces;

	while (tempBishops) {
		U64 isolated = _blsi_u64(tempBishops);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		U64 attacks = get_bishop_attacks(occupancies[both], sq) & checkers;
		
		while (attacks) {

			const U64 isolated2 = _blsi_u64(attacks);

			unsigned long to = 0UL;
			_BitScanForward64(&to, isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
			try_out_move(ret, move, ind);

			attacks = _blsr_u64(attacks);
		}
		tempBishops = _blsr_u64(tempBishops);
	}

	type++;
	U64 tempRooks = bitboards[type] & valid_pieces;
	while (tempRooks) {
		U64 isolated = _blsi_u64(tempRooks);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		U64 attacks = get_rook_attacks(occupancies[both], sq) & checkers;

		while (attacks) {

			const U64 isolated2 = _blsi_u64(attacks);

			unsigned long to = 0UL;
			_BitScanForward64(&to, isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), no_piece, true, false, false, false);

			try_out_move(ret, move, ind);

			attacks = _blsr_u64(attacks);
		}
		tempRooks = _blsr_u64(tempRooks);
	}

	type++;
	U64 tempQueens = bitboards[type] & valid_pieces;

	while (tempQueens) {
		U64 isolated = _blsi_u64(tempQueens);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		U64 attacks = get_queen_attacks(occupancies[both], sq) & checkers;

		while (attacks) {

			const U64 isolated2 = _blsi_u64(attacks);

			unsigned long to = 0UL;
			_BitScanForward64(&to, isolated2);
			move = encode_move(sq, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
			try_out_move(ret, move, ind);

			attacks = _blsr_u64(attacks);
		}
		tempQueens = _blsr_u64(tempQueens);
	}

	type++;
	U64 tempKing = bitboards[type];

	pop_bit(occupancies[both], kingpos);
	const U64 enemy_attacks_without_king = get_attacks_by(!side);
	set_bit(occupancies[both], kingpos);
	U64 attacks = (king_attacks[kingpos] & occupancies[(!side)]) & (~(enemy_attacks_without_king | occupancies[(side)]));
	while (attacks) {

		U64 isolated = _blsi_u64(attacks);

		unsigned long to = 0UL;
		_BitScanForward64(&to, isolated);
		move = encode_move(kingpos, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
		ret[ind++]=move;

		attacks = _blsr_u64(attacks);
	}
	in_check_get_pawn_captures(ret, kings_queen_scope, enemy_attacks,pinned, (not_double_check)*checkers, ind);
}

void Position::get_pawn_captures(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, int& ind) {
	(side) ? (legal_bpawn_captures(ret, kings_queen_scope, enemy_attacks, pinned, ind)) : (legal_wpawn_captures(ret, kings_queen_scope, enemy_attacks, pinned, ind));
}
void Position::in_check_get_pawn_captures(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, int& ind) {
	(side) ? (in_check_legal_bpawn_captures(ret, kings_queen_scope, enemy_attacks, pinned, targets, ind)) : (in_check_legal_wpawn_captures(ret, kings_queen_scope, enemy_attacks, pinned, targets, ind));
}

inline void Position::get_legal_pawn_moves(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, int& ind) {
	(side) ? (legal_bpawn_pushes(ret, kings_queen_scope, enemy_attacks, pinned, ind)) : (legal_wpawn_pushes(ret, kings_queen_scope, enemy_attacks, pinned, ind));
	(side) ? (legal_bpawn_captures(ret, kings_queen_scope, enemy_attacks, pinned, ind)) : (legal_wpawn_captures(ret, kings_queen_scope, enemy_attacks, pinned, ind));
}
inline void Position::legal_bpawn_pushes(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, int& ind) {
	U64 valid_targets = ~occupancies[both];
	U64 promoters = bitboards[6] & (~pinned) & rank2;
	U64 push_promotions = (promoters << 8) & valid_targets;
	U64 pushes = ((bitboards[6] & ~(pinned | promoters)) << 8) & valid_targets;
	U64 doublePushes = ((pushes << 8) & rank5) & valid_targets;
	unsigned int move;
	while (push_promotions) {
		U64 isolated = _blsi_u64(push_promotions);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		move = encode_move(sq - 8, sq, p, no_piece, n, false, false, false, true);
		ret[ind++]=move;
		set_promotion_type(move, b);
		ret[ind++]=move;
		set_promotion_type(move, r);
		ret[ind++]=move;
		set_promotion_type(move, q);
		ret[ind++]=move;
		push_promotions = _blsr_u64(push_promotions);
	}
	while (pushes) {
		U64 isolated = _blsi_u64(pushes);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		move = 0xff6000;
		move |= sq - 8;
		move |= sq << 6;
		ret[ind++]=move;
		pushes = _blsr_u64(pushes);
	}
	while (doublePushes) {
		U64 isolated = _blsi_u64(doublePushes);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		move = 0x2ff6000;
		move |= sq - 16;
		move |= sq << 6;
		ret[ind++]=move;
		doublePushes = _blsr_u64(doublePushes);
	}
}
inline void Position::legal_wpawn_pushes(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, int& ind) {
	U64 valid_targets = ~occupancies[both];
	U64 promoters = bitboards[0] & (~pinned) & rank7;
	U64 push_promotions = (promoters >> 8) & valid_targets;
	U64 pushes = ((bitboards[0] & ~(pinned | promoters)) >> 8) & valid_targets;
	U64 doublePushes = ((pushes >> 8) & rank4) & valid_targets;
	unsigned int move;
	while (push_promotions) {
		U64 isolated = _blsi_u64(push_promotions);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		move = encode_move(sq + 8, sq, P, no_piece, N, false, false, false, true);
		ret[ind++]=move;
		set_promotion_type(move, B);
		ret[ind++]=move;
		set_promotion_type(move, R);
		ret[ind++]=move;
		set_promotion_type(move, Q);
		ret[ind++]=move;
		push_promotions = _blsr_u64(push_promotions);
	}
	while (pushes) {
		U64 isolated = _blsi_u64(pushes);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);

		//move = encode_move(sq + 8, sq, P, 15, 15, false, false, false, false);
		move = 0xff0000;
		move |= sq + 8;
		move |= sq << 6;
		ret[ind++]=move;
		pushes = _blsr_u64(pushes);
	}
	while (doublePushes) {
		U64 isolated = _blsi_u64(doublePushes);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		//move = encode_move(sq + 16, sq, P, 15, 15, false, true, false, false);
		move = 0x2ff0000;
		move |= sq + 16;
		move |= sq << 6;
		ret[ind++]=move;
		doublePushes = _blsr_u64(doublePushes);
	}
}
inline void Position::legal_bpawn_captures(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, int& ind) {
	const U64 targets = occupancies[white];
	U64 promoters = bitboards[6] & (~pinned) & rank2;
	U64 captures = ((bitboards[6] & ~(pinned | promoters)) << 7) & notHFile & targets;

	U64 enpassant = 0ULL;
	set_bit(enpassant, enpassant_square);
	const bool left_enpassant = (enpassant >> 7) & notAFile & bitboards[p];
	const bool right_enpassant = (enpassant >> 9) & notHFile & bitboards[p];
	if (left_enpassant) {
		unsigned int move = encode_move(enpassant_square - 7, enpassant_square, p, P, no_piece, true, false, false, true);
		try_out_move(ret, move, ind);
	}
	if (right_enpassant) {
		unsigned int move = encode_move(enpassant_square - 9, enpassant_square, p, P, no_piece, true, false, false, true);
		try_out_move(ret, move, ind);
	}

	while (captures) {
		U64 isolated = _blsi_u64(captures);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		unsigned int move = encode_move(sq - 7, sq, p, get_piece_type_on(sq), no_piece, true, false, false, false);
		//unsigned int move = 0x1f06000;
		//move |= sq - 7;
		//move |= sq << 6;
		//move |= get_piece_type_on(sq)<<16;
		ret[ind++]=move;
		captures = _blsr_u64(captures);
	}
	captures = ((bitboards[6] & ~(pinned | promoters)) << 9) & notAFile & targets;
	while (captures) {
		U64 isolated = _blsi_u64(captures);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		unsigned int move = encode_move(sq - 9, sq, p, get_piece_type_on(sq), no_piece, true, false, false, false);
		//unsigned int move = 0x1f06000;
		//move |= sq - 9;
		//move |= sq << 6;
		//move |= get_piece_type_on(sq) << 16;
		ret[ind++]=move;
		captures = _blsr_u64(captures);
	}

	U64 promotion_captures = ((promoters) << 7) & notHFile & targets;
	while (promotion_captures) {
		U64 isolated = _blsi_u64(promotion_captures);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		unsigned int move = encode_move(sq - 7, sq, p, get_piece_type_on(sq), n, true, false, false, false);
		ret[ind++]=move;
		set_promotion_type(move, b);
		ret[ind++]=move;
		set_promotion_type(move, r);
		ret[ind++]=move;
		set_promotion_type(move, q);
		ret[ind++]=move;
		promotion_captures = _blsr_u64(promotion_captures);
	}
	promotion_captures = ((promoters) << 9) & notAFile & targets;
	while (promotion_captures) {
		U64 isolated = _blsi_u64(promotion_captures);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		unsigned int move = encode_move(sq - 9, sq, p, get_piece_type_on(sq), n, true, false, false, false);
		ret[ind++]=move;
		set_promotion_type(move, b);
		ret[ind++]=move;
		set_promotion_type(move, r);
		ret[ind++]=move;
		set_promotion_type(move, q);
		ret[ind++]=move;
		promotion_captures = _blsr_u64(promotion_captures);
	}
}
inline void Position::legal_wpawn_captures(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, int& ind) {
	U64 promoters = bitboards[0] & (~pinned) & rank7;
	const U64 targets = occupancies[black];
	U64 captures = ((bitboards[0] & ~(pinned | promoters)) >> 7) & notAFile & targets;


	U64 enpassant = 0ULL;
	enpassant = (enpassant_square != a8) * (1ULL << enpassant_square);
	const bool left_enpassant = (enpassant << 7) & notHFile & bitboards[P];
	const bool right_enpassant = (enpassant << 9) & notAFile & bitboards[P];
	if (left_enpassant) {
		unsigned int move = encode_move(enpassant_square + 7, enpassant_square, P, p, no_piece, true, false, false, true);
		try_out_move(ret, move, ind);
	}
	if (right_enpassant) {
		unsigned int move = encode_move(enpassant_square + 9, enpassant_square, P, p, no_piece, true, false, false, true);
		try_out_move(ret, move, ind);
	}

	while (captures) {
		U64 isolated = _blsi_u64(captures);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		unsigned int move = encode_move(sq + 7, sq, P, get_piece_type_on(sq), no_piece, true, false, false, false);
		//unsigned int move = 0x1f00000;
		//move |= sq + 7;
		//move |= sq << 6;
		//move |= get_piece_type_on(sq) << 16;
		ret[ind++]=move;
		captures = _blsr_u64(captures);
	}
	captures = ((bitboards[0] & ~(pinned | promoters)) >> 9) & notHFile & targets;
	while (captures) {
		U64 isolated = _blsi_u64(captures);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		unsigned int move = encode_move(sq + 9, sq, P, get_piece_type_on(sq), no_piece, true, false, false, false);
		//unsigned int move = 0x1f00000;
		//move |= sq + 9;
		//move |= sq << 6;
		//move |= get_piece_type_on(sq) << 16;
		ret[ind++]=move;
		captures = _blsr_u64(captures);
	}
	U64 promotion_captures = ((promoters) >> 7) & notAFile & targets;
	while (promotion_captures) {
		U64 isolated = _blsi_u64(promotion_captures);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		unsigned int move = encode_move(sq + 7, sq, P, get_piece_type_on(sq), N, true, false, false, false);
		ret[ind++]=move;
		set_promotion_type(move, B);
		ret[ind++]=move;
		set_promotion_type(move, R);
		ret[ind++]=move;
		set_promotion_type(move, Q);
		ret[ind++]=move;
		promotion_captures = _blsr_u64(promotion_captures);
	}
	promotion_captures = ((promoters) >> 9) & notHFile & targets;
	while (promotion_captures) {
		U64 isolated = _blsi_u64(promotion_captures);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		unsigned int move = encode_move(sq + 9, sq, P, get_piece_type_on(sq), N, true, false, false, false);
		ret[ind++]=move;
		set_promotion_type(move, B);
		ret[ind++]=move;
		set_promotion_type(move, R);
		ret[ind++]=move;
		set_promotion_type(move, Q);
		ret[ind++]=move;
		promotion_captures = _blsr_u64(promotion_captures);
	}
}

inline void Position::in_check_get_legal_pawn_moves(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, const U64 in_check_valid, int& ind) {
	(side) ? (in_check_legal_bpawn_pushes(ret, kings_queen_scope, enemy_attacks, pinned, targets, in_check_valid, ind)) : (in_check_legal_wpawn_pushes(ret, kings_queen_scope, enemy_attacks, pinned, targets, in_check_valid, ind));
	(side) ? (in_check_legal_bpawn_captures(ret, kings_queen_scope, enemy_attacks, pinned, targets, ind)) : (in_check_legal_wpawn_captures(ret, kings_queen_scope, enemy_attacks, pinned, targets, ind));
}
inline void Position::in_check_legal_bpawn_pushes(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, const U64 in_check_valid, int& ind) {
	const U64 valid_targets = ~occupancies[both];
	U64 promoters = bitboards[6] & (~pinned) & rank2;
	U64 push_promotions = (promoters << 8) & in_check_valid & valid_targets;
	U64 pushes = ((bitboards[6] & ~(pinned | promoters)) << 8) & valid_targets;
	U64 doublePushes = ((pushes << 8) & rank5) & in_check_valid & valid_targets;
	pushes = pushes & in_check_valid & valid_targets;
	unsigned int move;
	while (push_promotions) {
		U64 isolated = _blsi_u64(push_promotions);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		move = encode_move(sq - 8, sq, p, no_piece, n, false, false, false, false);
		ret[ind++]=move;
		set_promotion_type(move, b);
		ret[ind++]=move;
		set_promotion_type(move, r);
		ret[ind++]=move;
		set_promotion_type(move, q);
		ret[ind++]=move;
		push_promotions = _blsr_u64(push_promotions);
	}
	while (pushes) {
		U64 isolated = _blsi_u64(pushes);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		move = encode_move(sq - 8, sq, p, no_piece, no_piece, false, false, false, false);
		ret[ind++]=move;
		pushes = _blsr_u64(pushes);
	}
	while (doublePushes) {
		U64 isolated = _blsi_u64(doublePushes);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		move = encode_move(sq - 16, sq, p, no_piece, no_piece, false, true, false, false);
		ret[ind++]=move;
		doublePushes = _blsr_u64(doublePushes);
	}
}
inline void Position::in_check_legal_wpawn_pushes(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, const U64 in_check_valid, int& ind) {
	U64 valid_targets = ~occupancies[both];
	U64 promoters = bitboards[0] & (~pinned) & rank7;
	U64 push_promotions = (promoters >> 8) & in_check_valid & valid_targets;
	U64 pushes = ((bitboards[0] & ~(pinned | promoters)) >> 8) & valid_targets;
	U64 doublePushes = ((pushes >> 8) & rank4) & in_check_valid & valid_targets;
	pushes = pushes & in_check_valid & valid_targets;
	unsigned int move;
	while (push_promotions) {
		U64 isolated = _blsi_u64(push_promotions);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		move = encode_move(sq + 8, sq, P, no_piece, N, false, false, false, false);
		ret[ind++]=move;
		set_promotion_type(move, B);
		ret[ind++]=move;
		set_promotion_type(move, R);
		ret[ind++]=move;
		set_promotion_type(move, Q);
		ret[ind++]=move;
		push_promotions = _blsr_u64(push_promotions);
	}
	while (pushes) {
		U64 isolated = _blsi_u64(pushes);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		move = encode_move(sq + 8, sq, P, no_piece, no_piece, false, false, false, false);
		ret[ind++]=move;
		pushes = _blsr_u64(pushes);
	}
	while (doublePushes) {
		U64 isolated = _blsi_u64(doublePushes);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		move = encode_move(sq + 16, sq, P, no_piece, no_piece, false, true, false, false);
		ret[ind++]=move;
		doublePushes = _blsr_u64(doublePushes);
	}
}
inline void Position::in_check_legal_bpawn_captures(std::array<unsigned int,128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, int& ind) {
	U64 promoters = bitboards[6] & (~pinned) & rank2;
	U64 captures = ((bitboards[6] & (~(pinned | promoters))) << 7) & notHFile & targets;

	U64 enpassant = 0ULL;
	set_bit(enpassant, enpassant_square);
	const bool left_enpassant = (enpassant >> 7) & notAFile & (bitboards[p]);
	const bool right_enpassant = (enpassant >> 9) & notHFile & (bitboards[p]);
	if (left_enpassant) {
		unsigned int move = encode_move(enpassant_square - 7, enpassant_square, p, P, no_piece, true, false, false, true);
		try_out_move(ret, move, ind);
	}
	if (right_enpassant) {
		unsigned int move = encode_move(enpassant_square - 9, enpassant_square, p, P, no_piece, true, false, false, true);
		try_out_move(ret, move, ind);
	}

	while (captures) {
		U64 isolated = _blsi_u64(captures);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		unsigned int move = encode_move(sq - 7, sq, p, get_piece_type_on(sq), no_piece, true, false, false, false);
		ret[ind++] = move;
		captures = captures & ones_decrement(captures);
	}
	captures = ((bitboards[6] & ~(pinned | promoters)) << 9) & notAFile & targets;
	while (captures) {
		U64 isolated = _blsi_u64(captures);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		unsigned int move = encode_move(sq - 9, sq, p, get_piece_type_on(sq), no_piece, true, false, false, false);
		ret[ind++] = move;
		captures = captures & ones_decrement(captures);
	}

	U64 promotion_captures = ((promoters) << 7) & notHFile & targets;
	while (promotion_captures) {
		U64 isolated = promotion_captures & twos_complement(promotion_captures);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		unsigned int move = encode_move(sq - 7, sq, p, get_piece_type_on(sq), n, true, false, false, false);
		ret[ind++] = move;
		set_promotion_type(move, b);
		ret[ind++] = move;
		set_promotion_type(move, r);
		ret[ind++] = move;
		set_promotion_type(move, q);
		ret[ind++] = move;
		promotion_captures = promotion_captures & ones_decrement(promotion_captures);
	}
	promotion_captures = ((promoters) << 9) & notAFile & targets;
	while (promotion_captures) {
		U64 isolated = promotion_captures & twos_complement(promotion_captures);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		unsigned int move = encode_move(sq - 9, sq, p, get_piece_type_on(sq), n, true, false, false, false);
		ret[ind++] = move;
		set_promotion_type(move, b);
		ret[ind++] = move;
		set_promotion_type(move, r);
		ret[ind++] = move;
		set_promotion_type(move, q);
		ret[ind++] = move;
		promotion_captures = promotion_captures & ones_decrement(promotion_captures);
	}
}
inline void Position::in_check_legal_wpawn_captures(std::array<unsigned int, 128>& ret, const U64 kings_queen_scope, const U64 enemy_attacks, const U64 pinned, const U64 targets, int& ind) {
	U64 promoters = bitboards[0] & (~pinned) & rank7;
	U64 captures = ((bitboards[0] & (~(pinned | promoters))) >> 7) & notAFile & targets;

	U64 enpassant = (enpassant_square != a8) * (1ULL << enpassant_square);
	const bool left_enpassant = (enpassant << 7) & notHFile & bitboards[P];
	const bool right_enpassant = (enpassant << 9) & notAFile & bitboards[P];
	if (left_enpassant) {
		unsigned int move = encode_move(enpassant_square + 7, enpassant_square, P, p, no_piece, true, false, false, true);
		try_out_move(ret, move, ind);
	}
	if (right_enpassant) {
		unsigned int move = encode_move(enpassant_square + 9, enpassant_square, P, p, no_piece, true, false, false, true);
		try_out_move(ret, move, ind);
	}

	while (captures) {
		U64 isolated = _blsi_u64(captures);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		unsigned int move = encode_move(sq + 7, sq, P, get_piece_type_on(sq), no_piece, true, false, false, false);
		ret[ind++]=move;
		captures = captures & ones_decrement(captures);
	}
	captures = ((bitboards[0] & ~(pinned | promoters)) >> 9) & notHFile & targets;
	while (captures) {
		U64 isolated = _blsi_u64(captures);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		unsigned int move = encode_move(sq + 9, sq, P, get_piece_type_on(sq), no_piece, true, false, false, false);
		ret[ind++]=move;
		captures = captures & ones_decrement(captures);
	}
	U64 promotion_captures = ((promoters) >> 7) & notAFile & targets;
	while (promotion_captures) {
		U64 isolated = promotion_captures & twos_complement(promotion_captures);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		unsigned int move = encode_move(sq + 7, sq, P, get_piece_type_on(sq), N, true, false, false, false);
		ret[ind++]=move;
		set_promotion_type(move, B);
		ret[ind++]=move;
		set_promotion_type(move, R);
		ret[ind++]=move;
		set_promotion_type(move, Q);
		ret[ind++]=move;
		promotion_captures = promotion_captures & ones_decrement(promotion_captures);
	}
	promotion_captures = ((promoters) >> 9) & notHFile & targets;
	while (promotion_captures) {
		U64 isolated = promotion_captures & twos_complement(promotion_captures);
		unsigned long sq = 0UL;
		_BitScanForward64(&sq, isolated);
		unsigned int move = encode_move(sq + 9, sq, P, get_piece_type_on(sq), N, true, false, false, false);
		ret[ind++]=move;
		set_promotion_type(move, B);
		ret[ind++]=move;
		set_promotion_type(move, R);
		ret[ind++]=move;
		set_promotion_type(move, Q);
		ret[ind++]=move;
		promotion_captures = promotion_captures & ones_decrement(promotion_captures);
	}
}

inline void Position::get_castles(std::array<unsigned int,128>& ptr, const U64 enemy_attacks, int& ind) {
	const U64 empty = ~(occupancies[both]);

	const int king_index = 5 + (side) * 6;
	const int kingpos = bitscan(bitboards[king_index]);
	const bool king_on_valid_square = (side) ? (kingpos == e8) : (kingpos == e1);

	const int enemy_color = (side) ? white : black;
	
	const bool king_in_valid_state = king_on_valid_square && (!(int)_bittest64((long long*)&enemy_attacks, kingpos));
	bool queenside = king_in_valid_state && (get_bit(castling_rights, 1 + 2 * (side)));
	bool kingside = king_in_valid_state && (get_bit(castling_rights, 2 * (side)));

	const int square_offset = 56 * (side);

	queenside &= (((bitboards[king_index]) >> 1) & empty) && (!(bool)_bittest64((long long*)&enemy_attacks, d1 - square_offset));
	queenside &= (((bitboards[king_index]) >> 2) & empty) && (!(bool)_bittest64((long long*)&enemy_attacks, c1 - square_offset));
	queenside &= (bool)(((bitboards[king_index]) >> 3) & empty);

	unsigned int move = encode_move(kingpos, kingpos - 2, king_index, no_piece, no_piece, false, false, true, false);

	if (queenside) {
		ptr[ind++]=move;
	}

	kingside &= (((bitboards[king_index]) << 1) & empty) && (!(bool)_bittest64((long long*)&enemy_attacks, f1 - square_offset));
	kingside &= (((bitboards[king_index]) << 2) & empty) && (!(bool)_bittest64((long long*)&enemy_attacks, g1 - square_offset));
	set_to_square(move, kingpos + 2);
	if (kingside) {
		ptr[ind++] = move;
	}
}

U64 Position::get_pinned_pieces(const int kingpos, const U64 enemy_attacks) {
	U64 potentially_pinned_by_bishops = enemy_attacks & get_bishop_attacks(occupancies[both], kingpos) & occupancies[(side)];
	U64 potentially_pinned_by_rooks = enemy_attacks & get_rook_attacks(occupancies[both], kingpos) & occupancies[(side)];
	U64 pinned_pieces = 0ULL;
	const int offset = 6 * (!side);
	while (potentially_pinned_by_bishops) {
		const U64 isolated = _blsi_u64(potentially_pinned_by_bishops);
		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_pinner = get_bishop_attacks(occupancies[both], kingpos) & (bitboards[B + offset] | bitboards[Q + offset]);
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		pinned_pieces |= (((bool)(pot_pinner)) & ((bool)(isolated & get_bishop_attacks(occupancies[both], static_cast<unsigned long long>(bitscan(pot_pinner)))))) * isolated;
		potentially_pinned_by_bishops = _blsr_u64(potentially_pinned_by_bishops);
	}
	while (potentially_pinned_by_rooks) {
		const U64 isolated = _blsi_u64(potentially_pinned_by_rooks);
		occupancies[both] &= ~isolated;//pop bit of piece from occupancies
		U64 pot_pinner = get_rook_attacks(occupancies[both], kingpos) & (bitboards[R + offset] | bitboards[Q + offset]);
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		pinned_pieces |= (((bool)(pot_pinner)) & ((bool)(isolated & get_rook_attacks(occupancies[both], static_cast<unsigned long long>(bitscan(pot_pinner)))))) * isolated;

		potentially_pinned_by_rooks = _blsr_u64(potentially_pinned_by_rooks);
	}
	return pinned_pieces;
}
U64 Position::get_moves_for_pinned_pieces(std::array<unsigned int,128>& ret, const int kingpos, const U64 enemy_attacks, int& ind) {
	const int piece_offset = 6 * (side);
	const U64 bishop_attacks = get_bishop_attacks(occupancies[both], kingpos);
	const U64 rook_attacks = get_rook_attacks(occupancies[both], kingpos);
	const U64 pot_pinned_by_bishops = enemy_attacks & bishop_attacks;
	const U64 pot_pinned_by_rooks = enemy_attacks & rook_attacks;
	int type = P + piece_offset;

	__m256i _pieces = _mm256_loadu_epi64((__m256i*) &bitboards[type]);
	__m256i _pot_pinned_by_pieces = _mm256_set1_epi64x(pot_pinned_by_bishops);
	_pot_pinned_by_pieces = _mm256_and_si256(_pieces, _pot_pinned_by_pieces);

	U64 pawns_pot_pinned_by_bishops = _mm256_extract_epi64(_pot_pinned_by_pieces, 0);
	U64 knights_pot_pinned_by_bishops = _mm256_extract_epi64(_pot_pinned_by_pieces, 1);
	U64 bishops_pot_pinned_by_bishops = _mm256_extract_epi64(_pot_pinned_by_pieces, 2);
	U64 rooks_pot_pinned_by_bishops = _mm256_extract_epi64(_pot_pinned_by_pieces, 3);
	U64 queens_pot_pinned_by_bishops = pot_pinned_by_bishops & bitboards[Q + piece_offset];

	_pot_pinned_by_pieces = _mm256_set1_epi64x(pot_pinned_by_rooks);
	_pot_pinned_by_pieces = _mm256_and_si256(_pieces, _pot_pinned_by_pieces);


	U64 pawns_pot_pinned_by_rooks = _mm256_extract_epi64(_pot_pinned_by_pieces, 0);
	U64 knights_pot_pinned_by_rooks = _mm256_extract_epi64(_pot_pinned_by_pieces, 1);
	U64 bishops_pot_pinned_by_rooks = _mm256_extract_epi64(_pot_pinned_by_pieces, 2);
	U64 rooks_pot_pinned_by_rooks = _mm256_extract_epi64(_pot_pinned_by_pieces, 3);
	U64 queens_pot_pinned_by_rooks = pot_pinned_by_rooks & bitboards[Q + piece_offset];

	U64 pinned_pieces = 0ULL;

	const int offset = 6 * (!side);

	const U64 rook_and_queen_mask=bitboards[R + offset] | bitboards[Q + offset];
	const U64 bishop_and_queen_mask= bitboards[B + offset] | bitboards[Q + offset];
	type = P + piece_offset;
	while (pawns_pot_pinned_by_bishops) {
		const U64 isolated = _blsi_u64(pawns_pot_pinned_by_bishops);
		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_bishop_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & bishop_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			unsigned long from = 0UL;
			_BitScanForward64(&from, isolated);
			unsigned long to = 0UL;
			_BitScanForward64(&to, pinner);
			U64 attacks = pawn_attacks[(side)][from];
			if (attacks & pinner) {
				unsigned int move = encode_move(from, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
				if ((side && get_bit(rank1,to)) || ((!side) && get_bit(rank8,to))) {
					set_promotion_type(move, N + piece_offset);
					ret[ind++]=move;
					set_promotion_type(move, B + piece_offset);
					ret[ind++]=move;
					set_promotion_type(move, R + piece_offset);
					ret[ind++]=move;
					set_promotion_type(move, Q + piece_offset);
				}
				ret[ind++]=move;
			}
		}
		occupancies[both] |= isolated;//reset bit of piece from occupancies

		pawns_pot_pinned_by_bishops = _blsr_u64(pawns_pot_pinned_by_bishops);
	}
	const int sign = (side)-(!side);
	while (pawns_pot_pinned_by_rooks) {
		const U64 isolated = _blsi_u64(pawns_pot_pinned_by_rooks);
		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_rook_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & rook_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			unsigned long from = 0;
			_BitScanForward64(&from, isolated);
			unsigned long to = 0;
			_BitScanForward64(&to, isolated);
			U64 valid_targets = (~occupancies[both]) & pot_attacks;
			const int push_target = from + 8 * sign;
			const int double_push_target = from + 16 * sign;
			if (get_bit(valid_targets, push_target)) {
				unsigned int move = encode_move(from, push_target, type, no_piece, no_piece, false, false, false, false);
				if ((side && get_bit(rank1, to)) || ((!side) && get_bit(rank8, to))) {
					set_promotion_type(move, N + piece_offset);
					ret[ind++]=move;
					set_promotion_type(move, B + piece_offset);
					ret[ind++]=move;
					set_promotion_type(move, R + piece_offset);
					ret[ind++]=move;
					set_promotion_type(move, Q + piece_offset);
				}
				ret[ind++]=move;
			}
			if ((get_bit(valid_targets, push_target) && get_bit(valid_targets,double_push_target)) && ((side && get_bit(rank5, double_push_target)) || (!side && get_bit(rank4, double_push_target)))) {
				unsigned int move = encode_move(from, double_push_target, type, no_piece, no_piece, false, true, false, false);
				ret[ind++] = move;
			}
		}
		occupancies[both] |= isolated;//reset bit of piece from occupancies

		pawns_pot_pinned_by_rooks = _blsr_u64(pawns_pot_pinned_by_rooks);
	}

	type++;
	while (knights_pot_pinned_by_bishops) {
		const U64 isolated = _blsi_u64(knights_pot_pinned_by_bishops);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		pinned_pieces |= ((bool)(get_bishop_attacks(occupancies[both], kingpos) & bishop_and_queen_mask)) * isolated;
		occupancies[both] |= isolated;//reset bit of piece from occupancies

		knights_pot_pinned_by_bishops = _blsr_u64(knights_pot_pinned_by_bishops);
	}
	while (knights_pot_pinned_by_rooks) {
		const U64 isolated = _blsi_u64(knights_pot_pinned_by_rooks);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancies
		pinned_pieces |= ((bool)(get_rook_attacks(occupancies[both], kingpos) & rook_and_queen_mask)) * isolated;
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		knights_pot_pinned_by_rooks = _blsr_u64(knights_pot_pinned_by_rooks);
	}

	type++;
	while (bishops_pot_pinned_by_bishops) {
		const U64 isolated = _blsi_u64(bishops_pot_pinned_by_bishops);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_bishop_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & bishop_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			const int from = bitscan(isolated);
			const int to = bitscan(pinner);
			unsigned int move = encode_move(from, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
			ret[ind++]=move;
			U64 attacks = (pot_attacks & get_bishop_attacks(occupancies[both], to)) & (~isolated);
			while (attacks) {
				const U64 isolated2 = _blsi_u64(attacks);
				move = encode_move(from, bitscan(isolated2), type, no_piece, no_piece, false, false, false, false);
				ret[ind++]=move;
				attacks = _blsr_u64(attacks);
			}
		}
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		bishops_pot_pinned_by_bishops = bishops_pot_pinned_by_bishops & ones_decrement(bishops_pot_pinned_by_bishops);
	}
	while (bishops_pot_pinned_by_rooks) {
		const U64 isolated = _blsi_u64(bishops_pot_pinned_by_rooks);
		occupancies[both] &= ~isolated;//pop bit of piece from occupancies
		pinned_pieces |= ((bool)(get_rook_attacks(occupancies[both], kingpos) & rook_and_queen_mask)) * isolated;
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		bishops_pot_pinned_by_rooks = bishops_pot_pinned_by_rooks & ones_decrement(bishops_pot_pinned_by_rooks);
	}
	type++;
	while (rooks_pot_pinned_by_rooks) {
		const U64 isolated = _blsi_u64(rooks_pot_pinned_by_rooks);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_rook_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & rook_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			const int from = bitscan(isolated);
			const int to = bitscan(pinner);
			unsigned int move = encode_move(from, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
			ret[ind++]=move;
			U64 attacks = (pot_attacks & get_rook_attacks(occupancies[both], to)) & (~isolated);
			while (attacks) {
				const U64 isolated2 = _blsi_u64(attacks);
				move = encode_move(from, bitscan(isolated2), type, no_piece, no_piece, false, false, false, false);
				ret[ind++]=move;
				attacks = _blsr_u64(attacks);
			}
		}
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		rooks_pot_pinned_by_rooks = rooks_pot_pinned_by_rooks & ones_decrement(rooks_pot_pinned_by_rooks);
	}
	while (rooks_pot_pinned_by_bishops) {
		const U64 isolated = _blsi_u64(rooks_pot_pinned_by_bishops);
		occupancies[both] &= ~isolated;//pop bit of piece from occupancies
		pinned_pieces |= ((bool)(get_bishop_attacks(occupancies[both], kingpos) & bishop_and_queen_mask)) * isolated;
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		rooks_pot_pinned_by_bishops = rooks_pot_pinned_by_bishops & ones_decrement(rooks_pot_pinned_by_bishops);
	}

	type++;
	while (queens_pot_pinned_by_bishops) {
		const U64 isolated = _blsi_u64(queens_pot_pinned_by_bishops);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_bishop_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & bishop_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			const int from = bitscan(isolated);
			const int to = bitscan(pinner);
			unsigned int move = encode_move(from, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
			ret[ind++]=move;
			U64 attacks = (pot_attacks & get_bishop_attacks(occupancies[both], to)) & (~isolated);
			while (attacks) {
				const U64 isolated2 = _blsi_u64(attacks);
				move = encode_move(from, bitscan(isolated2), type, no_piece, no_piece, false, false, false, false);
				ret[ind++]=move;
				attacks = _blsr_u64(attacks);
			}
		}
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		queens_pot_pinned_by_bishops = queens_pot_pinned_by_bishops & ones_decrement(queens_pot_pinned_by_bishops);
	}
	while (queens_pot_pinned_by_rooks) {
		const U64 isolated = _blsi_u64(queens_pot_pinned_by_rooks);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_rook_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & rook_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			const int from = bitscan(isolated);
			const int to = bitscan(pinner);
			unsigned int move = encode_move(from, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
			ret[ind++]=move;
			U64 attacks = (pot_attacks & get_rook_attacks(occupancies[both], to)) & (~isolated);
			while (attacks) {
				const U64 isolated2 = _blsi_u64(attacks);
				move = encode_move(from, bitscan(isolated2), type, no_piece, no_piece, false, false, false, false);
				ret[ind++]=move;
				attacks = _blsr_u64(attacks);
			}
		}
		occupancies[both] |= isolated;//reset bit of piece from occupancies
		queens_pot_pinned_by_rooks = queens_pot_pinned_by_rooks & ones_decrement(queens_pot_pinned_by_rooks);
	}
	return pinned_pieces;
}
U64 Position::get_captures_for_pinned_pieces(std::array<unsigned int,128>& ret, const int kingpos, const U64 enemy_attacks, int& ind) {
	const int piece_offset = 6 * (side);
	const U64 bishop_attacks = get_bishop_attacks(occupancies[both], kingpos);
	const U64 rook_attacks = get_rook_attacks(occupancies[both], kingpos);
	const U64 pot_pinned_by_bishops = enemy_attacks & bishop_attacks;
	const U64 pot_pinned_by_rooks = enemy_attacks & rook_attacks;
	int type = P + piece_offset;

	__m256i _pieces = _mm256_loadu_epi64((__m256i*) & bitboards[type]);
	__m256i _pot_pinned_by_pieces = _mm256_set1_epi64x(pot_pinned_by_bishops);
	_pot_pinned_by_pieces = _mm256_and_si256(_pieces, _pot_pinned_by_pieces);

	U64 pawns_pot_pinned_by_bishops = _mm256_extract_epi64(_pot_pinned_by_pieces, 0);
	U64 knights_pot_pinned_by_bishops = _mm256_extract_epi64(_pot_pinned_by_pieces, 1);
	U64 bishops_pot_pinned_by_bishops = _mm256_extract_epi64(_pot_pinned_by_pieces, 2);
	U64 rooks_pot_pinned_by_bishops = _mm256_extract_epi64(_pot_pinned_by_pieces, 3);
	U64 queens_pot_pinned_by_bishops = pot_pinned_by_bishops & bitboards[Q + piece_offset];

	_pot_pinned_by_pieces = _mm256_set1_epi64x(pot_pinned_by_rooks);
	_pot_pinned_by_pieces = _mm256_and_si256(_pieces, _pot_pinned_by_pieces);


	U64 pawns_pot_pinned_by_rooks = _mm256_extract_epi64(_pot_pinned_by_pieces, 0);
	U64 knights_pot_pinned_by_rooks = _mm256_extract_epi64(_pot_pinned_by_pieces, 1);
	U64 bishops_pot_pinned_by_rooks = _mm256_extract_epi64(_pot_pinned_by_pieces, 2);
	U64 rooks_pot_pinned_by_rooks = _mm256_extract_epi64(_pot_pinned_by_pieces, 3);
	U64 queens_pot_pinned_by_rooks = pot_pinned_by_rooks & bitboards[Q + piece_offset];

	U64 pinned_pieces = 0ULL;

	const int offset = 6 * (!side);
	const U64 bishop_and_queen_mask = bitboards[B + offset] | bitboards[Q + offset];
	const U64 rook_and_queen_mask = bitboards[R + offset] | bitboards[Q + offset];
	type = B + piece_offset;
	while (bishops_pot_pinned_by_bishops) {
		const U64 isolated = bishops_pot_pinned_by_bishops & twos_complement(bishops_pot_pinned_by_bishops);

		occupancies[both] &= ~isolated;//pop bit of piece from occupancie
		U64 pot_attacks = get_bishop_attacks(occupancies[both], kingpos);
		const U64 pinner = pot_attacks & bishop_and_queen_mask;
		pinned_pieces |= ((bool)(pinner)) * isolated;
		if (pinner) {
			const int from = bitscan(isolated);
			const int to = bitscan(pinner);
			unsigned int move = encode_move(from, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
			ret[ind++]=move;
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
			unsigned int move = encode_move(from, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
			ret[ind++]=move;
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
			unsigned int move = encode_move(from, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
			ret[ind++]=move;
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
			unsigned int move = encode_move(from, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
			ret[ind++]=move;
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
			if (attacks & pinner) {
				unsigned int move = encode_move(from, to, type, get_piece_type_on(to), no_piece, true, false, false, false);
				if ((side && get_bit(rank1, to)) || ((!side) && get_bit(rank8, to))) {
					set_promotion_type(move, N + piece_offset);
					ret[ind++]=move;
					set_promotion_type(move, B + piece_offset);
					ret[ind++]=move;
					set_promotion_type(move, R + piece_offset);
					ret[ind++]=move;
					set_promotion_type(move, Q + piece_offset);
				}
				ret[ind++]=move;
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
	move_history = std::vector<unsigned int>{};
	move_history.reserve(256);
	enpassant_history = std::vector<short>{};
	enpassant_history.reserve(256);
	castling_rights_history = std::vector<short>{};
	castling_rights_history.reserve(256);
	no_pawns_or_captures_history = std::vector<short>{};
	no_pawns_or_captures_history.reserve(256);
	hash_history = std::vector<U64>{};
	hash_history.reserve(256);
	square_board = std::array<short, 64>{};
	for (int i = 0; i < square_board.size(); i++) {
		square_board[i] = no_piece;
	}
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
	square_board = std::array<short, 64>{};
	for (int i = 0; i < square_board.size(); i++) {
		square_board[i] = no_piece;
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
		default:
			set_bit(bitboards[char_pieces(letter)], sq);
			square_board[sq] = char_pieces(letter);
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

std::string Position::fen() const{
	std::string ret = "";
	const int arr[8] = { 7,6,5,4,3,2,1,0 };
	for (int i = 0; i < 8; i++) {
		int empty_spaces = 0;
		for (int j = 0; j < 8; j++) {
			const int ind = 8 * i + j;
			int piece = get_piece_type_on(ind);
			if (piece == no_piece) {
				piece = get_piece_type_on(ind);
			}
			if ((piece == no_piece) || ((ind == enpassant_square) && (enpassant_square != a8))) {
				empty_spaces++;
			}
			else {
				if (empty_spaces) {
					ret += std::to_string(empty_spaces);
				}
				empty_spaces = 0;
				ret += ascii_pieces[piece];
			}
		}
		if (empty_spaces) {
			ret += std::to_string(empty_spaces);
		}
		if (i < 7) {
			ret += "/";
		}
	}
	ret += " ";
	if (side) {
		ret += "b";
	}
	else {
		ret += "w";
	}
	ret += " ";
	if (get_bit(castling_rights, 0)) {
		ret += "K";
	}
	if (get_bit(castling_rights, 1)) {
		ret += "Q";
	}
	if (get_bit(castling_rights, 2)) {
		ret += "k";
	}
	if (get_bit(castling_rights, 3)) {
		ret += "q";
	}
	if (!(get_bit(castling_rights, 0) || get_bit(castling_rights, 1) || get_bit(castling_rights, 2) || get_bit(castling_rights, 3))) {
		ret += "-";
	}
	ret += " ";
	if ((enpassant_square == a8) || (enpassant_square == 64)) {
		ret += "-";
	}
	else {
		ret += square_coordinates[enpassant_square];
	}
	ret += " ";
	ret += std::to_string(no_pawns_or_captures);
	ret += " ";
	if (ply % 2) {
		ret += std::to_string((ply + 1) / 2);
	}
	else {
		ret += std::to_string(ply / 2 + 1);
	}
	return ret;
}
void Position::print() {
	printf("\n");
	for (int rank = 0; rank < 8; rank++) {
		for (int file = 0; file < 8; file++) {
			//loop over board ranks and files

			int square = rank * 8 + file;
			//convert to square index

			if (!file) {
				printf(" %d ", 8 - rank);
			}//print rank on the left side

			int piece = no_piece;

			for (int piece_on_square = P; piece_on_square <= k; piece_on_square++) {
				if (get_bit(bitboards[piece_on_square], square)) {
					piece = piece_on_square;
					break;
				}
			}
			printf(" %c", (piece == no_piece) ? ' .' : ascii_pieces[piece]);
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
	std::cout << "    fen: " << fen() << "\n";
}
void Position::print_square_board() const {
	printf("\n");
	for (int rank = 0; rank < 8; rank++) {
		for (int file = 0; file < 8; file++) {
			//loop over board ranks and files

			int square = rank * 8 + file;
			//convert to square index

			if (!file) {
				printf(" %d ", 8 - rank);
			}//print rank on the left side

			int piece = square_board[square];

			printf(" %c", (piece == no_piece) ? ' .' : ascii_pieces[piece]);
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
std::string Position::to_string() {
	std::stringstream stream = std::stringstream{};
	for (int rank = 0; rank < 8; rank++) {
		for (int file = 0; file < 8; file++) {
			//loop over board ranks and files

			int square = rank * 8 + file;
			//convert to square index

			if (!file) {
				stream<<' ' << 8 - rank;
			}//print rank on the left side

			int piece = no_piece;

			for (int piece_on_square = P; piece_on_square <= k; piece_on_square++) {
				if (get_bit(bitboards[piece_on_square], square)) {
					piece = piece_on_square;
					break;
				}
			}
			stream<< ' ' << ((piece == no_piece) ? '.' : ascii_pieces[piece]);
		}
		stream<<"\n";
	}
	stream<<"\n    a b c d e f g h \n\n";
	stream<<"    To move: "<< ((side) ? "black" : "white") <<"\n";
	stream<<"    enpassant square: "<< ((enpassant_square != a8) ? square_coordinates[enpassant_square].c_str() : "none") <<"\n";
	std::string castling_rights_str = "";
	castling_rights_str += (get_bit(castling_rights, 0)) ? "K" : "-";
	castling_rights_str += (get_bit(castling_rights, 1)) ? "Q" : "-";
	castling_rights_str += (get_bit(castling_rights, 2)) ? "k" : "-";
	castling_rights_str += (get_bit(castling_rights, 3)) ? "q" : "-";
	stream << "    castling rights: "<<castling_rights_str<<"\n";

	stream << "    halfmoves since last pawn move or capture: "<< no_pawns_or_captures <<"\n";;
	stream<<"    current halfclock turn: "<< ply<<"\n";
	stream << "    current game turn: " << ((int)ply / 2 + (side == white)) << "\n";
	stream << "    current hash: " << current_hash << "\n";
	stream << "    fen: " << fen() << "\n";
	std::string ret = std::move(stream).str();
	return ret;
}
std::string Position::square_board_to_string() const {
	std::stringstream stream = std::stringstream{};
	for (int rank = 0; rank < 8; rank++) {
		for (int file = 0; file < 8; file++) {
			//loop over board ranks and files

			int square = rank * 8 + file;
			//convert to square index

			if (!file) {
				stream << ' ' << 8 - rank;
			}//print rank on the left side

			int piece = square_board[square];

			for (int piece_on_square = P; piece_on_square <= k; piece_on_square++) {
				if (get_bit(bitboards[piece_on_square], square)) {
					piece = piece_on_square;
					break;
				}
			}
			stream << ' ' << ((piece == no_piece) ? '.' : ascii_pieces[piece]);
		}
		stream << "\n";
	}
	stream << "\n    a b c d e f g h \n\n";
	stream << "    To move: " << ((side) ? "black" : "white") << "\n";
	stream << "    enpassant square: " << ((enpassant_square != a8) ? square_coordinates[enpassant_square].c_str() : "none") << "\n";
	std::string castling_rights_str = "";
	castling_rights_str += (get_bit(castling_rights, 0)) ? "K" : "-";
	castling_rights_str += (get_bit(castling_rights, 1)) ? "Q" : "-";
	castling_rights_str += (get_bit(castling_rights, 2)) ? "k" : "-";
	castling_rights_str += (get_bit(castling_rights, 3)) ? "q" : "-";
	stream << "    castling rights: " << castling_rights_str << "\n";

	stream << "    halfmoves since last pawn move or capture: " << no_pawns_or_captures << "\n";;
	stream << "    current halfclock turn: " << ply << "\n";
	stream << "    current game turn: " << ((int)ply / 2 + (side == white)) << "\n";
	stream << "    current hash: " << current_hash << "\n";
	std::string ret = std::move(stream).str();
	return ret;
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
	assert(773 + enpassant_square % 8<781);
	ret ^= ((enpassant_square != a8) && (enpassant_square != 64)) * keys[static_cast<std::array<size_t, 781Ui64>::size_type>(773 + (enpassant_square % 8))];
	return ret % 4294967296;
}
inline void Position::update_hash(const unsigned int move) {
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
	current_hash ^= (capture) * ((is_enpassant) ? keys[to_square + ((side) ? (6 * 64 + 8) : (-8))] : keys[captured_type * 64 + to_square]);
	current_hash ^= keys[offset + to_square];

	if (is_castle) {
		const int rook_source = ((piece_type == k) * ((to_square == g8) * h8 + (to_square == c8) * a8) + (piece_type == K) * ((to_square == g1) * h1 + (to_square == c1) * a1));
		const int rook_target = (from_square + ((to_square == g8) | (to_square == g1)) - ((to_square == c8) | (to_square == c1)));
		current_hash ^= keys[offset - 128 + rook_source];
		current_hash ^= keys[offset - 128 + rook_target];
	}

	//const auto size = static_cast<std::vector<int, std::allocator<int>>::size_type>(enpassant_history.size() - 2);
	const int different_rights = castling_rights ^ castling_rights_history.back();
	current_hash ^= (get_bit(different_rights, 0)) * keys[12 * 64];
	current_hash ^= (get_bit(different_rights, 1)) * keys[12 * 64 + 1];
	current_hash ^= (get_bit(different_rights, 2)) * keys[12 * 64 + 2];
	current_hash ^= (get_bit(different_rights, 3)) * keys[12 * 64 + 3];
	current_hash ^= keys[772];

	const int old_enpassant_square = enpassant_history.back();
	current_hash ^= ((old_enpassant_square != a8) && (old_enpassant_square != 64)) * keys[static_cast<std::array<size_t, 781Ui64>::size_type>(773) + old_enpassant_square % 8];
	//undo old enpassant key
	current_hash ^= (is_double_push)*keys[773 + (from_square % 8)];

	current_hash = current_hash % 4294967296;
}
inline void Position::make_move(const unsigned int move) {
	no_pawns_or_captures_history.push_back(no_pawns_or_captures);
	enpassant_history.push_back(enpassant_square);
	move_history.push_back(move);
	castling_rights_history.push_back(castling_rights);
	hash_history.push_back(current_hash);
	ply++;//33520021
	
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

	if (is_castle) {
		const int square_offset = 56 * (side);
		const bool is_kingside = to_square > from_square;
		const int rook_source = (is_kingside) ? (h1 - square_offset) : (a1 - square_offset);
		const int rook_target = from_square + (to_square == g1 - square_offset) - (to_square == c1 - square_offset);
		pop_bit(bitboards[piece_type - 2], rook_source);
		//pop rooks if move is castling
		bitboards[piece_type - 2] |= ((1ULL) << (rook_target));
		//set rook if move is castling
		castling_rights &= ~((1ULL) << (2 * (piece_type == k)));
		castling_rights &= ~((1ULL) << (1 + 2 * (piece_type == k)));
		//pop castle right bits if move is castling

		square_board[rook_source] = no_piece;
		square_board[rook_target] = piece_type - 2;
	}
	else {
		const bool bK = (piece_type == k);
		const bool bR = (piece_type == r);
		pop_bit(castling_rights, (bK || (bR && (from_square == a8)) || (to_square == a8)) ? 3 : 4);
		pop_bit(castling_rights, (bK || (bR && (from_square == h8)) || (to_square == h8)) ? 2 : 4);
		const bool wK = (piece_type == K);
		const bool wR = (piece_type == R);
		pop_bit(castling_rights, (wK || (wR && (from_square == a1)) || (to_square == a1)) ? 1 : 4);
		pop_bit(castling_rights, (wK || (wR && (from_square == h1)) || (to_square == h1)) ? 0 : 4);
		//pop castle right bits
	}
	if (capture) {
		if (is_enpassant) {
			const int captured_pawn_sqare = 8 * ((!side) - (side));
			square_board[to_square + captured_pawn_sqare] = no_piece;
			pop_bit(bitboards[captured_type], to_square + captured_pawn_sqare);
		}
		else pop_bit(bitboards[captured_type], to_square);
	}

	const bool is_promotion = promoted_type != no_piece;
	const int true_piece_type = (!(is_promotion)) * piece_type + (is_promotion) * (promoted_type);
	pop_bit(bitboards[piece_type], from_square);
	set_bit(bitboards[true_piece_type], to_square);
	square_board[from_square] = no_piece;
	square_board[to_square] = true_piece_type;
	//make move of piece

	side = !side;
	occupancies[0] = (bitboards[0] | bitboards[1]) | (bitboards[2] | bitboards[3]) | (bitboards[4] | bitboards[5]);
	occupancies[1] = bitboards[6] | bitboards[7] | bitboards[8] | bitboards[9] | bitboards[10] | bitboards[11];
	occupancies[2] = occupancies[0] | occupancies[1];

	current_hash = get_hash();
	//IN CASE I WANT TO FIX HASH UPDATING
	//update_hash(move);
	//const U64 hash = get_hash();
	//if (current_hash != get_hash()) {
	//	std::stringstream stream{};
	//	stream<<"Updating hash did not yield correct result. Got " << current_hash<<" when fully calculating the hash yields "<<hash;
	//	std::string str = std::move(stream).str();
	//	throw Position_Error{str};
	//}
	//if (!boardsMatch()) {
	//	std::string str = "\n" + to_string();
	//	str += "| last move: \n" + move_to_string(move);
	//	str += square_board_to_string();
	//	std::cout << get_move_history()<<std::endl;
	//	throw Position_Error{str};
	//}
}
inline void Position::unmake_move() {
	const unsigned int move = move_history.back();
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
	square_board[from_square] = piece_type;
	square_board[to_square] = no_piece;

	const bool is_promotion = promoted_type != no_piece;
	bitboards[(is_promotion)*promoted_type] &= ~(((is_promotion) * 1ULL) << (to_square));
	//branchlessly pop the piece that was promoted. if move was not a promotion the white pawns are and'ed with a bitboard of ones, thus it would not change

	if (capture) {//set the captured piece
		if (is_enpassant) {
			const int captured_square = to_square + 8 * ((side)-(!side));
			square_board[captured_square] = captured_type;
			bitboards[captured_type] |= (1ULL << captured_square);
		}
		else {
			square_board[to_square] = captured_type;
			bitboards[captured_type] |= (1ULL << to_square);
		}
	}

	if (is_castle) {
		const int square_offset = 56 * (side);
		const bool is_kingside = to_square > from_square;
		const int rook_source = (is_kingside) ? (h8 + square_offset) : (a8 + square_offset);
		const int rook_target = (is_kingside) ? (f8 + square_offset) : (d8 + square_offset);
		const int rook_type = piece_type - 2; //since piece_type is the king of the castling color you can calc the rook_type
		bitboards[rook_type] |= (1ULL << rook_source);
		bitboards[rook_type] &= ~(1ULL << rook_target);
		//branchlessly set and pop rook bits if move was castling
		square_board[rook_target] = no_piece;
		square_board[rook_source] = rook_type;
	}
	occupancies[0] = bitboards[0] | bitboards[1] | bitboards[2] | bitboards[3] | bitboards[4] | bitboards[5];
	occupancies[1] = bitboards[6] | bitboards[7] | bitboards[8] | bitboards[9] | bitboards[10] | bitboards[11];
	occupancies[2] = occupancies[0] | occupancies[1];
	side = !side;
	//if (!boardsMatch()) {
	//	std::string str = "\n" + to_string();
	//	str += "| last move: \n" + move_to_string(move);
	//	str += square_board_to_string();
	//	std::cout << get_move_history() << std::endl;
	//	throw Position_Error{str};
	//}
}//position fen 8/2k4p/1b6/3P3p/p7/5K1P/P4P2/5q2 w - - 0 39