#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void usage(char* argv0)
{
	fprintf(stderr, "Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv0);
	return ;
}
int main(int argc, char** argv)
{
	int  opt;
	int vflag;
	int s,E,b;
	char filename[256];
	vflag=0;
	s=0;
	E=0;
	b=0;
	memset(filename, 0, sizeof(filename));
	while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
		switch (opt) {
		case 'h':	
			usage(argv[0]);
			exit(EXIT_FAILURE);
		case 'v':
			vflag=1;
			printf("vflag=%d\n", vflag);
			break;
		case 's':
			s = atoi(optarg);
			printf("s=%d\n", s);
			break;
		case 'E':
			E = atoi(optarg);
			printf("E=%d\n", E);
			break;
		case 'b':
			b = atoi(optarg);
			printf("b=%d\n", b);
			break;
		case 't':
			strcpy(filename, optarg);
			printf("t=%s\n", filename);
			break;
		default: /* '?' */
			usage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}
/*
	if (optind >= argc) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	printf("name argument = %s\n", argv[optind]);
*/

	
    printSummary(0, 0, 0);
    return 0;
}
