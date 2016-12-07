#include <stdio.h>
#include <stdlib.h>

#ifdef __gnu_linux__
#include <unistd.h>
#endif

#include <string.h>

#include "crypt.h"

//----------------------------------------------------------------------------//
int pl_buf_seek(pl_buffer *buffer, unsigned int seek) {

    buffer->pos = seek;
    return PL_COMP_E_OK;
}

//----------------------------------------------------------------------------//
int pl_reallocate(pl_buffer *buffer, unsigned int new_size) {

    if(buffer->allocated_size == new_size)
        return PL_COMP_E_OK;

    buffer->buffer = realloc(buffer->buffer, new_size);
    if(!buffer) {
        memset(buffer, 0, sizeof(pl_buffer));
        return PL_COMP_E_INVALID;
    }

    buffer->allocated_size = new_size;
    return PL_COMP_E_OK;
}

//----------------------------------------------------------------------------//
pl_buffer*      pl_buf_create(unsigned int size) {

    pl_buffer   *buffer = (pl_buffer*)malloc(sizeof(pl_buffer));
    if(!buffer)
        return NULL;

    memset(buffer, 0, sizeof(pl_buffer));
    buffer->buffer = (void*)malloc(size);
    if(!buffer->buffer) {
        pl_buf_destroy(buffer);
        return NULL;
    }

    buffer->allocated_size = size;

    return buffer;
}

//----------------------------------------------------------------------------//
pl_buffer*      pl_buf_create_from(void *buf, unsigned int size) {

    pl_buffer   *buffer = (pl_buffer*)malloc(sizeof(pl_buffer));
    if(!buffer)
        return NULL;

    memset(buffer, 0, sizeof(pl_buffer));
    buffer->buffer = (void*)malloc(size);
    if(!buffer->buffer) {
        pl_buf_destroy(buffer);
        return NULL;
    }
    memcpy(buffer->buffer, buf, size);

    buffer->allocated_size = size;
    buffer->length = size;

    return buffer;
}

//----------------------------------------------------------------------------//
int             pl_buf_assign(pl_buffer *buffer, void *buf, unsigned int size) {

    if(!buffer)
        return PL_COMP_E_INVALID;

    if(pl_reallocate(buffer, size))
        return PL_COMP_E_MEMORY;

    memcpy(buffer->buffer, buf, size);

    buffer->pos            = 0;
    buffer->length         = size;

    return PL_COMP_E_OK;
}

//----------------------------------------------------------------------------//
int             pl_buf_read_file(pl_buffer *buffer, char *filename) {

    FILE *fp;
    unsigned int size;

    if(!buffer)
        return PL_COMP_E_INVALID;

    fp = fopen(filename, "rb");

    if(fp == NULL)
        return PL_COMP_E_FILE;

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if(pl_reallocate(buffer, size)) {
        fclose(fp);
        return PL_COMP_E_MEMORY;
    }

    fread(buffer->buffer, 1, size, fp);
    buffer->pos            = 0;
    buffer->length         = size;
    fclose(fp);

    return PL_COMP_E_OK;
}

//----------------------------------------------------------------------------//
int            pl_buf_write_file(pl_buffer *buffer, char *filename) {

    FILE *fp;

    if(!buffer)
        return PL_COMP_E_INVALID;

    fp = fopen(filename, "wb");
    if(fp == NULL)
        return PL_COMP_E_FILE;

    fwrite(buffer->buffer, 1, buffer->length, fp);

    fclose(fp);

    return PL_COMP_E_OK;
}

//----------------------------------------------------------------------------//
int            pl_buf_write(pl_buffer *buffer, void *data, unsigned int size) {

    if(!buffer)
        return PL_COMP_E_INVALID;

    if(buffer->pos + size >= buffer->allocated_size) {
        if(pl_reallocate(buffer, buffer->pos + size + 8192))
            return PL_COMP_E_MEMORY;
        buffer->length = buffer->pos + size;
    }

    if(buffer->pos + size > buffer->length)
        buffer->length = buffer->pos + size;

    memcpy((char*)buffer->buffer + buffer->pos, data, size);
    buffer->pos += size;

    return size;
}

//----------------------------------------------------------------------------//
int            pl_buf_read(pl_buffer *buffer, void *data, unsigned int size) {
    unsigned int readed_size = 0;

    if(!buffer)
        return PL_COMP_E_INVALID;

    if(buffer->pos + size > buffer->length)
        size = buffer->length - buffer->pos;

    memcpy(data, (char*)buffer->buffer + buffer->pos, size);
    buffer->pos += size;

    return size;
}

//----------------------------------------------------------------------------//
void            pl_buf_destroy(pl_buffer *ptr) {

    if(!ptr)
        return;

    if(ptr->buffer && ptr->allocated_size)
        free(ptr->buffer);

    ptr->buffer = NULL;
    ptr->allocated_size = 0;

    free(ptr);

}
