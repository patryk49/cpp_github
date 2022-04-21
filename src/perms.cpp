#include "SPL/Arrays.hpp"


bool nextCombination(size_t *const first, size_t *const last, size_t n) noexcept{;
	for (size_t *I=last; first != I;)
		if (*--I < --n){
			for (size_t val=*I; I != last; ++I) *I = ++val;
			return false;
		}
	return true;
}

bool prevCombination(size_t *const first, size_t *const last, size_t n) noexcept{;
	for (size_t *I=last; first != I;)
		if (*--I < --n){
			for (size_t val=*I; I != last; ++I) *I = ++val;
			return false;
		}
	return true;
}


int main(int argc, char **argv){
	const size_t k = strtol(argv[1], nullptr, 10);
	
	if (argc != 3){
		fputs("error: wrong number of arguments\n", stderr);
		return 1;
	}

	sp::DArray<size_t> inds;
	inds.resize(k);
	for (size_t i=0; i!=std::size(inds); ++i) inds[i] = i;
	
	const size_t n = strlen(argv[2]);
	if (n < k){
		fputs("error: less elements than places to put them\n", stderr);
		return 1;
	}

	do{
		for (auto I : inds) putchar(argv[2][I]);
		putchar('\n');
	} while (!nextCombination(std::begin(inds), std::end(inds), n));

	return 0;
}
