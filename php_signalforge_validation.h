/*
 * Signalforge Validation - PHP C Extension
 * High-performance input validation for PHP 8.2+
 *
 * This extension provides a Laravel-compatible validation API implemented in C
 * for maximum performance. It supports:
 * - Type validation (string, integer, numeric, boolean, array)
 * - String validation (min, max, regex, alpha, email, url, etc.)
 * - Numeric comparison (gt, gte, lt, lte)
 * - Array validation (distinct, wildcard patterns)
 * - Conditional validation (when clauses)
 * - Regional formats (OIB, IBAN, EU VAT)
 *
 * Thread Safety:
 * - Fully ZTS-compatible using proper TSRM mechanisms
 * - Per-request regex caching stored in validator objects
 *
 * Memory Management:
 * - All allocations use Zend Memory Manager (emalloc/efree)
 * - Proper cleanup in all error paths
 * - Reference counting for zvals follows PHP conventions
 *
 * Copyright (c) Signalforge
 * Licensed under the MIT License
 */

#ifndef PHP_SIGNALFORGE_VALIDATION_H
#define PHP_SIGNALFORGE_VALIDATION_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/pcre/php_pcre.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

/*
 * Extension metadata constants
 */
#define PHP_SIGNALFORGE_VALIDATION_VERSION "1.0.0"
#define PHP_SIGNALFORGE_VALIDATION_EXTNAME "signalforge_validation"

/*
 * Validation limits - these prevent denial-of-service via resource exhaustion
 */
#define SF_RULE_NAME_MAX_LENGTH    1024   /* Maximum length of a rule name */
#define SF_FIELD_NAME_MAX_LENGTH   4096   /* Maximum length of a field path */
#define SF_EMAIL_MIN_LENGTH        3      /* Minimum valid email: a@b */
#define SF_EMAIL_MAX_LENGTH        254    /* RFC 5321 limit */
#define SF_EMAIL_LOCAL_MAX_LENGTH  64     /* RFC 5321 local part limit */
#define SF_EMAIL_DOMAIN_MAX_LENGTH 253    /* RFC 5321 domain limit */
#define SF_PHONE_MIN_DIGITS        7      /* Minimum phone digits */
#define SF_PHONE_MAX_LENGTH        20     /* Maximum phone string length */
#define SF_UUID_LENGTH             36     /* UUID string length */
#define SF_OIB_LENGTH              11     /* Croatian OIB length */
#define SF_IBAN_MIN_LENGTH         15     /* Minimum IBAN length */
#define SF_IBAN_MAX_LENGTH         34     /* Maximum IBAN length */
#define SF_VAT_EU_MIN_LENGTH       4      /* Minimum EU VAT length */
#define SF_VAT_EU_MAX_LENGTH       14     /* Maximum EU VAT length */
#define SF_REGEX_CACHE_INITIAL     8      /* Initial regex cache size */
#define SF_HASH_INITIAL_SIZE       8      /* Default hashtable initial size */

/* Backward compatibility alias */
#define RULE_NAME_MAX_LENGTH SF_RULE_NAME_MAX_LENGTH

extern zend_module_entry signalforge_validation_module_entry;
#define phpext_signalforge_validation_ptr &signalforge_validation_module_entry

/* Class entries */
extern zend_class_entry *signalforge_validator_ce;
extern zend_class_entry *signalforge_validation_result_ce;
extern zend_class_entry *signalforge_invalid_rule_exception_ce;

/* Object handlers */
extern zend_object_handlers signalforge_validator_handlers;
extern zend_object_handlers signalforge_validation_result_handlers;

/* Cached regex structure */
typedef struct {
    pcre2_code *compiled;
    pcre2_match_data *match_data;
} cached_regex_t;

/* Validator object */
typedef struct {
    HashTable *rules;           /* Parsed rules */
    HashTable *regex_cache;     /* Compiled regex patterns */
    zend_object std;
} signalforge_validator_t;

/* Result object */
typedef struct {
    zend_bool is_valid;
    HashTable *errors;
    HashTable *validated;
    zend_object std;
} signalforge_validation_result_t;

/* Helper macros to get object from zend_object */
static inline signalforge_validator_t *signalforge_validator_from_obj(zend_object *obj) {
    return (signalforge_validator_t *)((char *)(obj) - XtOffsetOf(signalforge_validator_t, std));
}

static inline signalforge_validation_result_t *signalforge_validation_result_from_obj(zend_object *obj) {
    return (signalforge_validation_result_t *)((char *)(obj) - XtOffsetOf(signalforge_validation_result_t, std));
}

#define Z_SIGNALFORGE_VALIDATOR_P(zv) signalforge_validator_from_obj(Z_OBJ_P(zv))
#define Z_SIGNALFORGE_VALIDATION_RESULT_P(zv) signalforge_validation_result_from_obj(Z_OBJ_P(zv))

/* Function declarations */
void signalforge_register_validator_class(void);
void signalforge_register_result_class(void);
void signalforge_register_exception_class(void);

/* Thread safety */
#ifdef ZTS
#include "TSRM.h"
#endif

#if defined(ZTS) && defined(COMPILE_DL_SIGNALFORGE_VALIDATION)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif /* PHP_SIGNALFORGE_VALIDATION_H */
