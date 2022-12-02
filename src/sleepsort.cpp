#include <stdio.h>
#include <thread>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

void printLater(const uint32_t x){
	usleep(x*1024);
	printf("%u ", x);
}

void sleepSort(uint32_t *first, const uint32_t *const last){
	std::vector<std::thread> threads;
	for (; first!=last; ++first)
		threads.emplace_back(printLater, *first);

	for (auto &I : threads) I.join();
}

constexpr bool isNumber(const char c){
	return c>='0' && c<='9';
}


int main(){
	std::vector<uint32_t> input;
	input.reserve(32);

	char str[1 << 16];
	fgets(str, sizeof(str)/sizeof(str[0]), stdin);

	for (const char *I=str; *I; ++I){
		if (isNumber(*I)){
			input.push_back(strtol(I, (char **)&I, 10));		
		} else if (*I==' ' || *I=='\t'){
			do ++I; while(*I==' ' || *I=='\t');
		} else{
			puts("wrong character");
			return 1;
		}
	}

	sleepSort(&*std::begin(input), &*std::end(input));
	
	putchar('\n');


	return 0;
}
