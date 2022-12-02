#include <stdlib.h>
#include "SPL/Polynomial.hpp"
#include <array>
#include <stdio.h>


template<class P>
void print_polynomial(const P &p) noexcept{
	printf("%8.2lf", (double)p[0]);
	for (size_t i=1; i!=len(p); ++i)
		printf(" %+8.2lfx^%u", (double)p[i], (unsigned)i);
	putchar('\n');
}

template<class P>
void print_array(const P &p) noexcept{
	for (size_t i=0; i!=len(p); ++i)
		printf(" %8.2lf", (double)p[i]);
	putchar('\n');
}


int main(){
	Polynomial<FixedArray<double, 2>> p1{{-1.0, 1.0}};
	Polynomial<FixedArray<double, 2>> p2{{ 0.4, 1.0}};
	Polynomial<FixedArray<double, 2>> p3{{ 1.0, 1.0}};
	
	Polynomial<> G_p;
	G_p = p1 * p2 * p3;

	DynamicArray<double> res;	

	find_roots(res, G_p * p1 - p2/2.0, 0.0001);
//	print_polynomial(G_p);
	
//	divide(res, G_p, p3);
	
	print_polynomial(G_p);
	print_polynomial(G_p * p1 - p2/2);
	print_array(res);


//	puts("roots:");
//	for (size_t i=0; i!=len(res); ++i) printf("  %lf\n", res[i]/*.real, res[i].imag*/);
//	putchar('\n');

//	for (double x=0.f; x<=10.f; x+=0.1f){
//		printf("G_p(%6.2lf) = %6.2lf\n", x, G_p(x));
//	}

	return 0;
}
