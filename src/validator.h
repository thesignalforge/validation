/*
 * Validator class
 */

#ifndef SIGNALFORGE_VALIDATOR_H
#define SIGNALFORGE_VALIDATOR_H

#include "php_signalforge_validation.h"

/* Register Validator class */
void signalforge_register_validator_class(void);

/* Register exception class */
void signalforge_register_exception_class(void);

/* Get or compile a regex pattern */
cached_regex_t *sf_get_or_compile_regex(
    signalforge_validator_t *validator,
    const char *pattern,
    size_t pattern_len
);

#endif /* SIGNALFORGE_VALIDATOR_H */
