#include <stdio.h>
#include <stdlib.h>

#include "SPL/Utils.hpp"



int main(int argc, char **argv){
	if (argc != 2) return 1;
	
	uint8_t x = strtol(argv[1], nullptr, 10);

	constexpr static decltype(x) bits = sizeof(x) * 8;


	decltype(x) res = x ^ (x >> 1);

//	for (size_t i=sizeof(x)*8; --i!=SIZE_MAX;) putchar('0' + ((x>>i) & 1));
//	putchar('\n');
	
	for (size_t i=sizeof(res)*8; --i!=SIZE_MAX;) putchar('0' + ((res>>i) & 1));
	putchar('\n');
	
	return 0;
}
