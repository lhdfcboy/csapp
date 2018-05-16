#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
struct cache_param
{
	int s, S;
	int E;
	int b, B;
	int m, t; //t=m-s-b;
	int C;
} ;
struct row
{
	int valid;
	int tag;
	char* block;
};
struct set
{
	struct row *rows;
};
struct cache
{
	struct set *sets;
};
void usage(char* argv0)
{
	fprintf(stderr, "Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv0);
	return ;
}

void process(FILE *fp)
{
	char instruction[256];
	unsigned int addr;
	int len;
	

	addr=0;
	len=0;
	
	/*e.g.
 M 7fefe059c,4
 L 7fefe0594,4
 L 7fefe059c,4
 M 7fefe0598,4
	*/
	while(!feof(fp))
	{
		memset(instruction, 0, sizeof(instruction));
		fscanf(fp, " %s %x,%d\n", instruction, &addr, &len);
		printf("instruction=%s,addr=0x%x,len=%d\n", instruction, addr, len);


	}



	return;
}

char* data_alloc(struct cache_param* param)
{
	char *data;
	int n = param->S * param->E * param->B;
	data = (char*)calloc(n, sizeof(char));

	return data;
}

void cache_free(struct row* rows)
{
#if 0
	while (*rows)
	{
		//struct row* row = rows;
		if (rows->block)
			free(rows->block);
		free(rows);
		
		rows++;
	}
#endif
	return;
}
int parse_cmd(int argc, char** argv, struct cache_param* param, char* filename)
{
	int  opt;
	int vflag;
	int s, E, b;

	vflag = 0;
	s = 0;
	E = 0;
	b = 0;

	while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
			exit(EXIT_FAILURE);
		case 'v':
			vflag = 1;
			printf("vflag=%d\n", vflag);
			break;
		case 's':
			s = atoi(optarg);
			printf("s=%d\n", s);
			param->s = s;
			break;
		case 'E':
			E = atoi(optarg);
			param->E = E;
			printf("E=%d\n", E);
			break;
		case 'b':
			b = atoi(optarg);
			param->b = b;
			printf("b=%d\n", b);
			break;
		case 't':
			strcpy(filename, optarg);
			printf("t=%s\n",filename);
			break;
		default: /* '?' */
			usage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	param->S = (1 << (param->s));
	param->B = (1 << (param->b));
	param->C = param->S * param->E * param->B;
	//param->m=

	printf("S=%d, B=%d, C=%d\n", param->S, param->B, param->C);

	return 0;
}
int main(int argc, char** argv)
{
	char filename[256];
	struct  cache_param param;
	FILE *fp;

		
	fp = NULL;
	memset(filename, 0, sizeof(filename));
	memset(&param, 0, sizeof(param));

	/*0. parse command line parameters */
	parse_cmd(argc, argv, &param, filename);
	


	/*1. open trace file */
	fp=fopen(filename, "r");
	if(!fp)
	{
		printf("can not open file %s\n", filename);
		exit(EXIT_FAILURE);
	}
	/*2. process trace file */
	process(fp);
	/*3. close trace file */
	fclose(fp);
	
    printSummary(0, 0, 0);
    return 0;
}
