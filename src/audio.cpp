#include <stdio.h>
#include <unistd.h>
#include "SPL/Utils.hpp"


int main(){
	system("amixer get Master");
	char buf[256];
	fgets(buf, sp::len(buf), stdin);

	puts("what was read:");
	puts(buf);

	return 0;
}
