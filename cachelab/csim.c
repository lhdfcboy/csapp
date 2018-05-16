#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

typedef unsigned int uint;
typedef unsigned long  ulong;

/*
 * some params are read from command line, 
 * others are calculated by parse_cmd()
 */
struct cache_param
{
	int s, S;
	int E;
	int b, B;
	//int m, t; //t=m-s-b, not used 
	int C;
} ;

/*
* valid tag count for each row(block)
*/
struct cache_sim
{
	//char* bdata;//block data, size: S*E*B, not used
	char* valid;//valid bit, size: S*E;
	uint* tag;//tag, size: S*E
	uint* count;//reference count, size: S*E
};
/*
* count result for misses hits evictions, separately
*/
struct result
{
	int misses;
	int hits;
	int evictions;
};

struct context
{
	struct cache_param param;
	struct result res;
	int vflag;
	char filename[256];
};
void usage(char* argv0)
{
	fprintf(stderr, "Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv0);
	return ;
}

void cache_sim_free(struct cache_sim* cache)
{

	if (cache->valid)
		free(cache->valid);
	if (cache->tag)
		free(cache->tag);
	if (cache->count)
		free(cache->count);

	return;
}
int cache_sim_alloc(struct cache_sim* cache, struct cache_param* param)
{

	if(NULL==(cache->valid = (char*)calloc(param->S * param->E, sizeof(char))))
		goto exit1;
	if(NULL==(cache->tag = (uint*)calloc(param->S * param->E, sizeof(uint))))
		goto exit1;
	if(NULL==(cache->count = (uint*)calloc(param->S * param->E, sizeof(uint))))
		goto exit1;

	return 0;
exit1:
	printf("malloc failed\n");
	cache_sim_free(cache);
	return -1;
}


void process(char* instruction,
	uint addr,
	int len,
	struct cache_sim* cache,
	struct context *ctx
)
{
	/* addr: [tag][set][block], addr_b is not used */
	uint addr_t;
	uint addr_s;
	struct cache_param* param = &ctx->param;
	struct result *res = &ctx->res;

	/*1. splite address */
	addr = addr >> (param->b);
	addr_s = (addr << (32 - param->s)) >> (32 - param->s);
	addr = addr >> (param->s);
	addr_t = addr;

	/*2. We know S and E, check valid for each block(total E) */

	/*2.1 update reference count for all rows */
	for (int addr_e = 0; addr_e < param->E; addr_e++)
	{
		uint  *count = &(cache->count[addr_s * param->E + addr_e]);
		(*count)++;
	}

	/*2.2 hit */
	for (int addr_e = 0; addr_e < param->E; addr_e++)
	{
		char *pvalid=&(cache->valid[addr_s * param->E + addr_e]);
		uint  *tag = &(cache->tag[addr_s * param->E + addr_e]);
		uint  *count = &(cache->count[addr_s * param->E + addr_e]);

		(*count)++;
		if (*pvalid == 1 && *tag == addr_t)
		{
				(res->hits)++;
				*count = 0;
				if(ctx->vflag) printf("hit ");
				return;
		}
	}
	

	/*2.3 miss */
	if (ctx->vflag) printf("miss ");
	(res->misses)++;
	for (int addr_e = 0; addr_e < param->E; addr_e++)
	{
		char *pvalid = &(cache->valid[addr_s * param->E + addr_e]);
		uint  *tag = &(cache->tag[addr_s * param->E + addr_e]);
		uint  *count = &(cache->count[addr_s * param->E + addr_e]);
		if (*pvalid == 0)
		{
			*count = 0;
			*tag = addr_t;
			*pvalid = 1;
			return;
		}
	}

	/*2.4 eviciton, find LRU( least recently used ) row*/
	if (ctx->vflag) printf("eviction ");
	(res->evictions)++;
	int addr_e_lru = 0;
	int count_max= cache->count[addr_s * param->E + 0];
	for (int addr_e = 0; addr_e < param->E; addr_e++)
	{
		uint  *count = &(cache->count[addr_s * param->E + addr_e]);
		if (*count > count_max)
		{
			count_max = *count;
			addr_e_lru = addr_e;
		}
	}
	cache->valid[addr_s * param->E + addr_e_lru] = 1;
	cache->tag[addr_s * param->E + addr_e_lru] = addr_t;
	cache->count[addr_s * param->E + addr_e_lru] = 0;

}
void scan_file(FILE *fp, struct context *ctx)
{
	char instruction[256];
	uint addr;
	int len;
	struct cache_sim cache;
	struct cache_param* param = &ctx->param;
	
	addr=0;
	len=0;
	
	memset(&cache, 0, sizeof(cache));
	if (cache_sim_alloc(&cache, param) != 0)
		return;

	while(!feof(fp))
	{
		/* e.g.
		 M 7fefe059c,4
		 L 7fefe0594,4
		I 7fefe0594,4
		 S 7fefe0594,4
		*/
		memset(instruction, 0, sizeof(instruction));
		fscanf(fp, " %s %x,%d\n", instruction, &addr, &len);
		if (instruction[0] == 'I')
		{
			continue;
		}

		if (ctx->vflag) printf(" %s %8x,%1d ", instruction, addr, len);
		process(instruction, addr, len, &cache, ctx);
		if (instruction[0] == 'M')
		{
			/*process again for the same address*/
			process(instruction, addr, len, &cache, ctx);
		}
		if (ctx->vflag) printf("\n");

	}


	cache_sim_free(&cache);

	return;
}


int parse_cmd(int argc, char** argv, struct context* ctx)
{
	int  opt;
	int s, E, b;


	s = 0;
	E = 0;
	b = 0;
	struct cache_param *param = &(ctx->param);
	while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
			exit(EXIT_FAILURE);
		case 'v':
			ctx->vflag = 1;
			break;
		case 's':
			s = atoi(optarg);
			if (s < 0 || s > 32)
			{
				exit(EXIT_FAILURE);
			}
			param->s = s;
			break;
		case 'E':
			E = atoi(optarg);
			param->E = E;
			break;
		case 'b':
			b = atoi(optarg);
			if (b < 0 || b > 32)
			{
				exit(EXIT_FAILURE);
			}
			param->b = b;
			break;
		case 't':
			strcpy(ctx->filename, optarg);
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
	

	if (ctx->vflag) printf("S=%d, B=%d, C=%d\n", param->S, param->B, param->C);

	return 0;
}
int main(int argc, char** argv)
{
	FILE *fp;
	struct context ctx;
	fp = NULL;
	
	/*0. parse command line parameters */
	memset(&ctx, 0, sizeof(ctx));
	parse_cmd(argc, argv, &ctx);
	

	/*1. open trace file */
	fp=fopen(ctx.filename, "r");
	if(!fp)
	{
		printf("can not open file %s\n", ctx.filename);
		exit(EXIT_FAILURE);
	}

	/*2. scan_file trace file */
	scan_file(fp, &ctx);

	/*3. close trace file */
	fclose(fp);
	
    printSummary(ctx.res.hits, ctx.res.misses, ctx.res.evictions);
    return 0;
}
