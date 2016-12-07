#ifndef __COMPRESSES_H__
#define __COMPRESSES_H__

#include "buffer.h"


enum {
    PL_COMP_OK        = 0,
    PL_COMP_INVALID   = -1,
    PL_COMP_MEMORY    = -2,
    PL_COMP_COMPRESS  = -3,
    PL_COMP_LENGTHS   = -4,

    PL_CHECK_COMPRESSESED = 1,
    PL_CHECK_NOT_COMP = 0,
    PL_CHECK_INVALID = -1,
    PL_CHECK_VERSION = -2,
    PL_CHECK_SIZE    = -3,
    PL_CHECK_CRC     = -4,
};


int             pl_compress(pl_buffer *input, pl_buffer **output, const char *key);
int             pl_check_compression(pl_buffer *input);


#endif
