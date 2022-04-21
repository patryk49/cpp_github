#include <stdio.h>
#include <stdint.h>

int main(){
	char buff[8] = "10";

	for (;;){
		printf("pompa:  %i\nzawor: %i\n\n", (int)(buff[1]=='0'), (int)(buff[0]=='1'));

	TypeInInput:
		fputs("Type in the input: ", stdout);
		if (!fgets(buff, sizeof(buff), stdin)){
			putchar('\n');
			return 0;
		}
		
		if ((buff[0]!='0' && buff[0]!='1') || (buff[1]!='0' && buff[1]!='1') || buff[2]!='\n'){
			puts("wrong input format");
			goto TypeInInput;
		}

		if (buff[0]=='0' && buff[1]=='1'){
			puts("illegal input");
			goto TypeInInput;
		}
	}
}
