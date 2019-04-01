/*
 * _COALESCE_C_  *
 * FUNCTIONS TO COALESCE THE MEMORY REQUESTS IN EACH PIMS UNIT
 *
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdint.h>
 #include <inttypes.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include "pims_struct.h"

/* ---------------------------------------------- FUNCTION PROTOTYPES*/
extern int getpimsid( uint32_t *pimsid,
                      uint64_t addr,
                      uint32_t shiftamt,
                      uint32_t num_vaults);

/* --------------------------------------------- JOINRQST */

extern int joinrqst( uint64_t addr,
                     uint32_t shiftamt,
                     uint32_t num_vaults,
                     uint32_t buf_size,
                     struct pimsbuf *pims_buf,
                     uint32_t cur_id,
                     int flag,
                     uint64_t *pim_rd_count_tmp,
                     FILE *outfile)
{
  /* vars */
  uint32_t *pimsid = NULL;
  uint32_t tmpid = 0x00l;
  uint64_t tmpaddr = 0x00ll;
  uint32_t i = 0x00l;
  uint32_t cur_insize = 0x00l;
  int out_flitindex = 0;
  int out_flitsize = 0;
  *pim_rd_count_tmp = 0;

  cur_insize = pims_buf[cur_id].in_size;

  /* Sanity Check */
  if( pims_buf == NULL || outfile == NULL) {
    return -1;
  }

  pimsid = malloc( sizeof( uint32_t ) * 1);
  if( pimsid == NULL ) {
    printf("Failed to allocation memory for pimsid\n");
    return -1;
  }

  if( (cur_insize < buf_size - 1) && (flag == 0) ) {
    // Insert current address to in_addr buffer
    pims_buf[cur_id].in_addr[cur_insize] = addr;
    pims_buf[cur_id].in_size ++;

  } else {
    // cur_insize = buf_size - 1, i.e. in_addr buffer is full
    // Colease the addresses
    for ( i = 0; i < buf_size; i++) {

      // Get PIMS id of the memory request destination
      tmpaddr = pims_buf[cur_id].in_addr[i];
      if( getpimsid( pimsid,
                     tmpaddr,
                     shiftamt,
                     num_vaults) != 0 ) {
        printf("Failed to get PIMS id\n");
      }
      tmpid = *pimsid;

      // Check if out_addr buffer of tmpid is full
      out_flitindex = pims_buf[cur_id].out_addr[tmpid].size;
      if( out_flitindex < (16 - 1) && (flag == 0) ) {
        pims_buf[cur_id].out_addr[tmpid].addr[out_flitindex] = tmpaddr;
        pims_buf[cur_id].out_addr[tmpid].size ++;
        *pim_rd_count_tmp += 0;
      } else {
        out_flitsize = out_flitindex + 1;
        fprintf( outfile,
                 "%s%d%s%016" PRIX64 "\n",
                 "RD:",
                 out_flitsize,
                 ":0:0x",
                 (uint64_t)pims_buf[cur_id].out_addr[tmpid].addr[0]);
        *pim_rd_count_tmp += 1;
        pims_buf[cur_id].out_addr[tmpid].size = 0;
        out_flitindex = 0;
        pims_buf[cur_id].out_addr[tmpid].addr[out_flitindex] = tmpaddr;
        pims_buf[cur_id].out_addr[tmpid].size ++;
      }
    }

    // Reset in_size,
    pims_buf[cur_id].in_size = 0;
    cur_insize = 0;
    pims_buf[cur_id].in_addr[cur_insize] = addr;
    pims_buf[cur_id].in_size ++;
  }
  return 0;
}

/* EOF */
