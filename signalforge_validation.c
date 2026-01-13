/*
 * Signalforge Validation - PHP C Extension
 * Main extension entry point
 *
 * This file implements the PHP module lifecycle functions (MINIT, MSHUTDOWN,
 * RINIT, RSHUTDOWN) and registers all classes with the Zend Engine.
 *
 * Extension Architecture:
 * - Validator: Main validation class, holds parsed rules and regex cache
 * - ValidationResult: Immutable result object with errors and validated data
 * - InvalidRuleException: Thrown when rule definitions are malformed
 *
 * Thread Safety:
 * - All global state is limited to class entries (registered once at MINIT)
 * - Per-request state is stored in object instances
 * - Regex cache is per-validator-instance, not global
 */

#include "php_signalforge_validation.h"
#include "src/validator.h"
#include "src/result.h"

/*
 * Global class entry pointers.
 *
 * These are populated once during MINIT and remain constant for the lifetime
 * of the PHP process. They point to the Zend class structures registered
 * with the engine.
 */
zend_class_entry *signalforge_validator_ce = NULL;
zend_class_entry *signalforge_validation_result_ce = NULL;
zend_class_entry *signalforge_invalid_rule_exception_ce = NULL;

/* Thread safety for dynamically loaded module */
#if defined(ZTS) && defined(COMPILE_DL_SIGNALFORGE_VALIDATION)
ZEND_TSRMLS_CACHE_DEFINE()
#endif

/*
 * Module information displayed in phpinfo() output.
 *
 * Shows version info and compile-time configuration for debugging.
 */
PHP_MINFO_FUNCTION(signalforge_validation)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "signalforge_validation support", "enabled");
    php_info_print_table_row(2, "Version", PHP_SIGNALFORGE_VALIDATION_VERSION);
    php_info_print_table_row(2, "PHP Version", PHP_VERSION);
#ifdef ZTS
    php_info_print_table_row(2, "Thread Safety", "enabled");
#else
    php_info_print_table_row(2, "Thread Safety", "disabled");
#endif
    php_info_print_table_end();

    php_info_print_table_start();
    php_info_print_table_header(2, "Supported Rules", "");
    php_info_print_table_row(2, "Presence", "required, nullable, filled, present");
    php_info_print_table_row(2, "Types", "string, integer, numeric, boolean, array");
    php_info_print_table_row(2, "String", "min, max, between, regex, alpha, alpha_num, alpha_dash");
    php_info_print_table_row(2, "Comparison", "gt, gte, lt, lte, in, not_in, same, different, confirmed");
    php_info_print_table_row(2, "Format", "email, url, ip, uuid, json, date, date_format");
    php_info_print_table_row(2, "Regional", "oib, phone, iban, vat_eu");
    php_info_print_table_row(2, "Conditional", "when");
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
ZEND_GET_MODULE(signalforge_validation)
#endif
