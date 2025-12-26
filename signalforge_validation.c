/*
 * Signalforge Validation - PHP C Extension
 * Main extension entry point
 */

#include "php_signalforge_validation.h"
#include "src/validator.h"
#include "src/result.h"

/* Class entries */
zend_class_entry *signalforge_validator_ce = NULL;
zend_class_entry *signalforge_validation_result_ce = NULL;
zend_class_entry *signalforge_invalid_rule_exception_ce = NULL;

/* Thread safety */
#if defined(ZTS) && defined(COMPILE_DL_SIGNALFORGE_VALIDATION)
ZEND_TSRMLS_CACHE_DEFINE()
#endif

/* Module info */
PHP_MINFO_FUNCTION(signalforge_validation)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "signalforge_validation support", "enabled");
    php_info_print_table_row(2, "Version", PHP_SIGNALFORGE_VALIDATION_VERSION);
    php_info_print_table_row(2, "PHP Version", PHP_VERSION);
    php_info_print_table_end();
}

/* Module initialization */
PHP_MINIT_FUNCTION(signalforge_validation)
{
#if defined(ZTS) && defined(COMPILE_DL_SIGNALFORGE_VALIDATION)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif

    /* Register exception class */
    signalforge_register_exception_class();

    /* Register Validator class */
    signalforge_register_validator_class();

    /* Register ValidationResult class */
    signalforge_register_result_class();

    return SUCCESS;
}

/* Module shutdown */
PHP_MSHUTDOWN_FUNCTION(signalforge_validation)
{
    return SUCCESS;
}

/* Request initialization */
PHP_RINIT_FUNCTION(signalforge_validation)
{
#if defined(ZTS) && defined(COMPILE_DL_SIGNALFORGE_VALIDATION)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    return SUCCESS;
}

/* Request shutdown */
PHP_RSHUTDOWN_FUNCTION(signalforge_validation)
{
    return SUCCESS;
}

/* Module entry */
zend_module_entry signalforge_validation_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_SIGNALFORGE_VALIDATION_EXTNAME,
    NULL,                                   /* Functions */
    PHP_MINIT(signalforge_validation),      /* MINIT */
    PHP_MSHUTDOWN(signalforge_validation),  /* MSHUTDOWN */
    PHP_RINIT(signalforge_validation),      /* RINIT */
    PHP_RSHUTDOWN(signalforge_validation),  /* RSHUTDOWN */
    PHP_MINFO(signalforge_validation),      /* MINFO */
    PHP_SIGNALFORGE_VALIDATION_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SIGNALFORGE_VALIDATION
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(signalforge_validation)
#endif
