/*
 * _PIMS_STRUCT_H_
 *
 * PIMS HEADER
 *
 */

#include <stdint.h>

/*
 * struct pimsbuf
 * contains struct data for pims buffer
 */

struct pimsbuf{
  uint32_t pimsid;          // pims id
  uint32_t in_size;    // input buffer size
  uint32_t out_size;   // output buffer size
  uint64_t *in_addr;   // save input address array
  struct flit *out_addr;  // save output FLICT array
};

struct flit{
  uint32_t flitid;
  int size;
  uint64_t addr[16];
};
