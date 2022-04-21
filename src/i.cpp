#include "SPL/Integral.hpp"

#include <stdio.h>
#include <math.h>

int main(){
	printf("%lf", sp::integrate(sinf, 0.0, M_PI));
	return 0;
}
