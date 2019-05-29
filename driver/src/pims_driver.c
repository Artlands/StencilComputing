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
#include "hmc_sim_addtypes.h"
#include "hmc_sim.h"

/* -------------------------------------------------- PRINT_HELP */
static void print_help(){
	printf( "------------------------------------------------------\n" );
	printf( "   MEMORY TRACE POWER CONSUMPATION | BANK CONFLICT    \n" );
	printf( "------------------------------------------------------\n" );
	printf( " Usage: run_hmcsim <options>\n" );
	printf( " <options>\n" );
	printf( " -h                       : print this help\n" );
	printf( " -f /path/to/filename     : read trace from file\n" );
	printf( " -C /path/to/config/file  : read config from file\n" );
	printf( "------------------------------------------------------\n" );
}

/*Print Power*/
static int print_power(struct hmcsim_t *hmc) {

	if( hmc == NULL ){
    return -1;
  }else if( hmc->tfile == NULL ){
    return -1;
	}
  /*
   * write out the total trace values for power on this clock cycle
   *
   */
    fprintf( hmc->tfile, "%s%"PRIu64"%s%"PRIu64"\n",
                         "HMCSIM_TRACE : ",
                         hmc->clk,
                         " : T_LINK_PHY_POWER : ",
                         hmc->power.t_link_phy );
    fprintf( hmc->tfile, "%s%"PRIu64"%s%"PRIu64"\n",
                         "HMCSIM_TRACE : ",
                         hmc->clk,
                         " : T_LINK_LOCAL_ROUTE_POWER : ",
                         hmc->power.t_link_local_route );
    fprintf( hmc->tfile, "%s%"PRIu64"%s%"PRIu64"\n",
                         "HMCSIM_TRACE : ",
                         hmc->clk,
                         " : T_LINK_REMOTE_ROUTE_POWER : ",
                         hmc->power.t_link_remote_route );
    fprintf( hmc->tfile, "%s%"PRIu64"%s%"PRIu64"\n",
                         "HMCSIM_TRACE : ",
                         hmc->clk,
                         " : T_XBAR_RQST_SLOT_POWER : ",
                         hmc->power.t_xbar_rqst_slot);
    fprintf( hmc->tfile, "%s%"PRIu64"%s%"PRIu64"\n",
                         "HMCSIM_TRACE : ",
                         hmc->clk,
                         " : T_XBAR_RSP_SLOT_POWER : ",
                         hmc->power.t_xbar_rsp_slot);
    fprintf( hmc->tfile, "%s%"PRIu64"%s%"PRIu64"\n",
                         "HMCSIM_TRACE : ",
                         hmc->clk,
                         " : T_XBAR_ROUTE_EXTERN_POWER : ",
                         hmc->power.t_xbar_route_extern);
    fprintf( hmc->tfile, "%s%"PRIu64"%s%"PRIu64"\n",
                         "HMCSIM_TRACE : ",
                         hmc->clk,
                         " : T_VAULT_RQST_SLOT_POWER : ",
                         hmc->power.t_vault_rqst_slot);
    fprintf( hmc->tfile, "%s%"PRIu64"%s%"PRIu64"\n",
                         "HMCSIM_TRACE : ",
                         hmc->clk,
                         " : T_VAULT_RSP_SLOT_POWER : ",
                         hmc->power.t_vault_rsp_slot);
    fprintf( hmc->tfile, "%s%"PRIu64"%s%"PRIu64"\n",
                         "HMCSIM_TRACE : ",
                         hmc->clk,
                         " : T_VAULT_CTRL_POWER : ",
                         hmc->power.t_vault_ctrl);
    fprintf( hmc->tfile, "%s%"PRIu64"%s%"PRIu64"\n",
                         "HMCSIM_TRACE : ",
                         hmc->clk,
                         " : T_ROW_ACCESS_POWER : ",
                         hmc->power.t_row_access);
		return 0;
};

// static int print_latency(struct hmcsim_t *hmc) {
//     if( hmc->tfile == NULL) {
// 	return -1;
//     }
//     fprintf( hmc->tfile, "HMCSIM_TRACE : %" PRIu64
//                         " : PACKET_LATENCY : %" PRIu64 "\n", hmc->clk, hmc->istat.t_latency);
// }

static int print_bankconflict(struct hmcsim_t *hmc) {
    if( hmc->tfile == NULL) {
	return -1;
    }
    fprintf( hmc->tfile, "HMCSIM_TRACE : %" PRIu64
                        " : BAND_CONFLICT : %" PRIu64 "\n", hmc->clk, hmc->istat.bank_conflict);
}
/* ----------------------------------------------GET_HMCMEMOP */
/* FOR DMC POWER*/
struct hmcmtrace {
  uint32_t proc;	/* proc id */
  int type;       /* read or write */
  hmc_rqst_t op;    /* memory operation type */
  uint64_t addr;  /* base address of the request */
};

static int get_hmcmemop( int type, int nbytes, hmc_rqst_t *op ){

	if( type == 0 ){
		switch( nbytes )
		{
			case 1 :
				*op = WR8;
				break;
			case 2 :
				*op = WR8;
				break;
			case 4 :
				*op = WR8;
				break;
			case 8 :
				*op = WR8;
				break;
			case 16:
				*op = WR16;
				break;
			case 32:
				*op = WR32;
				break;
			case 48:
				*op = WR48;
				break;
			case 64:
				*op = WR64;
				break;
			case 80:
				*op = WR80;
				break;
			case 96:
				*op = WR96;
				break;
			case 112:
				*op = WR112;
				break;
			case 128:
				*op = WR128;
				break;
			// case 256:
			// 	*op = WR256;
			// 	break;
			default:
				return -1;
				break;
		}
		return 0;
	}else if( type == 1 ){
		switch( nbytes )
		{
			case 1 :
				*op = RD8;
				break;
			case 2 :
				*op = RD8;
				break;
			case 4 :
				*op = RD8;
				break;
			case 8 :
				*op = RD8;
				break;
			case 16:
				*op = RD16;
				break;
			case 32:
				*op = RD32;
				break;
			case 48:
				*op = RD48;
				break;
			case 64:
				*op = RD64;
				break;
			case 80:
				*op = RD80;
				break;
			case 96:
				*op = RD96;
				break;
			case 112:
				*op = RD112;
				break;
			case 128:
				*op = RD128;
				break;
			// case 256:
			// 	*op = RD256;
			// 	break;
			default:
				return -1;
				break;
		}
		return 0;
	}else if ( type == 2 ){
		*op = FLOW_NULL;
	}else{
		return -1;
	}
	return 0;
}

/* ------------------------------------------------- RQST_PACKET_LENGTH */
static int rqst_packet_length( hmc_rqst_t op ){
	int val = 0;

	switch ( op ) {
		// case WR256:
		// 	val = 9;
		// 	break;
		case WR128:
			val = 9;
			break;
		case WR112:
			val = 8;
			break;
		case WR96:
			val = 7;
			break;
		case WR80:
			val = 6;
			break;
		case WR64:
			val = 5;
			break;
		case WR48:
			val = 4;
			break;
		case WR32:
			val = 3;
			break;
		case WR16:
    case WR8:
			val = 2;
			break;
		default:
			val = 1;
			break;
	}

  return val;
}

/* ---------------------------------------------------- ZERO_PACKET */
/* ZERO_PACKET */

static void zero_packet( uint64_t *packet ) {
	uint64_t i = 0x00ll;
	for( i=0; i<HMC_MAX_UQ_PACKET; i++ ){
		packet[i] = 0x00ll;
	}
	return ;
}

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


/* ----------------------------------------------- MAIN */
int main( int argc, char **argv ) {

	/* vars */
	int ret = 0;

	FILE *infile = NULL;
	FILE *outfile = NULL;		//save hmcsim power evaluation to an output file
	char filename[1024];
  char outfname[1024];

	clock_t start, end;
	double cpu_time_used;
	struct hmcmtrace *rqst =  malloc(sizeof(struct hmcmtrace));

	/* vars for init hmc-sim */
	uint32_t i		= 0;
	uint32_t num_devs	= 1;
	uint32_t num_links	= 4;
	uint32_t num_vaults	= 32;
	uint32_t queue_depth	= 64;
	uint32_t num_banks	= 16;
	uint32_t num_drams	= 20;
	uint32_t capacity	= 8;
	uint32_t xbar_depth	= 128;
  uint32_t bsize  = 256;
	char *cfile = NULL;
	struct hmcsim_t hmc;

	/* vars for running hmc-sim */
	uint64_t head	= 0x00ll;
  uint64_t tail	= 0x00ll;
  uint64_t payload[16]	= { 0x00ll, 0x00ll, 0x00ll, 0x00ll,
	                         	0x00ll, 0x00ll, 0x00ll, 0x00ll,
														0x00ll, 0x00ll, 0x00ll, 0x00ll,
														0x00ll, 0x00ll, 0x00ll, 0x00ll
													 };

	int simret = HMC_OK; 	// for running hmc-sim
	int rddone = 0; 			// for read_hmc_trace
	// int bidret = HMC_OK;	// for hmcsim_send_memrequest
	uint32_t done	= 0;

	uint8_t	cub		= 0;
	uint8_t link		= 0;
	uint16_t tag	= 0;
	int stall_sig	= 0;

	uint32_t plen = 0;	// hmc memory request packet size
	uint64_t packet[HMC_MAX_UQ_PACKET];

	int *rtns	= NULL;		//record status for each link
	long all_sent	= 0;
	long all_recv	= 0;
	long trace = 0;

	uint64_t d_response_head;
  uint64_t d_response_tail;
  hmc_response_t d_type;
  uint8_t d_length;
  uint16_t d_tag;
  uint8_t d_rtn_tag;
  uint8_t d_src_link;
  uint8_t d_rrp;
  uint8_t d_frp;
  uint8_t d_seq;
  uint8_t d_dinv;
  uint8_t d_errstat;
  uint8_t d_rtc;
  uint32_t d_crc;

	/* ---- */
	while(( ret = getopt( argc, argv, "f:hC:D" )) != -1 ) {
		switch( ret )
		{
			case 'f':
				sprintf( filename, "%s", optarg );
				break;
			case 'h' :
				print_help();
				return 0;
				break;
			case 'C' :
				cfile = malloc( sizeof( char ) * (strlen(optarg) + 1) );
				sprintf( cfile, "%s", optarg);
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

	/*
   * init the library
   *
   */
  ret = hmcsim_init( &hmc,
	                   num_devs,
	                   num_links,
	                   num_vaults,
	                   queue_depth,
	                   num_banks,
	                   num_drams,
	                   capacity,
	                   xbar_depth );
  if( ret != 0 ){
    printf( "FAILED TO INIT HMCSIM\n" );
    return -1;
  }else{
    printf( "SUCCESS : INITIALIZED HMCSIM\n" );
  }

	if( cfile != NULL ) {
		if( hmcsim_read_config( &hmc, cfile ) != 0 ) {
			printf("FAILED TO READ CONFIG FILE : %s\n", cfile );
			hmcsim_free( &hmc );
			free(rqst);
			return -1;
		}
	}

	/*
   * setup the device topology
   *
   */
  if( num_devs > 1 ){
    /*
     * MULTIPLE DEVICES
     *
     */
  }else{
    /*
     * SINGLE DEVICE
     *
     */

    for( i=0; i<num_links; i++ ){
      ret = hmcsim_link_config( &hmc,
                                (num_devs+1),
                                0,
                                i,
                                i,
                                HMC_LINK_HOST_DEV );

      if( ret != 0 ){
        printf( "FAILED TO INIT LINK %d\n", i );

        hmcsim_free( &hmc );
				free(rqst);

        return -1;
      }else{
        printf( "SUCCESS : INITIALIZED LINK %d\n", i );
      }
    }
  }

	/*
   * init the max request block size
   *
   */
  ret = hmcsim_util_set_all_max_blocksize( &hmc, bsize );

  if( ret != 0 ){
    printf( "FAILED TO INIT MAX BLOCKSIZE\n" );
    hmcsim_free( &hmc );
		free(rqst);
    return -1;
  }else {
    printf( "SUCCESS : INITIALIZED MAX BLOCK SIZE\n" );
  }

	/*
	 *open the file
	 *
	 *
	 */
  infile = fopen( filename, "r" );
  if( infile == NULL ){
    printf( "error : could not open file %s\n", filename );
  	hmcsim_free( &hmc );
  	free(rqst);
    return -1;
  }


  sprintf(outfname, "../hmcsim/%s", filename);
  outfile = fopen( outfname, "w" );
  if( outfile == NULL ){
    printf( "FAILED : COULD NOT OPEN OUPUT FILE hmc_trace.out\n" );
    hmcsim_free( &hmc );
    free(rqst);
    return -1;
  }

	/*
   * execute the hmc-sim
   * setup the tracing mechanisms
   */
	rtns = malloc( sizeof( int ) * hmc.num_links );
	memset( rtns, 0, sizeof( int ) * hmc.num_links );

	hmcsim_trace_handle( &hmc, outfile );
	hmcsim_trace_level( &hmc, (HMC_TRACE_BANK|        // trace bank info
														 HMC_TRACE_POWER) );		// trace power info
	hmcsim_trace_header( &hmc );

	printf( "SUCCESS : INITIALIZED TRACE HANDLERS\n" );

  /*
   * zero the packet
   *
   */
  zero_packet( &(packet[0]) );

  printf( "BEGINNING EXECUTION\n" );

	/*
	 * Attempt to execute all the requests
	 * Push requests into the device
	 * until we get a stall signal
 	 */
	start = clock();
	while( done != 1 ) {

		/*
	 	 * attempt to push a request in
		 * as long as we don't stall
		 *
	 	 */

		/* read first trace */
		if (rqst == NULL) {
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
		}

		if( rqst->type == 2) {
			/* end of the file, everything is sent, go to receive side */
			goto packet_recv;
		}

		// printf( "....sending packets\n" );
			/*
			 * try to push another request
			 *
			 * Build and send the Request
			 *
			 */
		while (simret != HMC_STALL) {
			// if(rqst->type == 0)
			// 	rqst->op = RD16;
			// else
			// 	rqst->op = WR16;
			simret = hmcsim_build_memrequest( &hmc,
																				cub,
																				rqst->addr,
																				tag,
																				rqst->op,
																				link,
																				&(packet[0]),
																				&head,
																				&tail );

			if( simret == 0) {
				plen = rqst_packet_length(rqst->op);
				// printf( "SENDING PACKET WITH %d FLITS", plen );
				if( plen > 1 ){
          // lay out packet
          packet[0] = head;
          for(i=0; i<((plen-1)*2); i++ ){
            packet[i+1] = payload[i];
          }
          packet[(plen*2)-1] = tail;
        }else{
          packet[0] = head;
          packet[1] = tail;
        }
        simret = hmcsim_send( &hmc, &(packet[0]) );
			} else {
				// printf("ERROR : FATAL : MALFORMED PACKET\n");
			}

			switch( simret ) {
				case 0:
					// printf( "SUCCESS : PACKET WAS SUCCESSFULLY SENT\n" );
					all_sent++;
					/* read the next request */
  				rddone = read_hmc_trace( infile, rqst );
					if( rddone == 0 ) {
						if( rqst->type != 2) {
							trace ++;
						}
					}
  				while ( rddone != 0 ) {
  					rddone = read_hmc_trace( infile, rqst);
						if( rddone == 0 ) {
							if( rqst->type != 2) {
								trace ++;
							}
						}
  				}
					break;
				case HMC_STALL:
					// printf( "STALLED : PACKET WAS STALLED IN SENDING\n" );
					break;
				case -1:
				default:
					// printf( "FAILED : PACKET SEND FAILED\n" );
					goto complete_failure;
					break;
			}

			/*
			 * zero the packet
			 *
			 */
			zero_packet( &(packet[0]) );

			tag++;
			if( tag == 2048 ){
				tag = 0;
			}

			link++;
			if( link == hmc.num_links ){
				/* -- TODO : look at the number of connected links
				 * to the host processor
				 */
				link = 0;
			}

			/*
			 * check to see if we're at the end of the packet queue
			 *
			 */

 			if( rqst->type == 2) {
 				/* everything is sent, go to receive side */
 				goto packet_recv;
 			}
			/* DONE SENDING REQUESTS */
		}
	packet_recv:
		/*
		 * reset the return code for receives
		 *
		 */
		simret = HMC_OK;

		/*
		 * We hit a stall or an error
		 *
		 * Try to drain the responses off all the links
		 *
		 */
		// printf( "...reading responses\n" );

		while( simret != HMC_STALL) {

			for( i = 0; i < hmc.num_links; i++) {

				rtns[i] = hmcsim_recv( &hmc, cub, i, &(packet[0]) );

				if( rtns[i] == HMC_STALL ) {
					stall_sig++;
				}else {
					/* successfully received a packet */
					// printf( "SUCCESS : RECEIVED A SUCCESSFUL PACKET RESPONSE\n" );
          hmcsim_decode_memresponse(  &hmc,
	                                    &(packet[0]),
	                                    &d_response_head,
	                                    &d_response_tail,
	                                    &d_type,
	                                    &d_length,
	                                    &d_tag,
	                                    &d_rtn_tag,
	                                    &d_src_link,
	                                    &d_rrp,
	                                    &d_frp,
	                                    &d_seq,
	                                    &d_dinv,
	                                    &d_errstat,
	                                    &d_rtc,
	                                    &d_crc );
					// printf( "RECV tag=%d; rtn_tag=%d\n", d_tag, d_rtn_tag );
					all_recv++;
				}
				/*
				 * zero the packet
				 *
				 */
				zero_packet( &(packet[0]) );
			}

			/* count the number of stall signals received */
			if( stall_sig == hmc.num_links ){
				/*
				 * if all links returned stalls,
				 * then we're done receiving packets
				 *
				 */

				// printf( "STALLED : STALLED IN RECEIVING\n" );
				simret = HMC_STALL;
			}

			stall_sig = 0;
			for( i=0; i< hmc.num_links; i++){
				rtns[i] = HMC_OK;
			}
		}

		/*
		 * reset the return code
		 *
		 */
		stall_sig = 0;
		for( i=0; i< hmc.num_links; i++){
			rtns[i] = HMC_OK;
		}
		simret = HMC_OK;

		/*
	 	 * done with sending/receiving packets
		 * update the clock
		 */
		// printf( "SIGNALING HMCSIM TO CLOCK\n" );

		// printf( "ALL_SENT = %ld\n", all_sent );
		// printf( "0x%016llx\n",rqst.addr );

		hmcsim_clock( &hmc );

		// printf( "ALL_SENT = %ld\n", all_sent );
		// printf( "ALL_RECV = %ld\n", all_recv );

		if( rqst->type == 2 ) {
			if( all_sent == all_recv ){
				printf( "ALL_TRACES = %ld\n", trace );
				done = 1;
				print_power(&hmc);
				print_bankconflict(&hmc);
				end = clock();
				cpu_time_used = ( (double) (end - start) )/CLOCKS_PER_SEC;
				printf( "Total Computing time is : %f s\n", cpu_time_used );
			}
		}
	}

complete_failure:

	hmcsim_free( &hmc );
	free(rqst);
	if( cfile != NULL ) {
		free(cfile);
		cfile = NULL;
	}

  fclose( infile );
	fclose( outfile );
	infile = NULL;
	outfile = NULL;

	free( rtns );
	rtns = NULL;

	return 0;
}

/* EOF */
