/*
 * Signalforge Validation - PHP C Extension
 * High-performance input validation for PHP 8.2+
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

#define PHP_SIGNALFORGE_VALIDATION_VERSION "1.0.0"
#define PHP_SIGNALFORGE_VALIDATION_EXTNAME "signalforge_validation"

#define RULE_NAME_MAX_LENGTH 1024

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
