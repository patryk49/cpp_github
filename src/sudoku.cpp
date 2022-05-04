#include "SPL/Utils.hpp"

#include <stdlib.h>
#include <stdio.h>
#include "SPL/Allocators.hpp"
#include "SPL/Arrays.hpp"


constexpr uint16_t FullMask = 0x1ff;

int main(int argc, char **argv){
	if (argc > 2){
		perror("wrong number of arguments");
		return -1;
	}

	bool print_flag = false;
	if (argc == 2){
		if (argv[1][0]!='p' && argv[1][1]!='\0'){
			perror("wrong argument");
			return -1;
		}
		print_flag = true;
	}

	uint16_t board[9][9];
	for (size_t i=0; i!=9; ++i)
		for (size_t j=0; j!=9; ++j)
			board[i][j] = FullMask;

	for (size_t i=0; i!=9; ++i){
		char c;
		for (size_t j=0; j!=9; ++j){
		Repeat:
			c = getchar();
			if (c != '_'){
				if ('1' > c || c > '9') goto Repeat;
				board[i][j] = 1 << (c - '1');
			}
			getchar();
		}
	}

	// solving
	Loop:{
		// print
		if (print_flag){
			putchar('\n');
			for (size_t i=0; i!=9; ++i){
				for (size_t j=0; j!=9; ++j){
					if (j != 0) putchar('|');
					if (std::has_single_bit(board[i][j]))
						printf("%1u", (unsigned)std::bit_width(board[i][j]));
					else
						printf("_");
				}
				putchar('\n');
			}
			if (getchar() != '\n') return 0;
		}

		// handle rows
		for (size_t i=0; i!=9; ++i){
			uint16_t occupied = 0;
			for (size_t j=0; j!=9; ++j)
				occupied |= -std::has_single_bit(board[i][j]) & board[i][j];
			for (size_t j=0; j!=9; ++j)
				board[i][j] &= -std::has_single_bit(board[i][j]) | ~occupied;
		}
		// handle columns
		for (size_t i=0; i!=9; ++i){
			uint16_t occupied = 0;
			for (size_t j=0; j!=9; ++j)
				occupied |= -std::has_single_bit(board[j][i]) & board[j][i];
			for (size_t j=0; j!=9; ++j)
				board[j][i] &= -std::has_single_bit(board[j][i]) | ~occupied;
		}
		// handle 3x3 squares
		for (size_t ii=0; ii!=9; ii+=3)
			for (size_t jj=0; jj!=9; jj+=3){
				uint16_t occupied = 0;
				for (size_t i=0; i!=3; ++i)
					for (size_t j=0; j!=3; ++j)
						occupied |= -std::has_single_bit(board[ii+i][jj+j]) & board[ii+i][jj+j];
				for (size_t i=0; i!=3; ++i)
					for (size_t j=0; j!=3; ++j)
						board[ii+i][jj+j] &= -std::has_single_bit(board[ii+i][jj+j]) | ~occupied;
			}


		for (size_t i=0; i!=9; ++i)
			for (size_t j=0; j!=9; ++j)
				if (!std::has_single_bit(board[i][j])) goto Loop;
	}

	putchar('\n');
	for (size_t i=0; i!=9; ++i){
		for (size_t j=0; j!=9; ++j){
			if (j != 0) putchar('|');
			if (std::has_single_bit(board[i][j]))
				printf("%1u", (unsigned)std::bit_width(board[i][j]));
			else
				printf("_");
		}
		putchar('\n');
	}

	return 0;
}
