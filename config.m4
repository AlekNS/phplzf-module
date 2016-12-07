PHP_ARG_ENABLE(phplzf, for simply propriate your sources,
[  --disable-phplzf   Disable phplzf support], yes)

if test "$PHP_PHAR" != "no"; then
  PHP_NEW_EXTENSION(phplzf, crc32.c \
    module.c \
    decompress.c \
    compress.c \
    lzf_c.c \
    lzf_d.c \
    phplzf.c, $ext_shared)
  AC_MSG_CHECKING([for phar openssl support])
  PHP_ADD_MAKEFILE_FRAGMENT
fi
