#define PHPLZF_VERSION "0.1.1"

#include <php_version.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _WIN32
#if (PHP_MAJOR_VERSION >= 5 && PHP_MINOR_VERSION >= 5)
#define PHP_COMPILER_ID "VC11"
#else
#define PHP_COMPILER_ID "VC9"
#endif
#endif

#ifdef _WIN32
#include "zend_config.w32.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "module.h"
#include "compress.h"
#include "decompress.h"


#ifdef _WIN32
ZEND_GET_MODULE(phplzf)
ZEND_DECLARE_MODULE_GLOBALS(phplzf)
#endif


zend_function_entry phplzf_functions[] = {
    {NULL, NULL, NULL}
};



zend_module_entry phplzf_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "phplzf",
    phplzf_functions,
    PHP_MINIT(phplzf),
    PHP_MSHUTDOWN(phplzf),
    PHP_RINIT(phplzf),
    PHP_RSHUTDOWN(phplzf),
    PHP_MINFO(phplzf),
#if ZEND_MODULE_API_NO >= 20010901
    "2.0",
#endif
    STANDARD_MODULE_PROPERTIES
};


#ifdef COMPILE_DL_PHPLZF
ZEND_GET_MODULE(phplzf)
#endif


//
// Default handle for phplzf
//
typedef struct _phplzf_handler {
    pl_buffer           *buf;
    unsigned int        real_size;
    void                *stream_handle;
    zend_file_handle    *zend_handler;
} phplzf_handler;



//------------------------------------------------------------------------------------------------//
int (*phplzf_orig_stream_open_function)(const char *filename, zend_file_handle *handle TSRMLS_DC);
zend_op_array *(*phplzf_orig_compile_file)(zend_file_handle *file_handle, int type TSRMLS_DC);


//------------------------------------------------------------------------------------------------//
//
// Read by stream
//
static size_t phplzf_zend_stream_reader(void *handle, char *buf, size_t len) /* {{{ */
{
    phplzf_handler *file_handle = handle;
    size_t res = pl_buf_read(file_handle->buf, buf, len);

    return res;

}


//------------------------------------------------------------------------------------------------//
//
// Avoid stream
//
static void phplzf_zend_stream_closer(void* handle TSRMLS_DC) {
    phplzf_handler *file_handle = handle;

#if (PHP_MAJOR_VERSION >= 5 && PHP_MINOR_VERSION >= 4)
    // php_stream_close(file_handle->zend_handler);
#else
    php_stream_close(file_handle->zend_handler->handle.stream.handle);
#endif
    pl_buf_destroy(file_handle->buf);
    efree(handle);
}


//------------------------------------------------------------------------------------------------//
//
// Get whole data size
//
static size_t phplzf_zend_stream_fsizer(void *handle TSRMLS_DC)
{
    phplzf_handler *file_handle = handle;
    return file_handle->real_size;
}


//------------------------------------------------------------------------------------------------//
//
// Read and decompress php files
//
int phplzf_open_by_filedesc(zend_file_handle *file_handle,
        pl_buffer *pl_buf_in) {

    unsigned int                       *header =
        (unsigned int *)((char*)pl_buf_in->buffer +
            pl_header_magic_text_size);

    phplzf_handler                *handle = (phplzf_handler*)
            emalloc(sizeof(phplzf_handler));

    int res = pl_decompress(pl_buf_in, &handle->buf, "not_working", 1);

    header += 2;

    if(res != PL_DECOMP_OK) {
        return 0;
    }

    handle->zend_handler  = file_handle;
    handle->stream_handle = file_handle->handle.stream.handle;
    handle->real_size     = *header++;

    file_handle->type                   = ZEND_HANDLE_STREAM;
    file_handle->handle.stream.handle   = handle;
    file_handle->handle.stream.reader   = phplzf_zend_stream_reader;
    file_handle->handle.stream.closer   = phplzf_zend_stream_closer;
    file_handle->handle.stream.fsizer   = phplzf_zend_stream_fsizer;
    file_handle->handle.stream.isatty   = 0;

    return 0;
}

//------------------------------------------------------------------------------------------------//
//
// Read compressed file
//
static zend_op_array *phplzf_open_protected(zend_file_handle *file_handle, int type TSRMLS_DC) {

    int             is_occurred_exception;
    zend_op_array  *result;
    pl_buffer      *pl_buf_in;
    char           *buf = NULL;
    size_t          len = 0;

    // fix for require and include.
    if(file_handle->type == ZEND_HANDLE_FILENAME) {
        if(zend_stream_fixup(file_handle, &buf, &len TSRMLS_CC) != SUCCESS) {
            len = 0;
        }
    }

    if(file_handle->type == ZEND_HANDLE_MAPPED || len > 0) {

        // check length and version.
        if(file_handle->handle.stream.mmap.len > pl_header_magic_text_size + PL_HEADER_ITEMS_SIZE &&
            *(unsigned int*)((char*)file_handle->handle.stream.mmap.buf + pl_header_magic_text_size) == 0x00000001) {

            // assign buffer
            pl_buf_in = pl_buf_create_from(file_handle->handle.stream.mmap.buf,
                file_handle->handle.stream.mmap.len);
            if(pl_buf_in) {
                phplzf_open_by_filedesc(file_handle, pl_buf_in);
                pl_buf_destroy(pl_buf_in);
            }
        }

    }

    zend_try {
        is_occurred_exception = 0;
        result = phplzf_orig_compile_file(file_handle, type TSRMLS_CC);
    } zend_catch {
        is_occurred_exception = 1;
    } zend_end_try();

    if(is_occurred_exception) {
        zend_bailout();
    }

    return result;
}

//------------------------------------------------------------------------------------------------//
// Overrided zend_file_compile
static zend_op_array *phplzf_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC)
{
    if (!file_handle || !file_handle->filename || file_handle->type == ZEND_HANDLE_STREAM) {
        return phplzf_orig_compile_file(file_handle, type TSRMLS_CC);
    }

    return phplzf_open_protected(file_handle, type TSRMLS_CC);
}


// Types for APC
typedef zend_op_array* (zend_compile_t)(zend_file_handle*, int TSRMLS_DC);
typedef zend_compile_t* (*set_compile_hook_t)(zend_compile_t *ptr);


//------------------------------------------------------------------------------------------------//
PHP_MINIT_FUNCTION(phplzf)
{

    // APC
    zval                    apc_magic_constant;
    set_compile_hook_t      set_compile_hook = NULL;

    static const char apc_magic_text[] = "\000apc_magic";
    static const int  apc_magic_len    = sizeof(apc_magic_text);


    // Check APC availability
    if(!set_compile_hook &&
        zend_get_constant(apc_magic_text, apc_magic_len - 1, &apc_magic_constant TSRMLS_CC)) {

        if(Z_LVAL(apc_magic_constant) != 0) {
            set_compile_hook = ((set_compile_hook_t)Z_LVAL(apc_magic_constant));
            phplzf_orig_compile_file = set_compile_hook(phplzf_compile_file);
        }

    }
    else {
        phplzf_orig_compile_file = zend_compile_file;
        zend_compile_file             = phplzf_compile_file;
    }

    return SUCCESS;
}


//------------------------------------------------------------------------------------------------//
PHP_MSHUTDOWN_FUNCTION(phplzf)
{
    if (zend_compile_file == phplzf_compile_file) {
        zend_compile_file = phplzf_orig_compile_file;
    }

    return SUCCESS;
}


//------------------------------------------------------------------------------------------------//
PHP_RINIT_FUNCTION(phplzf)
{
    return SUCCESS;
}


//------------------------------------------------------------------------------------------------//
PHP_RSHUTDOWN_FUNCTION(phplzf)
{
    return SUCCESS;
}


//------------------------------------------------------------------------------------------------//
PHP_MINFO_FUNCTION(phplzf)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "phplzf module", "enabled");
    php_info_print_table_row(2, "Version", PHPLZF_VERSION);
    php_info_print_table_end();
}
