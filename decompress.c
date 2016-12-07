#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "decompress.h"
#include "lzf.h"
#include "compress.h"
#include "crc32.h"


int pl_decompress(pl_buffer *input, pl_buffer **output, const char *key, int check_compression) {

    pl_buffer *result;
    unsigned int data_size, real_size, *header,
            header_size, crc32_ptr_compresed, i;
    char   *data;

    if(!input)
        return PL_DECOMP_INVALID;

    if(check_compression && pl_check_compression(input) != PL_CHECK_COMPRESSESED) {
        return PL_DECOMP_CHECK_COMP;
    }

    *output = NULL;

    header = (unsigned int*)((char*)input->buffer + pl_header_magic_text_size);

    // skip version
    ++header;

    // get crc32 of compressed
    crc32_ptr_compresed = *header++;

    // get real size
    real_size = *header++;

    // get compressed size
    data_size = *header;

    header_size = PL_HEADER_ITEMS_SIZE + pl_header_magic_text_size;

    // check buffer crc
    if(crc32_ptr_compresed != calculate_crc32((char*)input->buffer + header_size, data_size)) {
        return PL_DECOMP_WRONG_KEY;
    }

    // decompress
    result = pl_buf_create(real_size + 16); // with dummy
    if(!result) {
        return PL_DECOMP_MEMORY;
    }

    result->length = lzf_decompress((char*)input->buffer + header_size, data_size,
            result->buffer, result->allocated_size);

    if(result->length == 0) {
        pl_buf_destroy(result);
        return PL_DECOMP_DECOMPRESS;
    }

    result->length = real_size;

    *output = result;

    return PL_DECOMP_OK;
}
