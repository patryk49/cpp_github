#include "SPL/Hashmaps.hpp"
#include <stdio.h>

int main(){
	sp::FiniteHashmap<int, double, 32> hm;
	sp::set(hm, 10, 10.0);
	sp::set(hm, 12, 12.0);
	sp::set(hm, 11, 11.0);
	sp::set(hm, 42, 42.0);
	sp::set(hm, 100, 100.0);
	
	double *d = sp::get(hm, 42);

	printf("value: %lf\n", hm[100]);

	return 0;
}
