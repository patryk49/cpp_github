#include "SPL/Utils.hpp"




int main(){
	char input[256];
	scanf("%s", input);
	putchar('\n');

	const char c = *input;
	int charNum = c>='a' && c<='z' ? c-'a'+1 : c>='A' && c<='Z' ? c-'A'+1 : 2137;

	printf("czasem sie naucze czasem nie\n%d\n", charNum);


	return 0;
}
