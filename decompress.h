#ifndef __DECRYPT_H__
#define __DECRYPT_H__

#include "crypt.h"


enum {
    PL_DECOMP_INVALID    = -1,
    PL_DECOMP_MEMORY     = -2,
    PL_DECOMP_CHECK_COMP = -3,
    PL_DECOMP_DECR_SIZE  = -4,
    PL_DECOMP_WRONG_KEY  = -5,
    PL_DECOMP_DECOMPRESS = -6,
    PL_DECOMP_OK         = 0,
};


int pl_decompress(pl_buffer *input, pl_buffer **output, const char *key,
        int check_compression);


#endif
