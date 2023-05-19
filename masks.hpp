#pragma once
#include "bitboard.hpp"
#include <array>
static constexpr U64 rank8 = (1ULL << 8) - 1ULL;
static constexpr U64 rank7 = rank8 << 8;
static constexpr U64 rank6 = rank7 << 8;
static constexpr U64 rank5 = rank6 << 8;
static constexpr U64 rank4 = rank5 << 8;
static constexpr U64 rank3 = rank4 << 8;
static constexpr U64 rank2 = rank3 << 8;
static constexpr U64 rank1 = rank2 << 8;
static constexpr U64 aFile = 0x101010101010101;
static constexpr U64 bFile = 0x202020202020202;
static constexpr U64 cFile = 0x404040404040404;
static constexpr U64 dFile = 0x808080808080808;
static constexpr U64 eFile = 0x1010101010101010;
static constexpr U64 fFile = 0x2020202020202020;
static constexpr U64 gFile = 0x4040404040404040;
static constexpr U64 hFile = 0x8080808080808080;
static constexpr U64 notHFile = ~hFile;
static constexpr U64 notAFile = ~aFile;
static constexpr U64 whiteSquares = 12273903644374837845ULL;
static constexpr U64 blackSquares = 6172840429334713770ULL;
static constexpr U64 centralSquares = 0x1818000000;
static constexpr U64 notEdges = 0x7E7E7E7E7E7E00;
static constexpr U64 passed_pawn_masks[2][64] = {
	{0, 0, 0, 0, 0, 0, 0, 0,
	3, 7, 14, 28, 56, 112, 224, 192,
	771, 1799, 3598, 7196, 14392, 28784, 57568, 49344,
	197379, 460551, 921102, 1842204, 3684408, 7368816, 14737632, 12632256,
	50529027, 117901063, 235802126, 471604252, 943208504, 1886417008, 3772834016, 3233857728,
	12935430915, 30182672135, 60365344270, 120730688540, 241461377080, 482922754160, 965845508320, 827867578560,
	3311470314243, 7726764066567, 15453528133134, 30907056266268, 61814112532536, 123628225065072, 247256450130144, 211934100111552,
	847736400446211, 1978051601041159, 3956103202082318, 7912206404164636, 15824412808329272, 31648825616658544, 63297651233317088, 54255129628557504},
	{217020518514230016, 506381209866536704, 1012762419733073408, 2025524839466146816, 4051049678932293632, 8102099357864587264, 16204198715729174528, 13889313184910721024,
	217020518514229248, 506381209866534912, 1012762419733069824, 2025524839466139648, 4051049678932279296, 8102099357864558592, 16204198715729117184, 13889313184910671872,
	217020518514032640, 506381209866076160, 1012762419732152320, 2025524839464304640, 4051049678928609280, 8102099357857218560, 16204198715714437120, 13889313184898088960,
	217020518463700992, 506381209748635648, 1012762419497271296, 2025524838994542592, 4051049677989085184, 8102099355978170368, 16204198711956340736, 13889313181676863488,
	217020505578799104, 506381179683864576, 1012762359367729152, 2025524718735458304, 4051049437470916608, 8102098874941833216, 16204197749883666432, 13889312357043142656,
	217017207043915776, 506373483102470144, 1012746966204940288, 2025493932409880576, 4050987864819761152, 8101975729639522304, 16203951459279044608, 13889101250810609664,
	216172782113783808, 504403158265495552, 1008806316530991104, 2017612633061982208, 4035225266123964416, 8070450532247928832, 16140901064495857664, 13835058055282163712,
	0, 0, 0, 0, 0, 0, 0, 0}
};
static constexpr U64 neighbour_pawn_masks[8] = { 565157600297472,1412894000743680,2825788001487360,5651576002974720,11303152005949440,22606304011898880,45212608023797760,18085043209519104 };
static constexpr std::array<U64, 64> init_doubled_pawn_masks(std::array<U64, 64> ret) {
	for (int i = 8; i < 56; i++) {
		U64 isolated = 1ULL << i;
		U64 span = isolated << 8;
		span |= span << 8;
		span |= span << 8;
		span |= span << 8;
		span |= span << 8;
		span &= ~rank1;
		ret[i] = span;
	}
	return ret;
}
static constexpr std::array<U64, 64> doubled_pawn_masks = init_doubled_pawn_masks(std::array<U64, 64>{});
static constexpr std::array<U64, 64> init_doubled_pawn_reset_masks(std::array<U64, 64> ret) {
	for (int i = 8; i < 56; i++) {
		U64 isolated = 1ULL << i;
		U64 span = isolated;
		span |= span << 8;
		span |= span << 8;
		span |= span << 8;
		span |= span << 8;
		span |= span << 8;
		span &= ~rank1;
		ret[i] = span;
	}
	return ret;
}
static constexpr std::array<U64, 64> doubled_pawn_reset_masks = init_doubled_pawn_reset_masks(std::array<U64, 64>{});
static constexpr std::array<std::array<U64, 64>, 2> init_front_pawn_attack_spans(std::array<std::array<U64, 64>, 2> ret) {
	for (int i = 8; i < 64; i++) {
		U64 isolated = 1ULL << i;
		U64 span = ((isolated << 7) & notHFile) | ((isolated << 9) & notAFile);
		span |= span << 8;
		span |= span << 8;
		span |= span << 8;
		span |= span << 8;
		span |= span << 8;
		ret[1][i] = span;
	}
	for (int i = 0; i < 56; i++) {
		U64 isolated = 1ULL << i;
		U64 span = ((isolated >> 7) & notAFile) | ((isolated >> 9) & notHFile);
		span |= span >> 8;
		span |= span >> 8;
		span |= span >> 8;
		span |= span >> 8;
		span |= span >> 8;
		ret[0][i] = span;
	}
	return ret;
}
static constexpr std::array<std::array<U64, 64>, 2> front_pawn_attack_spans = init_front_pawn_attack_spans(std::array<std::array<U64, 64>, 2>{});