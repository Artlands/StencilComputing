/*
 *
 * _PIMS_DRIVER_C
 *
 * MEMORY TRACE POWER CONSUMPATION, BANK CONFLICT
 *
 * Initialize a HMC instantiation
 * Read memory trace file to the device
 * and extracts output
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <time.h>
#include "hmc_sim.h"

#define BANK_MASK = 0x000000000001FF00
#define BANK_SHIFT = 8;

/* -------------------------------------------------- PRINT_HELP */
static void print_help(){
	printf( "------------------------------------------------------\n" );
	printf( "   MEMORY TRACE BANK CONFLICT    \n" );
	printf( "------------------------------------------------------\n" );
	printf( " Usage: run_hmcsim <options>\n" );
	printf( " <options>\n" );
	printf( " -h                       : print this help\n" );
	printf( " -f /path/to/filename     : read trace from file\n" );
	printf( "------------------------------------------------------\n" );
}

/* ----------------------------------------------GET_HMCMEMOP */
/* FOR DMC POWER*/
struct hmcmtrace {
  uint32_t proc;	/* proc id */
  int type;       /* read or write */
  hmc_rqst_t op;    /* memory operation type */
  uint64_t addr;  /* base address of the request */
};

/* ----------------------------------------read_hmc_trace_FILE */
/* only works when trace file is this type: RD/WR:INT:ADDR */
static int read_hmc_trace( FILE *infile, struct hmcmtrace *otrace ){
	/* vars */
	char buf[50];
	char *token;
	size_t len	= 0;
	int type	= -1;	/* {0=WR,1=RD,2=EX}*/
	int nbytes	= 0;
	uint32_t proc	= 0;
	uint64_t addr	= 0x00ull;
	hmc_rqst_t op;
	/* ---- */

	/* read a single entry from the trace file */
	if( feof( infile ) ){		// test the end-of-file indicator for the given stream
		/* fill out the data structure */
		otrace->proc 	= 0;
		otrace->type = 2;
		otrace->op	= FLOW_NULL;
		otrace->addr	= 0x00ull;
		return 0;
	}

	if( fgets( buf, 50, infile ) == NULL ){		//read a line from the stream
		return -1;
	}

	/*
	 * we have a valid buffer
	 * strip the newline and tokenize it
	 *
	 */
	len = strlen( buf );
	if( buf[len] == '\n' ){
		buf[len] = '\0';
	}

	if( (len < 23 ) || (len >29) ){
		/* return a special error code indicating that this is a bogus line*/
		return -1;
	}
	/*
	 * now to tokenize it
	 * Format: {HOST_WR, HOST_RD, PIMS_RD}:{NUM_BYTES}:{0xADDR}
	 */
	token = strtok( buf, ":");	//break string into a series of tokens using ":"
	if( strcmp( token, "HOST_WR" ) == 0 ){
		type = 0;
	}else if( (strcmp( token, "HOST_RD" ) == 0) || (strcmp( token, "PIMS_RD") == 0) ){
		type = 1;
  }else{
		// printf( "error : erroneous message type : %s\n", token );
		return -1;
	}

	/* num_bytes */
	token = strtok( NULL, ":" );
	nbytes = atoi( token );

	/* -- first part of address = 0x */
	token = strtok( NULL, "x" );
	/* -- last part of address in hex */
	addr = (uint64_t)(strtol( token, NULL, 16 ));

	/* get the appropriate memory operand */
	if( get_hmcmemop( type, nbytes, &op) != 0 ){
		// printf( "error : erroneous message type from get_hmcmemop\n" );
		return -1;
	}

	/* fill out the data structure */
	otrace->proc 	= proc;
	otrace->type = type;
	otrace->op	= op;
	otrace->addr	= addr;

#ifdef DEBUG
	printf( "::DEBUG::      proc = %d\n", otrace->proc );
	printf( "::DEBUG::      type = %d\n", otrace->type );
	printf( "::DEBUG::      op   = %d\n", (int)(otrace->op) );
	printf( "::DEBUG::      addr = 0x%016lx\n", otrace->addr );
#endif

  return 0;

}

static void zero_bank( int *bank ) {
	int i;
	for( i = 0; i < 512; i++ ) {
		bank[i] = 0;
	}
	return;
}

static int get_bankid( uint64_t address ) {
	int id;
	id = (int)((address & BANK_MASK) >> BANK_SHIFT);
	return id;
}


/* ----------------------------------------------- MAIN */
int main( int argc, char **argv ) {

	/* vars */
	int i;

	FILE *infile = NULL;
	FILE *outfile = NULL;		//save hmcsim power evaluation to an output file
	char filename[1024];
  char outfname[1024];

	struct hmcmtrace *rqst =  malloc(sizeof(struct hmcmtrace));
	long trace;

	int bankid = 0;
	int windowsize = 372;
	uint64_t totalconflict = 0;
	int bank[512];

	int rddone = 0;
	int done = 0;

	clock_t start, end;
	double cpu_time_used;
	struct hmcmtrace *rqst =  malloc(sizeof(struct hmcmtrace));


	/* ---- */
	while(( ret = getopt( argc, argv, "f:h" )) != -1 ) {
		switch( ret )
		{
			case 'f':
				sprintf( filename, "%s", optarg );
				break;
			case 'h' :
				print_help();
				return 0;
				break;
			default:
				printf("Unknown option!\n");
				print_help();
				return -1;
				break;
		}
	}

	/* sanity check */
	if( (strlen( filename ) == 0) ) {
		printf( "error : filename is invalid\n" );
		return -1;
	}

	infile = fopen( filename, "r" );
  if( infile == NULL ){
    printf( "error : could not open trace file %s\n", filename );
    return -1;
  }

	sprintf(outfname, "../bankconflicts.log");
	outfile = fopen(outfname, "a");
	if( outfile == NULL )
	{
		printf("ERROR: Cannot open bankconflict log file %s\n", outfname);
    return -1;
	}

	start = clock();
	i = 0;
	while( done != 1 ) {
		/* read trace */
		rddone = read_hmc_trace( infile, rqst );
		if( rddone == 0 ) {
			if( rqst->type != 2) {
				trace ++;
			}
		}
		while ( rddone != 0 ) {
			rddone = read_hmc_trace( infile, rqst );
			if( rddone == 0 ) {
				if( rqst->type != 2) {
					trace ++;
				}
			}
		}

		if( rqst -> type == 2 ) {
			// end of the trace file
			done = 1;
			break;
		}

		bankid = get_bankid(rqst->addr);
		if( bank[bankid] == 1) {
			totalconflict ++;
		} else {
			bank[bankid] = 1;
		}

		if( i == windowsize ) {
			zero_bank(bank);
			i = 0;
		} else {
			i++;
		}
	}
	fprintf(outfile, "# Trace file path:  %s\n", filename);
	fprintf(outfile, "# Bank Conflicts:   %" PRId64 "\n", totalconflict);
	fprintf(outfile, "#==============================================================================\n");
	return 0;
}

/* EOF */