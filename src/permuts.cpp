#include <algorithm>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


void printPermutations(const char *first, const char *last);

bool printVariations(const char *first, const char *last, size_t k);

bool printCombinations(const char *first, const char *last, size_t k);



int main (int argc, char *argv[]) {
	if (argc < 3){
		puts(
			"You are callng this function in a wrong way.\nFirst argument must be a letter:\nP -"
			" permutations\nV - variations\nC - combinations\nSecond argument is an input string."
		);
		return 1;
	}

	char *const inpBegin = argv[2];
	char *const inpEnd = inpBegin + strlen(inpBegin);

	switch (*argv[1]){
	case 'P':
		if (argc != 3){
			puts("too many argumenst");
			return 1;
		}
		printPermutations(inpBegin, inpEnd);
		break;
	case 'V':
		break;
	case 'C':
		if (argc != 4){
			puts("wrong number of argumenst");
			return 1;
		}
		printCombinations(inpBegin, inpEnd, (size_t)strtol(argv[3], nullptr, 10));
		break;
	default:
		puts("wrong first argument");
		return 1;
	}
	
	return 0;
}


void printPermutationsRec(char *const first, char *const last, char *const begin){
	if (begin+1 == last){
		puts(first);
		return;
	}
	
	char *iter = begin;
	while (++iter != last){
		printPermutationsRec(first, last, begin+1);
		std::iter_swap(begin, iter);
	}
	printPermutationsRec(first, last, begin+1);
	
	char temp = std::move(*begin);
	std::move(begin+1, last, begin);
	*(last-1) = std::move(temp);
}
void printPermutations(const char *const first, const char *const last){
	std::string buffer(first, last);
	printPermutationsRec(&*std::begin(buffer), &*std::end(buffer), &*std::begin(buffer));
}



void printCombinationsRec(
	const char *first, const char *const last, const char *const partition,
	char *const result, char *const begin
){
	while (first != partition){
		*begin = *first;
		++first;
		if (partition == last) puts(result);
		else printCombinationsRec(first, last, partition+1, result, begin+1);
	}
}
bool printCombinations(const char *const first, const char *const last, const size_t k){
	if ((ssize_t)k > (last-first) || !k) return false;
	std::string buffer; buffer.resize(k);
	printCombinationsRec(first, last, last-k+1, &*std::begin(buffer), &*std::begin(buffer));
	return true;
}





void printVariationsRec(const char *first, const char *const last, const char *const partition, char *const result, char *const begin){
}
bool printVariations(const char *const first, const char *const last, const size_t k){
	if ((ssize_t)k > (last-first) || !k) return false;
	std::string buffer;
	buffer.resize(k);
	printVariationsRec(first, last, last-k, &*std::begin(buffer), &*std::begin(buffer));
	return true;
}

