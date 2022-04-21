#include <stdio.h>
#include <stdlib.h>

constexpr const char *maxbrFilepath = "/sys/class/backlight/intel_backlight/max_brightness";
constexpr const char *brFilepath = "/sys/class/backlight/intel_backlight/brightness";
constexpr size_t      minbr = 64;


int main(int argc, char **argv){
	if (argc > 2){
		fputs("wrong number of arguments\n", stderr);
		return 1;
	}

	FILE *maxbrFile = fopen(maxbrFilepath, "r");
	if (!maxbrFile){
		fprintf(stderr, "could not open a file: %s\n", maxbrFilepath);
		return 1;
	}

	size_t maxbr;
	{
		bool errFlag = fscanf(maxbrFile, "%lu", &maxbr) != 1;
		fclose(maxbrFile);
		if (errFlag){
			fprintf(stderr, "wrong format of the data in file: %s\n", maxbrFilepath);
			return 1;
		}
	}
	if (maxbr < minbr){
		fprintf(
			stderr,
			"maximum brightness (%lu) is less than minimum brightness (%lu) from file: %s\n",
			maxbr, minbr, maxbrFilepath
		);
		return 1;
	}


	bool isRelative = false;
	if (argc == 2) isRelative = argv[1][0]=='+' || argv[1][0]=='-';

	FILE *brFile = fopen(brFilepath, isRelative ? "r+" : "w");
	if (!brFile){
		fprintf(stderr, "could not open a file: %s\n", brFilepath);
		return 1;
	}

	size_t br;
	if (argc == 2){
		char *flagPtr;
		br = strtol(argv[1]+isRelative, &flagPtr, 10);
		if (*flagPtr != '\0'){
			fputs("wrong format of the input data\n", stderr);
			return 1;
		}
		if (isRelative){
			size_t prevbr;
			if (fscanf(brFile, "%lu", &prevbr) != 1){
				fprintf(stderr, "wrong format of the data in file: %s\n", maxbrFilepath);
				fclose(brFile);
				return 1;
			}

			if (argv[1][0] == '+'){
				br = prevbr + br;
				if (br < prevbr) br = maxbr;
			} else{
				br = prevbr - br;
				if (br > prevbr) br = minbr;
			}
		}
	} else{
		br = maxbr;
	}
		
	fprintf(brFile, "%lu", minbr<br ? br<maxbr ? br : maxbr : minbr);
	fclose(brFile);

	return 0;
}
