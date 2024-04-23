#pragma once
#include<cstdlib>
#include"position.hpp"
/*does not work currently*/
const vector<U64> bm{ {1421333693633924095ULL,1924174376277378102ULL,85642061323208706ULL,5634037167685634ULL,2452775214801485952ULL,2594364893654286336ULL,351749660022469463ULL,
994655748440981656ULL,2946971884703580214ULL,822495102338873337ULL,18018813803252224ULL,102465826132592642ULL,4611687187732238352ULL,9295434308250501124ULL,4435087606549578689ULL,
1669607696868884441ULL,3404632479751372815ULL,9042418271848978ULL,10376865305292177416ULL,74311603914293248ULL,4653344647851032642ULL,289638309385339402ULL,4122060509885480915ULL,
1078400727717109778ULL,4662208609062912ULL,72640335470133776ULL,5207023186696225280ULL,2323870602970107920ULL,5764890028886999044ULL,648686021999330304ULL,14610807489285882920ULL,
9367631262029971968ULL,286019064715328ULL,19150333614231040ULL,147493059595338313ULL,342282402133051904ULL,9531870861903270144ULL,9223530385858692224ULL,9572356821370882ULL,
10423604432129245696ULL,184659761139072991ULL,3744754732103884789ULL,948025393338798098ULL,2254136561108992ULL,73045032956857344ULL,5787899073167872ULL,3476942787230728703ULL,
3449785683287665471ULL,2483752419887516035ULL,3063585200252556151ULL,585468127786573824ULL,9086553616286212ULL,4611936844555486212ULL,503136740900945713ULL,3026629147789541324ULL,
2738194950066243769ULL,4034105229724797445ULL,2846903918030952736ULL,577727393997852682ULL,9223442491508819968ULL,5784900350423204353ULL,1297335213127997759ULL,1240463110869033229ULL,936958901420498994ULL}
};
const array<short, 64> bs{ {5,3,5,5,5,5,3,5,
4,4,5,5,5,5,3,2,
4,5,7,7,7,7,4,4,
5,5,7,9,9,7,5,5,
5,5,7,9,9,7,5,5,
4,4,7,7,7,7,4,4,
3,4,5,5,5,4,3,4,
5,4,5,5,5,5,4,5}
};

void generateMagicsBishops() {
	array<U64, 4096> arr{};
	array<U64, 64> magics{};
	array<short, 64> shifts{};
	U64 fails{ 0ULL };
	for (int sq = 0; sq < 64; ++sq) {
	bishop_start:
		for (int i = 0; i < arr.size(); ++i) { arr[i] = 0ULL; }
		const U64 piece = 1ULL << sq;
		const U64 board = bishop_masks[sq];
		//print_bitboard(board);
		const auto bits = count_bits(board);
		U64 magic = (U64)(rand() % (1ULL << 15)) ^ (U64)(rand() % (1ULL << 15)) << 15 ^ (U64)(rand() % (1ULL << 15)) << 31 ^ (U64)(rand() % (1ULL << 15)) << 47;
		size_t target_shift{ bishopShifts[sq] - 2ULL + (fails>= 10000000)};
		if (fails == 10000000) cout << "switching to accepting worse magics" << endl;
		if (fails == 50000000) {
			magic = bishop_magics[sq];
			target_shift = bishopShifts[sq];
		}
		if (fails == 0) {
			magic = bm[sq];
			target_shift = bs[sq];
		}
		for (int i = 1; i < 1ULL << bits; ++i) {
			U64 brd{ board };
			U64 mask{ 0ULL };
			size_t bitindex{ 0 };
			while (brd) {
				mask |= get_ls1b(brd) * (((U64)i & 1ULL << (bitindex++)) != 0);
				brd = pop_ls1b(brd);
			}
			const U64 maskcpy{ mask };
			U64 occ{ mask };
			occ &= bishop_masks[sq];
			occ *= bishop_magics[sq];
			occ >>= 64ULL - bishopShifts[sq];
			mask *= magic;
			mask >>= 64ULL - target_shift;
			if (arr[mask]) {
				if (bishop_attacks[sq][occ] != arr[mask]) {
					//cout << magic <<" failed at configuration " << i << " / " << (1ULL << bits) << endl;
					//print_bitboard(maskcpy);
					//print_bitboard(arr[mask]);
					//print_bitboard(bishop_attacks[sq][occ]);
					++fails;
					goto bishop_start;
				}
			}
			else {
				arr[mask] = bishop_attacks[sq][occ];
			}
		}
		cout << "found magic " << magic << "ULL with " << target_shift << " shifts for square " << sq << " after " << fails << " fails" << endl;
		shifts[sq] = target_shift;
		magics[sq] = magic;
		fails = 0ULL;
		size_t size=0;
		for (size_t i = arr.size()-1; i >= 0; --i) {
			if (arr[i] != 0ULL) {
				size = i + 1;
				break;
			}
		}
		cout << "size: " << size << "\n";
		cout << "{\n";
		for (int i = 0; i < size; ++i) {
			if(i) cout << ",";
			if (i % 8 == 7) cout << "\n";
			cout << arr[i];
		}
		cout << "}\n";

	}
	cout << "new magics: {";
	for (int i = 0; i < 64; ++i) {
		if (i) cout << ",";
		if (i % 8 == 7 && i != 63) cout << "\n";
		cout << magics[i];
	}
	cout << "}";
}
void generateMagicsRooks() {
	array<U64, 4096> arr{};
	array<U64, 64> magics{};
	U64 fails{ 0ULL };
	for (int sq = 0; sq < 64; ++sq) {
	rook_start:
		for (int i = 0; i < arr.size(); ++i) { arr[i] = 0ULL; }
		const U64 piece = 1ULL << sq;
		const U64 board = rook_masks[sq];
		//print_bitboard(board);
		const auto bits = count_bits(board);
		U64 magic = (U64)(rand() % (1ULL << 15)) ^ (U64)(rand() % (1ULL << 15)) << 15 ^ (U64)(rand() % (1ULL << 15)) << 31 ^ (U64)(rand() % (1ULL << 15)) << 47;
		size_t target_shift{ rookShifts[sq] - 2ULL + (fails >= 10000000) };
		if (fails == 10000000) cout << "switching to accepting worse magics" << endl;
		if (fails == 50000000) {
			magic = rook_magics[sq];
			target_shift = rookShifts[sq];
		}
		for (int i = 1; i < 1ULL << bits; ++i) {
			U64 brd{ board };
			U64 mask{ 0ULL };
			size_t bitindex{ 0 };
			while (brd) {
				mask |= get_ls1b(brd) * (((U64)i & 1ULL << (bitindex++)) != 0);
				brd = pop_ls1b(brd);
			}
			const U64 maskcpy{ mask };
			U64 occ{ mask };
			occ &= rook_masks[sq];
			occ *= rook_magics[sq];
			occ >>= 64ULL - rookShifts[sq];
			mask *= magic;
			mask >>= 64ULL - target_shift;
			if (arr[mask]) {
				if (rook_attacks[sq][occ] != arr[mask]) {
					//cout << magic <<" failed at configuration " << i << " / " << (1ULL << bits) << endl;
					//print_bitboard(maskcpy);
					//print_bitboard(arr[mask]);
					//print_bitboard(rook_attacks[sq][occ]);
					++fails;
					goto rook_start;
				}
			}
			else {
				arr[mask] = rook_attacks[sq][occ];
			}
		}
		cout << "found magic " << magic << "ULL with " << target_shift << " shifts for square " << sq << " after " << fails << " fails" << endl;
		magics[sq] = magic;
		fails = 0ULL;
	}
	cout << "new magics: {";
	for (int i = 0; i < 64; ++i) {
		if (i) cout << ",";
		if (i % 8 == 7 && i != 63) cout << "\n";
		cout << magics[i];
	}
	cout << "}";
}