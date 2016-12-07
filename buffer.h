#ifndef __CRYPT_H__
#define __CRYPT_H__


typedef struct _pl_buffer {

    unsigned int                pos;
    unsigned int                length;
    unsigned int                allocated_size;
    void                       *buffer;

} pl_buffer;

enum {
    PL_COMP_E_OK      = 0,
    PL_COMP_E_INVALID = -1,
    PL_COMP_E_MEMORY  = -2,
    PL_COMP_E_FILE    = -3,
};

pl_buffer*      pl_buf_create(unsigned int size);
pl_buffer*      pl_buf_create_from(void *buf, unsigned int size);
int             pl_buf_assign(pl_buffer *buffer, void *buf, unsigned int size);
int             pl_buf_seek(pl_buffer *buffer, unsigned int seek);
int             pl_buf_read_file(pl_buffer *buffer, char *filename);
int             pl_buf_write_file(pl_buffer *buffer, char *filename);
int             pl_buf_write(pl_buffer *buffer, void *data, unsigned int size);
int             pl_buf_read(pl_buffer *buffer, void *data, unsigned int size);
void            pl_buf_destroy(pl_buffer *ptr);


#define PL_HEADER_ITEMS_SIZE            16

static const char *pl_header_magic_text =
    "<html><body><h2>Enable PHPLZF extension</h2></html><?php die();__halt_compiler();"
;
static const int pl_header_magic_text_size = 80;

#endif
