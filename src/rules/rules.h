/*
 * Validation rules
 */

#ifndef SIGNALFORGE_RULES_H
#define SIGNALFORGE_RULES_H

#include "php_signalforge_validation.h"
#include "src/parser.h"

/* Validation context passed to rule functions */
typedef struct {
    signalforge_validator_t *validator;
    HashTable *data;             /* All input data */
    const char *field_name;      /* Current field being validated */
    size_t field_len;
    zval *value;                 /* Current field value */
    HashTable *errors;           /* Errors hashtable to populate */
    zend_bool has_nullable;      /* Whether nullable rule is present */
    zend_bool is_null_or_empty;  /* Whether value is null or empty */
    zend_bool bail;              /* Stop on first error */
} sf_validation_context_t;

/* Rule validation result */
typedef enum {
    RULE_PASS,           /* Validation passed */
    RULE_FAIL,           /* Validation failed */
    RULE_SKIP,           /* Skip remaining rules (e.g., nullable with null value) */
} sf_rule_result_t;

/* Add an error to the context */
void sf_add_error(sf_validation_context_t *ctx, const char *key, ...);

/* Add an error with params hashtable */
void sf_add_error_with_params(sf_validation_context_t *ctx, const char *key, HashTable *params);

/* Presence rules */
sf_rule_result_t sf_rule_required(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_nullable(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_filled(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_present(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);

/* Type rules */
sf_rule_result_t sf_rule_string(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_integer(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_numeric(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_boolean(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_array(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);

/* String rules */
sf_rule_result_t sf_rule_min(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_max(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_between(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_regex(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_not_regex(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_alpha(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_alpha_num(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_alpha_dash(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_lowercase(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_uppercase(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_starts_with(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_ends_with(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_contains(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);

/* Numeric rules */
sf_rule_result_t sf_rule_gt(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_gte(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_lt(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_lte(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);

/* Array rules */
sf_rule_result_t sf_rule_distinct(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);

/* Format rules */
sf_rule_result_t sf_rule_email(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_url(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_ip(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_uuid(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_json(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_date(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_date_format(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_after(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_before(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_after_or_equal(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_before_or_equal(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);

/* Comparison rules */
sf_rule_result_t sf_rule_in(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_not_in(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_same(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_different(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_confirmed(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);

/* Regional rules */
sf_rule_result_t sf_rule_oib(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_phone(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_iban(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);
sf_rule_result_t sf_rule_vat_eu(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);

/* Dispatch function - execute a rule by type */
sf_rule_result_t sf_execute_rule(sf_validation_context_t *ctx, sf_parsed_rule_t *rule);

#endif /* SIGNALFORGE_RULES_H */
