#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "compress.h"
#include "lzf.h"
#include "crc32.h"


//----------------------------------------------------------------------------//
int pl_compress(pl_buffer *input, pl_buffer **output, const char *key) {

    pl_buffer *result;
    unsigned int *header, *crc32_ptr_compressed, i, reserved_header_size, real_size;
    char *data;

    *output = (pl_buffer *)NULL;

    if(!input) {
        return PL_COMP_INVALID;
    }

    // header size
    reserved_header_size = PL_HEADER_ITEMS_SIZE + pl_header_magic_text_size;

    // get more memory to prevent lzf overflowing
    result = pl_buf_create(input->length + 8192);
    if(!result) {
        return PL_COMP_MEMORY;
    }

    header = (unsigned int*)((char*)result->buffer + pl_header_magic_text_size);

    // copy header magic text
    memcpy(result->buffer, pl_header_magic_text, pl_header_magic_text_size);

    // put version
    *header++ = 0x00000001;
    // get pointer to crc32 of source data
    crc32_ptr_compressed = header++;
    // put uncompressed size
    *header++ = input->length;

    // compress
    result->length =
            lzf_compress((char*)input->buffer, input->length,
                (char*)result->buffer + reserved_header_size, result->allocated_size - reserved_header_size);

    if(!result->length) {
        pl_buf_destroy(result);
        return PL_COMP_COMPRESS;
    }

    // compressed data size
    *header++ = result->length;

    // calculate crc32 of compressed data
    *crc32_ptr_compressed = calculate_crc32((char*)result->buffer +
        reserved_header_size, result->length);

    result->length += reserved_header_size;

    *output = result;

    return PL_COMP_OK;
}

//----------------------------------------------------------------------------//
int pl_check_compression(pl_buffer *input) {

    unsigned int    header_size = PL_HEADER_ITEMS_SIZE + pl_header_magic_text_size,
                    data_size,
                    *header,
                    crc32;

    if(!input)
        return PL_CHECK_INVALID;

    if(input->length < header_size) {
        // no compression
        return PL_CHECK_NOT_COMP;
    }

    // check magic header
    if(memcmp(input->buffer, pl_header_magic_text, pl_header_magic_text_size) != 0) {
        // no compression
        return PL_CHECK_NOT_COMP;
    }

    header = (unsigned int*)((char*)input->buffer + pl_header_magic_text_size);

    // check version
    if(*header++ != 0x00000001) {
        return PL_CHECK_VERSION;
    }

    // get crc32 of compressed
    crc32 = *header++;

    // skip real size
    header += 1;

    // check compressed data size
    data_size = input->length - header_size;
    if(*header != data_size) {
        return PL_CHECK_SIZE;
    }

    // check crc32
    if(crc32 != calculate_crc32((char*)input->buffer + header_size, data_size)) {
        return PL_CHECK_CRC;
    }

    // all right!
    return PL_CHECK_COMPRESSESED;
}
