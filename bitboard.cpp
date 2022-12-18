#include "bitboard.h"
void print_bitboard(const U64 bitboard) {
	printf("\n");
	for (int rank = 0; rank < 8; rank++) {
		for (int file = 0; file < 8; file++) {
			//loop over board ranks and files

			int square = rank * 8 + file;
			//convert to square index

			if (!file) {
				printf(" %d ", 8 - rank);
			}//print rank on the left side

			printf(" %d", get_bit(bitboard, square) ? 1 : 0);
			//print bitstate (1 or 0)
		}
		printf("\n");
	}
	printf("\n    a b c d e f g h \n");
	printf("    Bitboard:%llud\n", bitboard);
}