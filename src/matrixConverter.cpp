#include "SPL/Utils.hpp"
#include <stdio.h>

int main(){
	size_t maxLineLength = 0x00000f00;
	char *matrix = (char *)malloc(0x00008000);
	*matrix = '[';
	char *charPtr = matrix+1;

	ssize_t lineSize = 0;
	for (;;){
		lineSize = getline(&charPtr, &maxLineLength, stdin);
		if (lineSize <= 1) break;

		charPtr += lineSize;
		*(charPtr-1) = ';';
	}
	*(charPtr-1) = ']';
	*charPtr = 0;
	
	
	puts(matrix);
	return 0;
}
