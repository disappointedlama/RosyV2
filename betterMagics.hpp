#pragma once
#include<cstdlib>
#include<iostream>
using std::cout, std::endl;
#include"position.hpp"
#include<fstream>

inline void generateMagicsBishops() {
	array<U64, 4096> arr{};
	array<U64, 64> magics{};
	array<short, 64> shifts{};
	array<int,64> offsets{};
	U64 fails{ 0ULL };
	U64 totalEntries{0};
	for (int sq = 0; sq < 64; ++sq) {
		offsets[sq]=totalEntries;
	bishop_start:
		for (int i = 0; i < arr.size(); ++i) { arr[i] = ~0ULL; }
		const U64 piece = 1ULL << sq;
		const U64 board = bishop_masks[sq];
		//print_bitboard(board);
		const auto bits = count_bits(board);
		U64 magic = (U64)(rand() % (1ULL << 15)) ^ (U64)(rand() % (1ULL << 15)) << 15 ^ (U64)(rand() % (1ULL << 15)) << 31 ^ (U64)(rand() % (1ULL << 15)) << 47;
		size_t target_shift{ bishopShifts[sq] - 2ULL + (fails>= 10000000)};
		//if (fails == 10000000) cout << "switching to accepting worse magics" << endl;
		if (fails == 50000000) {
			magic = bishop_magics[sq];
			target_shift = bishopShifts[sq];
		}
		if (fails == 0) {
			magic = bishop_magics[sq];
			target_shift = bishopShifts[sq];
		}
		//cout<<"square "<<sq<<" bits: "<<bits<<"\n";
		for (int i = 0; i < 1ULL << bits; ++i) {
			U64 brd{ board };
			U64 mask{ 0ULL };
			size_t bitindex{ 0 };
			while (brd) {
				mask |= get_ls1b(brd) * (((U64)i & (1ULL << (bitindex++))) != 0);
				brd = pop_ls1b(brd);
			}
			const U64 maskcpy{ mask };
			U64 occ{ mask };
			occ &= bishop_masks[sq];
			occ *= bishop_magics[sq];
			occ >>= 64ULL - bishopShifts[sq];
			mask &= bishop_masks[sq];
			mask *= magic;
			mask >>= 64ULL - target_shift;
			if (arr[mask] != ~0ULL && bishop_attacks[bishop_offsets[sq]+occ] != arr[mask]) {
				//cout << magic <<" failed at configuration " << i << " / " << (1ULL << bits) << endl;
				//print_bitboard(maskcpy);
				//print_bitboard(arr[mask]);
				//print_bitboard(bishop_attacks[sq][occ]);
				++fails;
				goto bishop_start;
			}
			arr[mask] = bishop_attacks[bishop_offsets[sq]+occ];
		}
		//cout << "found magic " << magic << "ULL with " << target_shift << " shifts for square " << sq << " after " << fails << " fails" << endl;
		shifts[sq] = target_shift;
		magics[sq] = magic;
		fails = 0ULL;
		size_t size=0;
		for (size_t i = arr.size()-1; i >= 0; --i) {
			if (arr[i] != ~0ULL) {
				size = i + 1;
				break;
			}
		}
		totalEntries+=size;
		//cout << "size: " << size << "\n";
		for (int i = 0; i < size; ++i) {
			if (i % 8 == 0) cout << "\n";
			cout << "," << arr[i] << "ULL";
		}
	}
	cout<<"\ntotal entries "<<totalEntries<<"\n";
	cout << "new magics: {";
	for (int i = 0; i < 64; ++i) {
		if (i) cout << ",";
		if (i % 8 == 7 && i != 63) cout << "\n";
		cout << magics[i] << "ULL";
	}
	cout << "}";
	cout << "\nnew shifts: \n{";
	for (int i = 0; i < 64; ++i) {
		if (i) cout << ",";
		if (i % 8 == 7 && i != 63) cout << "\n";
		cout << shifts[i];
	}
	cout << "}";
	cout << "\noffsets: \n{";
	for (int i = 0; i < 64; ++i) {
		if (i) cout << ",";
		if (i % 8 == 7 && i != 63) cout << "\n";
		cout << offsets[i];
	}
	cout << "}";
}
inline void generateMagicsRooks() {
	std::ofstream file{"./tmp.cpp"};
	if(file.is_open()){
		array<U64, 4096> arr{};
		array<U64, 64> magics{};
		array<short, 64> shifts{};
		U64 fails{ 0ULL };
		array<int,64> offsets{};
		U64 totalEntries{0};
		for (int sq = 0; sq < 64; ++sq) {
			offsets[sq]=totalEntries;
		rook_start:
			for (int i = 0; i < arr.size(); ++i) { arr[i] = ~0ULL; }
			const U64 piece = 1ULL << sq;
			const U64 board = rook_masks[sq];
			//print_bitboard(board);
			const auto bits = count_bits(board);
			U64 magic{0ULL};
			for(int i=0;i<64;++i){
				magic|= (U64)(rand()%2==0)<<i;
			}
			size_t target_shift{ rookShifts[sq] - 1ULL};
			//if (fails == 10000000) cout << "switching to accepting worse magics" << endl;
			if (fails == 50000000) {
				magic = rook_magics[sq];
				target_shift = rookShifts[sq];
			}
			if (fails == 0) {
				magic = rook_magics[sq];
				target_shift = rookShifts[sq];
			}
			//cout<<"square "<<sq<<" bits: "<<bits<<"\n";
			for (int i = 0; i < 1ULL << bits; ++i) {
				U64 brd{ board };
				U64 mask{ 0ULL };
				size_t bitindex{ 0 };
				while (brd) {
					mask |= get_ls1b(brd) * (((U64)i & (1ULL << (bitindex++))) != 0);
					brd = pop_ls1b(brd);
				}
				const U64 maskcpy{ mask };
				U64 occ{ mask };
				occ &= rook_masks[sq];
				occ *= rook_magics[sq];
				occ >>= 64ULL - rookShifts[sq];
				mask &= rook_masks[sq];
				mask *= magic;
				mask >>= 64ULL - target_shift;
				if (arr[mask] != ~0ULL && rook_attacks[rook_offsets[sq]+occ] != arr[mask]) {
					//cout << magic <<" failed at configuration " << i << " / " << (1ULL << bits) << endl;
					//print_bitboard(maskcpy);
					//print_bitboard(arr[mask]);
					//print_bitboard(rook_attacks[sq][occ]);
					++fails;
					goto rook_start;
				}
				arr[mask] = rook_attacks[rook_offsets[sq]+occ];
			}
			cout << "found magic " << magic << "ULL with " << target_shift << " shifts for square " << sq << " after " << fails << " fails" << endl;
			shifts[sq] = target_shift;
			magics[sq] = magic;
			fails = 0ULL;
			size_t size=0;
			for (size_t i = arr.size()-1; i >= 0; --i) {
				if (arr[i] != ~0ULL) {
					size = i + 1;
					break;
				}
			}
			totalEntries+=size;
			//cout << "size: " << size << "\n";
			for (int i = 0; i < size; ++i) {
				if (i % 8 == 0) file << "\n";
				file << "," << arr[i] << "ULL";
			}
		}
		cout<<"total entries "<<totalEntries<<"\n";
		file << "new magics: {";
		for (int i = 0; i < 64; ++i) {
			if (i) file << ",";
			if (i % 8 == 7 && i != 63) file << "\n";
			file << magics[i] << "ULL";
		}
		file << "}";
		file << "\nnew shifts: \n{";
		for (int i = 0; i < 64; ++i) {
			if (i) file << ",";
			if (i % 8 == 7 && i != 63) file << "\n";
			file << shifts[i];
		}
		file << "}\n";
		file << "\noffsets: \n{";
		for (int i = 0; i < 64; ++i) {
			if (i) file << ",";
			if (i % 8 == 7 && i != 63) file << "\n";
			file << offsets[i];
		}
		file << "}";
		file.close();
	}
}