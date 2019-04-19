/*
 * _PIMSID_C_  *
 * FUNCTIONS TO GET THE PIMS ID BASED ON THE ADDRESS OF ELEMENT
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* --------------------------------------------- GETSHIFTAMOUNT */

extern int getpimsid (uint32_t *pimsid,
                      uint64_t addr,
                      uint32_t shiftamt,
                      uint32_t num_vaults)
{
  if( pimsid == NULL ) {
    return -1;
  }

  switch( num_vaults )
  {
    case 32:
      *pimsid = (uint32_t) ((addr >> shiftamt) & 0x1F);
      break;
    case 16:
      *pimsid = (uint32_t) ((addr >> shiftamt) & 0xF);
      break;
    default:
      return -1;
      break;
  }
  return 0;
}

/* EOF */
