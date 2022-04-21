#include <stdio.h>
#include <stdint.h>

int main(){
	int8_t transition_table[4][4] = {
	//  00  01  10  11
		{ 0, -1,  1, -1},
		{ 1,  2,  1,  2},
		{ 2,  2,  3,  2},
		{ 0, -1,  3, -1},
	};

	int8_t output_table[4][4] = {
	//  00  01  10  11
		{ 0, -1,  1, -1},
		{ 1,  1,  1,  1},
		{ 0,  1,  0,  1},
		{ 0, -1,  0, -1},
	};
	
	int8_t input = 0;
	int8_t state = 0;

	for (;;){
		printf("state:  %i\noutput: %i\n\n", (int)state, (int)output_table[state][input]);

	TypeInInput:
		fputs("Type in the input: ", stdout);
		char buff[8];
		if (!fgets(buff, sizeof(buff), stdin)){
			putchar('\n');
			return 0;
		}
		
		if (
			(buff[0]!='0' && buff[0]!='1') || (buff[1]!='0' && buff[1]!='1')
			|| (buff[2]!='0' && buff[2]!='1') || buff[3]!='\n'
		){
			puts("wrong input format");
			goto TypeInInput;
		}

		input = (((buff[0]-'0')|(buff[2]-'0')) << 1) | (buff[1]-'0');
		int8_t new_state = transition_table[state][input];
		
		if (new_state == -1){
			puts("illegal input");
			goto TypeInInput;
		}
		state = new_state;
	}
}
