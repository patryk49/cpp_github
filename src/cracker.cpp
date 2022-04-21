#include "SPL/Utils.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "SPL/Arrays.hpp"
#include "SPL/Allocators.hpp"

#include <unistd.h>

int main(int argc, char **argv){
	sp::Range<const char> filepath{nullptr, 0};

	size_t min_len = 3;
	size_t max_len = 6;

	char from_char =  'a';
	char to_char = 'z';

	for (size_t i=1; i!=argc; ++i){
		if (argv[i][0] == '-'){
			const char *it = argv[i] + 1;
			switch (*it){
				case 'L':
					break;
				case 'h':
					puts("");
					return 0;
				default:
					fputs("wrong flag\n", stderr);
					return 1;
			}
		} else{
			if (filepath.ptr != nullptr){
				fputs("filepath was specified too many times\n", stderr);
				return 1;
			}
			filepath.ptr = argv[i];
			filepath.size = strlen(argv[i]);
		}
	}

	sp::DynamicArray<char, sp::MallocAllocator<>> text;
	sp::push_range(text, sp::range("mupdf -p"));
	sp::expand_back(text, max_len+1);
	for (char *I=sp::end(text)-max_len-1; I!=sp::end(text); ++I) *I = ' ';

	sp::push_range(text, filepath);
	sp::Range<char> pass = {sp::end(text)-max_len-1-sp::len(filepath), min_len};


	for (; sp::len(pass)!=max_len; ++pass.size){
		for (char *I=sp::beg(pass); I!=sp::end(pass); ++I) *I = from_char;

		char *end_pos = sp::end(pass) - 1;
		for (;;){
			if (!system(sp::beg(text))){
				pass[pass.size] = '\0';
				printf("password: %s\n", pass.ptr);
				return 0;
			}
		
			char *J = end_pos;
			while (*J == to_char){
				if (J == sp::beg(pass)) goto Continue;
				*J = from_char;
				--J;
			}
			++*J;
		}
	Continue:;
	}

	fputs("cracking was unsuccessful\n", stderr);
	return 1;
}
