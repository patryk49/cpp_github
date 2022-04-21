#include "SPL/Utils.hpp"
#include <stdio.h>
#include <time.h>


template<class T, class R>
SP_CSI void shuffle(sp::Range<T> range, R rng) noexcept{
	if (range.size <= 1) return;
	
	for (size_t i=range.size; i!=1;){
		T *rand = range.ptr + rng() % i;
		--i;
		sp::iswap(range.ptr + i, rand);
	}
}


int main(int argc, char **argv){
	sp::Rand32 rng{clock()};
	
	size_t count = argc==2 ? strtol(argv[1], nullptr, 10) : 10000;
	
	//uint32_t arr[500];
	for (size_t i=0; i!=count; ++i) printf("%c", rng()&1 ? 'O' : ' ');
	putchar('\n');

	return 0;
}
