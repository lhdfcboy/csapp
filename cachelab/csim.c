#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

typedef unsigned int uint;
struct cache_param
{
	int s, S;
	int E;
	int b, B;
	//int m, t; //t=m-s-b;
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
struct cache_sim
{
	char* bdata;//block data, size: S*E*B
	char* valid;//valid bit, size: S*E;
	uint* tag;//tag, size: S*E
};
void usage(char* argv0)
{
	fprintf(stderr, "Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv0);
	return ;
}
int cache_sim_alloc(struct cache_sim* cache, struct cache_param* param)
{

	int n = param->S * param->E * param->B;

	cache->bdata = (char*)calloc(n, sizeof(char));
	cache->valid = (char*)calloc(param->S * param->E, sizeof(char));
	cache->tag = (uint*)calloc(param->S * param->E, sizeof(uint));

	return 0;
}

void cache_sim_free(struct cache_sim* cache)
{
	if (cache->bdata)
		free(cache->bdata);
	if (cache->valid)
		free(cache->valid);
	if (cache->tag)
		free(cache->tag);

	return;
}

void process(char* instruction, 
	uint addr, 
	int len, 
	struct cache_param* param,
	struct cache_sim* cache)
{
	/* addr: [tag][set][block] */
	uint addr_t;
	uint addr_s;
	uint addr_b;

	/*1. splite address */
	
	addr_b =( addr << (32 - param->b)) >> (32 - param->b);
	addr = addr >> (param->b);
	addr_s = (addr << (32 - param->s)) >> (32 - param->s);
	addr = addr >> (param->s);
	addr_t = addr;
	printf("t=0x%x, s=0x%x, b=0x%x\n", addr_t, addr_s, addr_b);

	/*2. We know S and E, check valid for each block(total E) */
	for (int addr_e = 0; addr_e < param->E; addr_e++)
	{
		char *pvalid=&(cache->valid[addr_s * param->E + addr_e]);
		uint  *tag = &(cache->tag[addr_s * param->E + addr_e]);

		if (*pvalid == 1 && *tag == addr_t)
		{
				printf("hit\n");
		}
		else
		{
			printf("miss\n");
			/* fetch */
			*pvalid = 1;
			*tag = addr_t;
		}

	}

	
	

}
void scan_file(FILE *fp, struct cache_param* param)
{
	char instruction[256];
	uint addr;
	int len;
	struct cache_sim cache;

	addr=0;
	len=0;
	
	memset(&cache, 0, sizeof(cache));
	cache_sim_alloc(&cache, param);

	while(!feof(fp))
	{
	/*e.g.
 M 7fefe059c,4
 L 7fefe0594,4
	*/
		memset(instruction, 0, sizeof(instruction));
		fscanf(fp, " %s %x,%d\n", instruction, &addr, &len);
		printf("instruction=%s,addr=0x%x,len=%d\n", instruction, addr, len);

		process(instruction, addr, len, param, &cache);
		if (instruction[0] == 'M')
		{
			/*process again for the same address*/
			process(instruction, addr, len, param, &cache);
		}
	}


	cache_sim_free(&cache);

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
			if (s < 0 || s > 32)
			{
				exit(EXIT_FAILURE);
			}
			param->s = s;
			break;
		case 'E':
			E = atoi(optarg);
			param->E = E;
			printf("E=%d\n", E);
			break;
		case 'b':
			b = atoi(optarg);
			printf("b=%d\n", b);
			if (b < 0 || b > 32)
			{
				exit(EXIT_FAILURE);
			}
			param->b = b;
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
	if ( s+b > 32)
	{
		printf("s+b > 32\n");
		exit(EXIT_FAILURE);
	}
	param->S = (1 << (param->s));
	param->B = (1 << (param->b));
	param->C = param->S * param->E * param->B;
	

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
	scan_file(fp, &param);

	/*3. close trace file */
	fclose(fp);
	
    printSummary(0, 0, 0);
    return 0;
}
