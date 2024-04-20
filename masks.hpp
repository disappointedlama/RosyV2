#pragma once
#include "bitboard.hpp"
#include "rookAttacks.hpp"
#include "bishopAttacks.hpp"
#include "immintrin.h"
#include "intrin.h"
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
static constexpr U64 files[8] = {aFile,bFile,cFile,dFile,eFile,fFile,gFile,hFile};
static constexpr U64 notHFile = ~hFile;
static constexpr U64 notAFile = ~aFile;
static constexpr U64 whiteSquares = 12273903644374837845ULL;
static constexpr U64 blackSquares = 6172840429334713770ULL;
static constexpr U64 centralSquares = 0x1818000000;
static constexpr U64 notEdges = 0x7E7E7E7E7E7E00;
static constexpr U64 bKingposABCPawnShield = 7ULL;
static constexpr U64 wKingposABCPawnShield = bKingposABCPawnShield<<7*8;
static constexpr U64 bKingposFGHPawnShield = 7ULL << 5;
static constexpr U64 wKingposFGHPawnShield = bKingposFGHPawnShield << 7 * 8;
static constexpr U64 wABCPawnShield = (wKingposABCPawnShield >> 8) | (wKingposABCPawnShield >> 16);
static constexpr U64 bABCPawnShield = (bKingposABCPawnShield << 8) | (bKingposABCPawnShield << 16);
static constexpr U64 wFGHPawnShield = (wKingposFGHPawnShield >> 8) | (wKingposFGHPawnShield >> 16);
static constexpr U64 bFGHPawnShield = (bKingposFGHPawnShield << 8) | (bKingposFGHPawnShield << 16);
static constexpr U64 passed_pawn_masks[2][64] = {
	{0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL,
	3ULL, 7ULL, 14ULL, 28ULL, 56ULL, 112ULL, 224ULL, 192ULL,
	771ULL, 1799ULL, 3598ULL, 7196ULL, 14392ULL, 28784ULL, 57568ULL, 49344ULL,
	197379ULL, 460551ULL, 921102ULL, 1842204ULL, 3684408ULL, 7368816ULL, 14737632ULL, 12632256ULL,
	50529027ULL, 117901063ULL, 235802126ULL, 471604252ULL, 943208504ULL, 1886417008ULL, 3772834016ULL, 3233857728ULL,
	12935430915ULL, 30182672135ULL, 60365344270ULL, 120730688540ULL, 241461377080ULL, 482922754160ULL, 965845508320ULL, 827867578560ULL,
	3311470314243ULL, 7726764066567ULL, 15453528133134ULL, 30907056266268ULL, 61814112532536ULL, 123628225065072ULL, 247256450130144ULL, 211934100111552ULL,
	847736400446211ULL, 1978051601041159ULL, 3956103202082318ULL, 7912206404164636ULL, 15824412808329272ULL, 31648825616658544ULL, 63297651233317088ULL, 54255129628557504},
	{217020518514230016ULL, 506381209866536704ULL, 1012762419733073408ULL, 2025524839466146816ULL, 4051049678932293632ULL, 8102099357864587264ULL, 16204198715729174528ULL, 13889313184910721024ULL,
	217020518514229248ULL, 506381209866534912ULL, 1012762419733069824ULL, 2025524839466139648ULL, 4051049678932279296ULL, 8102099357864558592ULL, 16204198715729117184ULL, 13889313184910671872ULL,
	217020518514032640ULL, 506381209866076160ULL, 1012762419732152320ULL, 2025524839464304640ULL, 4051049678928609280ULL, 8102099357857218560ULL, 16204198715714437120ULL, 13889313184898088960ULL,
	217020518463700992ULL, 506381209748635648ULL, 1012762419497271296ULL, 2025524838994542592ULL, 4051049677989085184ULL, 8102099355978170368ULL, 16204198711956340736ULL, 13889313181676863488ULL,
	217020505578799104ULL, 506381179683864576ULL, 1012762359367729152ULL, 2025524718735458304ULL, 4051049437470916608ULL, 8102098874941833216ULL, 16204197749883666432ULL, 13889312357043142656ULL,
	217017207043915776ULL, 506373483102470144ULL, 1012746966204940288ULL, 2025493932409880576ULL, 4050987864819761152ULL, 8101975729639522304ULL, 16203951459279044608ULL, 13889101250810609664ULL,
	216172782113783808ULL, 504403158265495552ULL, 1008806316530991104ULL, 2017612633061982208ULL, 4035225266123964416ULL, 8070450532247928832ULL, 16140901064495857664ULL, 13835058055282163712ULL,
	0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL}
};
static constexpr U64 neighbour_pawn_masks[8] = { 565157600297472ULL,1412894000743680ULL,2825788001487360ULL,5651576002974720ULL,11303152005949440ULL,22606304011898880ULL,45212608023797760ULL,18085043209519104ULL };
static constexpr array<U64, 64> init_doubled_pawn_masks(array<U64, 64> ret) {
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
static constexpr array<U64, 64> doubled_pawn_masks = init_doubled_pawn_masks(array<U64, 64>{});
static constexpr array<U64, 64> init_doubled_pawn_reset_masks(array<U64, 64> ret) {
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
static constexpr array<U64, 64> doubled_pawn_reset_masks = init_doubled_pawn_reset_masks(array<U64, 64>{});
static constexpr array<array<U64, 64>, 2> init_front_pawn_attack_spans(array<array<U64, 64>, 2> ret) {
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
static constexpr array<array<U64, 64>, 2> front_pawn_attack_spans = init_front_pawn_attack_spans(array<array<U64, 64>, 2>{});
static constexpr array<array<U64, 64>, 64> init_checkingRays(array<array<U64, 64>, 64>ret) {
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			ret[i][j] = 0ULL;
		}
	}
	for (int i = 0; i < 64; i++) {
		U64 rookAttacks = get_rook_attacks(0ULL, i);
		while (rookAttacks) {
			const U64 isolated = _blsi_u64(rookAttacks);
			const int square = bitscan(isolated);
			U64 tmpAttacks = get_rook_attacks(1ULL << square, i);
			const U64 other = get_rook_attacks(1ULL<<i, square);
			ret[i][square] = (other | isolated) & tmpAttacks;
			rookAttacks = _blsr_u64(rookAttacks);
		}
		U64 bishopAttacks = get_bishop_attacks(0ULL, i);
		while (bishopAttacks) {
			const U64 isolated = _blsi_u64(bishopAttacks);
			const int square = bitscan(isolated);
			U64 tmpAttacks = get_bishop_attacks(1ULL << square, i);
			const U64 other = get_bishop_attacks(1ULL<<i, square);
			ret[i][square] = (other | isolated) & tmpAttacks;
			bishopAttacks = _blsr_u64(bishopAttacks);
		}
	}
	return ret;
}
static const array<array<U64, 64>, 64> checkingRays = init_checkingRays(array<array<U64, 64>, 64>{});
static constexpr array<array<U64, 64>, 64> init_connectionRays(array<array<U64, 64>, 64>ret) {
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			ret[i][j] = 0;
		}
	}
	for (int i = 0; i < 64; i++) {
		const U64 sq = 1ULL << i;
		U64 rookAttacks = get_rook_attacks(0ULL, i);
		while (rookAttacks) {
			const U64 isolated = _blsi_u64(rookAttacks);
			const int square = bitscan(isolated);
			U64 tmpAttacks = get_rook_attacks(1ULL << square, i);
			const U64 other = get_rook_attacks(1ULL<<i, square);
			ret[i][square] = (other | isolated) & (tmpAttacks | sq);
			rookAttacks = _blsr_u64(rookAttacks);
		}
		U64 bishopAttacks = get_bishop_attacks(0ULL, i);
		while (bishopAttacks) {
			const U64 isolated = _blsi_u64(bishopAttacks);
			const int square = bitscan(isolated);
			U64 tmpAttacks = get_bishop_attacks(1ULL << square, i);
			const U64 other = get_bishop_attacks(1ULL<<i, square);
			ret[i][square] = (other | isolated) & (tmpAttacks | sq);
			bishopAttacks = _blsr_u64(bishopAttacks);
		}
	}
	return ret;
}
static const array<array<U64, 64>, 64> connectionRays = init_connectionRays(array<array<U64, 64>, 64>{});