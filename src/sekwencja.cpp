#include <stdio.h>
#include <stdint.h>

int main(){
	char buff[8];
TypeInSequence:
	fputs("Type in the sequence: ", stdout);
	if (!fgets(buff, sizeof(buff), stdin)){
		putchar('\n');
		return 0;
	}
	if (
		(buff[0]!='0' && buff[0]!='1') || (buff[1]!='0' && buff[1]!='1')
		|| (buff[2]!='0' && buff[2]!='1') || buff[3]!='\n'
	){
		puts("wrong sequence format");
		goto TypeInSequence;
	}
	bool s0 = buff[0] - '0';
	bool s1 = buff[1] - '0';
	bool s2 = buff[2] - '0';


	int8_t transition_table[4][2] = {
	//                                                       0                                                   1
		{                                                   !s0,                                                 s0},
		{                                         !s1 ? 2 : !s0,                                        s1 ? 2 : s0},
		{                     !s2 ? 3 : s1==s0 && !s1 ? 2 : !s0,                     s2 ? 3 : s1==s0 && s1 ? 2 : s0},
		{ s2==s1 && s1==s0 && !s2 ? 3 : s2==s0 && !s1 ? 2 : !s0, s2==s1 && s1==s0 && s2 ? 3 : s2==s0 && s1 ? 2 : s0},
	};

	int8_t input = !s0;
	int8_t state = 0;

	for (;;){
		printf("state:  %i\noutput: %i\n\n", (int)state, (int)(state == 3));

	TypeInInput:
		fputs("Type in the input: ", stdout);
		if (!fgets(buff, sizeof(buff), stdin)){
			putchar('\n');
			return 0;
		}
		
		if ((buff[0]!='0' && buff[0]!='1') || buff[1]!='\n'){
			puts("wrong input format");
			goto TypeInInput;
		}

		input = buff[0] - '0';
		int8_t new_state = transition_table[state][input];
		
		if (new_state == -1){
			puts("illegal input");
			goto TypeInInput;
		}
		state = new_state;
	}
}
