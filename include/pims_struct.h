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
  int pimsid;          // pims id
  uint32_t in_size;    // input buffer size
  uint32_t out_size;   // output buffer size
  uint64_t *in_addr;   // save input address array
  uint64_t *out_addr;  // save output address array
};
