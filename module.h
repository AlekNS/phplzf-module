#ifndef PHP_PHPLZF_H
#define PHP_PHPLZF_H

#ifdef PHP_WIN32
#define PHP_PHPLZF_API __declspec(dllexport)
#else
#define PHP_PHPLZF_API
#endif

extern zend_module_entry phplzf_module_entry;
#define phpext_phplzf_ptr &phplzf_module_entry

#ifdef ZTS
#include <TSRM.h>
#endif

PHP_MINIT_FUNCTION(phplzf);
PHP_MSHUTDOWN_FUNCTION(phplzf);
PHP_RINIT_FUNCTION(phplzf);
PHP_RSHUTDOWN_FUNCTION(phplzf);
PHP_MINFO_FUNCTION(phplzf);

#endif
