#include <stdlib.h>
#include "SPL/Polynomial.hpp"
#include <stdio.h>

int main(){
	sp::Polynomial<> pol0;
	sp::Polynomial<> pol1;
	sp::Polynomial<> pol2;
	
	sp::resize(pol0, 3);
	pol0[2] =  1.0;
	pol0[1] =  -1.0;
	pol0[0] =  -1.0;

	sp::resize(pol1, 3);
	pol1[2] = -1.0;
	pol1[1] = -8.0;
	pol1[0] =  1.0;

	sp::resize(pol2, 3);
	pol2[2] = -1.0;
	pol2[1] = -8.0;
	pol2[0] =  1.0;


	sp::DynamicArray<double, sp::MallocAllocator<>> res;

	sp::find_roots(res, pol0, 0.001);

	puts("roots:");
	for (size_t i=0; i!=sp::len(res); ++i) printf("  %lf\n", res[i]);
	putchar('\n');

	return 0;
}
