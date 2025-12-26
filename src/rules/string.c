/*
 * String validation rules
 */

#include "rules.h"
#include "src/condition.h"
#include "src/validator.h"
#include "src/util/utf8.h"

/* Get size based on value type */
static zend_long get_size(zval *value)
{
    if (!value) return 0;

    switch (Z_TYPE_P(value)) {
        case IS_STRING:
            return (zend_long)sf_utf8_strlen(Z_STRVAL_P(value), Z_STRLEN_P(value));

        case IS_ARRAY:
            return (zend_long)zend_hash_num_elements(Z_ARRVAL_P(value));

        case IS_LONG:
            return Z_LVAL_P(value);

        case IS_DOUBLE:
            return (zend_long)Z_DVAL_P(value);

        default:
            return 0;
    }
}

/* min - Minimum size/length/value */
sf_rule_result_t sf_rule_min(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    zend_long size = get_size(ctx->value);
    if (size < rule->params.size.value) {
        HashTable params;
        zend_hash_init(&params, 2, NULL, ZVAL_PTR_DTOR, 0);

        zval min_val;
        ZVAL_LONG(&min_val, rule->params.size.value);
        zend_hash_str_add(&params, "min", 3, &min_val);

        sf_add_error_with_params(ctx, "validation.min", &params);
        zend_hash_destroy(&params);
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* max - Maximum size/length/value */
sf_rule_result_t sf_rule_max(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    zend_long size = get_size(ctx->value);
    if (size > rule->params.size.value) {
        HashTable params;
        zend_hash_init(&params, 2, NULL, ZVAL_PTR_DTOR, 0);

        zval max_val;
        ZVAL_LONG(&max_val, rule->params.size.value);
        zend_hash_str_add(&params, "max", 3, &max_val);

        sf_add_error_with_params(ctx, "validation.max", &params);
        zend_hash_destroy(&params);
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* between - Size must be between min and max */
sf_rule_result_t sf_rule_between(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    zend_long size = get_size(ctx->value);
    if (size < rule->params.range.min || size > rule->params.range.max) {
        HashTable params;
        zend_hash_init(&params, 4, NULL, ZVAL_PTR_DTOR, 0);

        zval min_val, max_val;
        ZVAL_LONG(&min_val, rule->params.range.min);
        ZVAL_LONG(&max_val, rule->params.range.max);
        zend_hash_str_add(&params, "min", 3, &min_val);
        zend_hash_str_add(&params, "max", 3, &max_val);

        sf_add_error_with_params(ctx, "validation.between", &params);
        zend_hash_destroy(&params);
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* regex - Must match regex pattern */
sf_rule_result_t sf_rule_regex(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.regex");
        return RULE_FAIL;
    }

    cached_regex_t *cached = sf_get_or_compile_regex(
        ctx->validator,
        rule->params.regex.pattern,
        rule->params.regex.len
    );

    if (!cached) {
        sf_add_error(ctx, "validation.regex");
        return RULE_FAIL;
    }

    int rc = pcre2_match(
        cached->compiled,
        (PCRE2_SPTR)Z_STRVAL_P(ctx->value),
        Z_STRLEN_P(ctx->value),
        0,
        0,
        cached->match_data,
        NULL
    );

    if (rc < 0) {
        sf_add_error(ctx, "validation.regex");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* not_regex - Must NOT match regex pattern */
sf_rule_result_t sf_rule_not_regex(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        return RULE_PASS;  /* Non-string doesn't match regex */
    }

    cached_regex_t *cached = sf_get_or_compile_regex(
        ctx->validator,
        rule->params.regex.pattern,
        rule->params.regex.len
    );

    if (!cached) {
        return RULE_PASS;  /* Invalid regex doesn't match */
    }

    int rc = pcre2_match(
        cached->compiled,
        (PCRE2_SPTR)Z_STRVAL_P(ctx->value),
        Z_STRLEN_P(ctx->value),
        0,
        0,
        cached->match_data,
        NULL
    );

    if (rc >= 0) {
        sf_add_error(ctx, "validation.not_regex");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* alpha - Only alphabetic characters */
sf_rule_result_t sf_rule_alpha(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.alpha");
        return RULE_FAIL;
    }

    const char *str = Z_STRVAL_P(ctx->value);
    size_t len = Z_STRLEN_P(ctx->value);

    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)str[i];
        /* Allow UTF-8 multi-byte characters and ASCII letters */
        if (c >= 0x80) continue;  /* UTF-8 continuation or lead byte */
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
            sf_add_error(ctx, "validation.alpha");
            return RULE_FAIL;
        }
    }

    return RULE_PASS;
}

/* alpha_num - Alphabetic and numeric characters */
sf_rule_result_t sf_rule_alpha_num(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.alpha_num");
        return RULE_FAIL;
    }

    const char *str = Z_STRVAL_P(ctx->value);
    size_t len = Z_STRLEN_P(ctx->value);

    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)str[i];
        if (c >= 0x80) continue;  /* UTF-8 */
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) {
            sf_add_error(ctx, "validation.alpha_num");
            return RULE_FAIL;
        }
    }

    return RULE_PASS;
}

/* alpha_dash - Alphabetic, numeric, dashes, underscores */
sf_rule_result_t sf_rule_alpha_dash(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.alpha_dash");
        return RULE_FAIL;
    }

    const char *str = Z_STRVAL_P(ctx->value);
    size_t len = Z_STRLEN_P(ctx->value);

    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)str[i];
        if (c >= 0x80) continue;  /* UTF-8 */
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '-' || c == '_')) {
            sf_add_error(ctx, "validation.alpha_dash");
            return RULE_FAIL;
        }
    }

    return RULE_PASS;
}

/* lowercase - Must be all lowercase */
sf_rule_result_t sf_rule_lowercase(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.lowercase");
        return RULE_FAIL;
    }

    const char *str = Z_STRVAL_P(ctx->value);
    size_t len = Z_STRLEN_P(ctx->value);

    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)str[i];
        if (c >= 0x80) continue;  /* UTF-8 */
        if (c >= 'A' && c <= 'Z') {
            sf_add_error(ctx, "validation.lowercase");
            return RULE_FAIL;
        }
    }

    return RULE_PASS;
}

/* uppercase - Must be all uppercase */
sf_rule_result_t sf_rule_uppercase(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.uppercase");
        return RULE_FAIL;
    }

    const char *str = Z_STRVAL_P(ctx->value);
    size_t len = Z_STRLEN_P(ctx->value);

    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)str[i];
        if (c >= 0x80) continue;  /* UTF-8 */
        if (c >= 'a' && c <= 'z') {
            sf_add_error(ctx, "validation.uppercase");
            return RULE_FAIL;
        }
    }

    return RULE_PASS;
}

/* starts_with - Must start with given string */
sf_rule_result_t sf_rule_starts_with(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.starts_with");
        return RULE_FAIL;
    }

    const char *str = Z_STRVAL_P(ctx->value);
    size_t len = Z_STRLEN_P(ctx->value);

    if (len < rule->params.string.len ||
        memcmp(str, rule->params.string.str, rule->params.string.len) != 0) {
        sf_add_error(ctx, "validation.starts_with");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* ends_with - Must end with given string */
sf_rule_result_t sf_rule_ends_with(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.ends_with");
        return RULE_FAIL;
    }

    const char *str = Z_STRVAL_P(ctx->value);
    size_t len = Z_STRLEN_P(ctx->value);

    if (len < rule->params.string.len ||
        memcmp(str + len - rule->params.string.len, rule->params.string.str, rule->params.string.len) != 0) {
        sf_add_error(ctx, "validation.ends_with");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* contains - Must contain given string */
sf_rule_result_t sf_rule_contains(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.contains");
        return RULE_FAIL;
    }

    const char *str = Z_STRVAL_P(ctx->value);
    size_t len = Z_STRLEN_P(ctx->value);

    if (rule->params.string.len > len) {
        sf_add_error(ctx, "validation.contains");
        return RULE_FAIL;
    }

    /* Simple substring search */
    const char *found = memmem(str, len, rule->params.string.str, rule->params.string.len);
    if (!found) {
        sf_add_error(ctx, "validation.contains");
        return RULE_FAIL;
    }

    return RULE_PASS;
}
