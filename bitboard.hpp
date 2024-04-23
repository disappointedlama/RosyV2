#pragma once
#include <iostream>
#include <string>
#include <bit>
#include <array>
using std::string, std::array;
#define U64 unsigned long long
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square)))
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
#define ones_decrement(bitboard) ((U64)bitboard - 1)
#define twos_complement(bitboard) ((~(U64)bitboard) + 1)
#if defined(_WIN64)
#include "intrin.h"
#define get_ls1b(bitboard) (_blsi_u64(bitboard))
#define pop_ls1b(bitboard) (_blsr_u64(bitboard))
#define count_bits(bitboard) (std::_Popcount(bitboard))
#else
#define get_ls1b(bitboard) ((U64)bitboard & -(U64)bitboard)
#define pop_ls1b(bitboard) ((U64)bitboard & ones_decrement((U64)bitboard))

/*
https://www.chessprogramming.org/Population_Count#The_PopCount_routine
*/
constexpr U64 k1 = 0x5555555555555555ULL;
constexpr U64 k2 = 0x3333333333333333ULL;
constexpr U64 k4 = 0x0f0f0f0f0f0f0f0fULL;
constexpr U64 kf = 0x0101010101010101ULL;
constexpr int count_bits (U64 x) {
    x =  x       - ((x >> 1)  & k1);
    x = (x & k2) + ((x >> 2)  & k2);
    x = (x       +  (x >> 4)) & k4 ;
    x = (x * kf) >> 56;
    return (int) x;
}

#endif
#if defined(__GNUC__) && defined(__LP64__)
    static inline unsigned char _BitScanForward64(unsigned long* Index, const U64 Mask)
    {
        U64 Ret;
        __asm__
        (
            "bsfq %[Mask], %[Ret]"
            :[Ret] "=r" (Ret)
            :[Mask] "mr" (Mask)
        );
        *Index = (unsigned long)Ret;
        return Mask?1:0;
    }
    
   inline unsigned char _bittest64(long long const *a, std::int64_t b)
   {
      auto const bits{ reinterpret_cast<unsigned char const*>(a) };
      auto const value{ bits[b >> 3] };
      auto const mask{ (unsigned char)(1 << (b & 7)) };
      return (value & mask) != 0;
   }
    #define USING_INTRINSICS
#endif


constexpr static U64 falseMask = 0ULL;
constexpr static U64 trueMask = ~falseMask;
#define U32 uint32_t
constexpr static U32 falseMask32 = 0;
constexpr static U32 trueMask32 = ~0;
constexpr static U64 debruijn64 = 0x07EDD5E59A4E28C2;
constexpr static array<int,64> index64 {
   63,  0, 58,  1, 59, 47, 53,  2,
   60, 39, 48, 27, 54, 33, 42,  3,
   61, 51, 37, 40, 49, 18, 28, 20,
   55, 30, 34, 11, 43, 14, 22,  4,
   62, 57, 46, 52, 38, 26, 32, 41,
   50, 36, 17, 19, 29, 10, 13, 21,
   56, 45, 25, 31, 35, 16,  9, 12,
   44, 24, 15,  8, 23,  7,  6,  5
};
constexpr int bitscan(U64 bitboard) { return (index64[((bitboard & twos_complement(bitboard)) * debruijn64) >> 58]); }

void print_bitboard(const U64 bitboard);
const string square_coordinates[64] = {
"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};
consteval U64 get_ls1b_consteval(const U64 bitboard) {
    return bitboard & (0 - bitboard);
}
consteval U64 pop_ls1b_consteval(const U64 bitboard) {
    return bitboard & (bitboard-1);
}