#pragma once
#include "bitboard.h"
#define encode_move(from,to,piece,capture,promotion,capture_flag,double_push,castling,enpassant) ((from)|((to)<<6)|((piece)<<12)|((capture)<<16)|((promotion)<<20)|((capture_flag)<<24)|((double_push)<<25)|((castling)<<26)|((enpassant)<<27))
#define get_from_square(move) ((move)&0x3f)
#define get_to_square(move) (((move)>>6)&0x3f)
#define get_piece_type(move) (((move)>>12)&0xf)
#define get_captured_type(move) (((move)>>16)&0xf)
#define get_promotion_type(move) (((move)>>20)&0xf)

#define get_capture_flag(move) ((move)&0x1000000)
#define get_double_push_flag(move) ((move)&0x2000000)
#define get_castling_flag(move) ((move)&0x4000000)
#define get_enpassant_flag(move) ((move)&0x8000000)

#define set_from_square(move,from) (move)=(((move)&0xfffffc0)|(from))
#define set_to_square(move,to) (move)=(((move)&0xffff03f)|((to)<<6))
#define set_piece_type(move,piece) (move)=(((move)&0xfff0fff)|((piece)<<12))
#define set_captured_type(move,capture) (move)=(((move)&0xff0ffff)|((capture)<<16))
#define set_promotion_type(move,promotion) (move)=(((move)&0xf0fffff)|((promotion)<<20))

#define set_capture_flag(move,capture) ((move)=(((move)&0xeffffff)|((capture)<<24)))
#define set_double_push_flag(move,double_push) (move)=(((move)&0xdffffff)|((double_push)<<25))
#define set_castling_flag(move,castling) (move)=(((move)&0xbffffff)|((castling)<<26))
#define set_enpassant_flag(move,enpassant) (move)=(((move)&0x7ffffff)|((enpassant)<<27))
constexpr char ascii_promotion_symbols[] = "pnbrqkpnbrqk    ";
void print_move(int move);
std::string uci(const int move);
void print_move_bits(int move);
/*
new Move encoding:
				binary:												hexadecimal:		negation:
0000 0000 0000 0000 0000 0011 1111 from square				0x3f					0xfffffc0
0000 0000 0000 0000 1111 1100 0000 to square				0xfc0					0xffff03f
0000 0000 0000 1111 0000 0000 0000 piece type				0xf000					0xfff0fff
0000 0000 1111 0000 0000 0000 0000 captured type			0xf0000					0xff0ffff
0000 1111 0000 0000 0000 0000 0000 promoted type			0xf00000				0xf0fffff
0001 0000 0000 0000 0000 0000 0000 capture flag				0x1000000				0xeffffff
0010 0000 0000 0000 0000 0000 0000 double pawn push flag	0x2000000				0xdffffff
0100 0000 0000 0000 0000 0000 0000 castling flag			0x4000000				0xbffffff
1000 0000 0000 0000 0000 0000 0000 enpassant flag			0x8000000				0x7ffffff
*/