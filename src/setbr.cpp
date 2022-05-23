#include <stdio.h>
#include <stdlib.h>

constexpr const char *maxbr_filepath = "/sys/class/backlight/intel_backlight/max_brightness";
constexpr const char *br_filepath = "/sys/class/backlight/intel_backlight/brightness";
constexpr size_t      minbr = 64;


int main(int argc, char **argv){
	if (argc > 2){
		fputs("wrong number of arguments\n", stderr);
		return 1;
	}

	FILE *maxbr_file = fopen(maxbr_filepath, "r");
	if (!maxbr_file){
		fprintf(stderr, "could not open a file: %s\n", maxbr_filepath);
		return 1;
	}

	size_t maxbr;
	{
		bool errFlag = fscanf(maxbr_file, "%lu", &maxbr) != 1;
		//fclose(maxbr_file);
		if (errFlag){
			fprintf(stderr, "wrong format of the data in file: %s\n", maxbr_filepath);
			return 1;
		}
	}
	if (maxbr < minbr){
		fprintf(
			stderr,
			"maximum brightness (%lu) is less than minimum brightness (%lu) from file: %s\n",
			maxbr, minbr, maxbr_filepath
		);
		return 1;
	}


	bool is_relative = false;
	if (argc == 2) is_relative = argv[1][0]=='+' || argv[1][0]=='-';

	FILE *br_file = fopen(br_filepath, "r+");
	if (!br_file){
		fprintf(stderr, "could not open a file: %s\n", br_filepath);
		return 1;
	}

	size_t prevbr;
	if (fscanf(br_file, "%lu", &prevbr) != 1){
		fprintf(stderr, "wrong format of the data in file: %s\n", maxbr_filepath);
		fclose(br_file);
		return 1;
	}
	
	size_t br;
	if (argc == 2){
		char *flag_ptr;
		br = strtol(argv[1]+is_relative, &flag_ptr, 10);
		if (*flag_ptr != '\0'){
			fputs("wrong format of the input data\n", stderr);
			return 1;
		}
		if (is_relative){
			if (argv[1][0] == '+'){
				br = prevbr + br;
				if (br < prevbr) br = maxbr;
			} else{
				br = prevbr - br;
				if (br > prevbr) br = minbr;
			}
			if (br == prevbr) return 0;
		}
	} else{
		br = prevbr==maxbr ? minbr : maxbr;
	}
		
	fprintf(br_file, "%lu", (unsigned long)(minbr<br ? br<maxbr ? br : maxbr : minbr));
	//fclose(br_file);

	return 0;
}
