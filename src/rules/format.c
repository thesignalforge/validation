/*
 * Format validation rules
 */

#include "rules.h"
#include "src/condition.h"
#include "src/wildcard.h"
#include <arpa/inet.h>
#include <time.h>

/* Fast email validation */
static zend_bool validate_email_fast(const char *email, size_t len)
{
    if (len < 3 || len > 254) {
        return 0;
    }

    const char *at = memchr(email, '@', len);
    if (!at) {
        return 0;
    }

    size_t local_len = at - email;
    size_t domain_len = len - local_len - 1;

    /* Local part: 1-64 chars */
    if (local_len < 1 || local_len > 64) {
        return 0;
    }

    /* Domain: 1-253 chars, must contain dot */
    if (domain_len < 1 || domain_len > 253) {
        return 0;
    }

    const char *domain = at + 1;
    const char *dot = memchr(domain, '.', domain_len);
    if (!dot || dot == domain || dot == domain + domain_len - 1) {
        return 0;
    }

    return 1;
}

/* email - Valid email address */
sf_rule_result_t sf_rule_email(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.email");
        return RULE_FAIL;
    }

    if (!validate_email_fast(Z_STRVAL_P(ctx->value), Z_STRLEN_P(ctx->value))) {
        sf_add_error(ctx, "validation.email");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* url - Valid URL */
sf_rule_result_t sf_rule_url(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.url");
        return RULE_FAIL;
    }

    const char *url = Z_STRVAL_P(ctx->value);
    size_t len = Z_STRLEN_P(ctx->value);

    /* Must start with http:// or https:// */
    if (len >= 7 && strncmp(url, "http://", 7) == 0) {
        /* Valid HTTP */
    } else if (len >= 8 && strncmp(url, "https://", 8) == 0) {
        /* Valid HTTPS */
    } else {
        sf_add_error(ctx, "validation.url");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* ip - Valid IP address (v4 or v6) */
sf_rule_result_t sf_rule_ip(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.ip");
        return RULE_FAIL;
    }

    const char *ip = Z_STRVAL_P(ctx->value);
    struct in_addr addr4;
    struct in6_addr addr6;

    /* Try IPv4 first */
    if (inet_pton(AF_INET, ip, &addr4) == 1) {
        return RULE_PASS;
    }

    /* Try IPv6 */
    if (inet_pton(AF_INET6, ip, &addr6) == 1) {
        return RULE_PASS;
    }

    sf_add_error(ctx, "validation.ip");
    return RULE_FAIL;
}

/* uuid - Valid UUID */
sf_rule_result_t sf_rule_uuid(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.uuid");
        return RULE_FAIL;
    }

    const char *uuid = Z_STRVAL_P(ctx->value);
    size_t len = Z_STRLEN_P(ctx->value);

    /* UUID format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx (36 chars) */
    if (len != 36) {
        sf_add_error(ctx, "validation.uuid");
        return RULE_FAIL;
    }

    /* Check hyphens at positions 8, 13, 18, 23 */
    if (uuid[8] != '-' || uuid[13] != '-' || uuid[18] != '-' || uuid[23] != '-') {
        sf_add_error(ctx, "validation.uuid");
        return RULE_FAIL;
    }

    /* Check hex digits */
    for (size_t i = 0; i < 36; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) continue;

        char c = uuid[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            sf_add_error(ctx, "validation.uuid");
            return RULE_FAIL;
        }
    }

    return RULE_PASS;
}

/* json - Valid JSON string */
sf_rule_result_t sf_rule_json(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.json");
        return RULE_FAIL;
    }

    /* Use PHP's json_decode to validate via call_user_function */
    zval func_name, retval, params[1];
    ZVAL_STRING(&func_name, "json_decode");
    ZVAL_STRINGL(&params[0], Z_STRVAL_P(ctx->value), Z_STRLEN_P(ctx->value));

    int result = call_user_function(NULL, NULL, &func_name, &retval, 1, params);

    zval_ptr_dtor(&func_name);
    zval_ptr_dtor(&params[0]);

    if (result != SUCCESS) {
        sf_add_error(ctx, "validation.json");
        return RULE_FAIL;
    }

    /* Check json_last_error() */
    zval error_func, error_retval;
    ZVAL_STRING(&error_func, "json_last_error");
    call_user_function(NULL, NULL, &error_func, &error_retval, 0, NULL);
    zval_ptr_dtor(&error_func);

    zend_long error_code = Z_LVAL(error_retval);
    zval_ptr_dtor(&retval);

    if (error_code != 0) {  /* JSON_ERROR_NONE = 0 */
        sf_add_error(ctx, "validation.json");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* date - Valid date string */
sf_rule_result_t sf_rule_date(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.date");
        return RULE_FAIL;
    }

    /* Try common date formats */
    struct tm tm;
    const char *str = Z_STRVAL_P(ctx->value);

    if (strptime(str, "%Y-%m-%d", &tm) ||
        strptime(str, "%Y-%m-%d %H:%M:%S", &tm) ||
        strptime(str, "%Y-%m-%dT%H:%M:%S", &tm)) {
        return RULE_PASS;
    }

    sf_add_error(ctx, "validation.date");
    return RULE_FAIL;
}

/* date_format - Date must match specific format */
sf_rule_result_t sf_rule_date_format(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.date_format");
        return RULE_FAIL;
    }

    struct tm tm;
    memset(&tm, 0, sizeof(tm));

    char *result = strptime(Z_STRVAL_P(ctx->value), rule->params.string.str, &tm);
    if (!result || *result != '\0') {
        sf_add_error(ctx, "validation.date_format");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* Parse date from field value */
static zend_bool parse_date(zval *value, time_t *result)
{
    if (!value || Z_TYPE_P(value) != IS_STRING) {
        return 0;
    }

    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    const char *str = Z_STRVAL_P(value);

    if (strptime(str, "%Y-%m-%d", &tm) ||
        strptime(str, "%Y-%m-%d %H:%M:%S", &tm) ||
        strptime(str, "%Y-%m-%dT%H:%M:%S", &tm)) {
        *result = mktime(&tm);
        return 1;
    }

    return 0;
}

/* after - Date must be after another date/field */
sf_rule_result_t sf_rule_after(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    time_t current_date;
    if (!parse_date(ctx->value, &current_date)) {
        sf_add_error(ctx, "validation.after");
        return RULE_FAIL;
    }

    /* Get comparison date from field */
    zval *compare_value = sf_get_nested_value(
        rule->params.field_ref.field,
        rule->params.field_ref.len,
        ctx->data
    );

    time_t compare_date;
    if (!parse_date(compare_value, &compare_date)) {
        sf_add_error(ctx, "validation.after");
        return RULE_FAIL;
    }

    if (current_date <= compare_date) {
        sf_add_error(ctx, "validation.after");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* before - Date must be before another date/field */
sf_rule_result_t sf_rule_before(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    time_t current_date;
    if (!parse_date(ctx->value, &current_date)) {
        sf_add_error(ctx, "validation.before");
        return RULE_FAIL;
    }

    zval *compare_value = sf_get_nested_value(
        rule->params.field_ref.field,
        rule->params.field_ref.len,
        ctx->data
    );

    time_t compare_date;
    if (!parse_date(compare_value, &compare_date)) {
        sf_add_error(ctx, "validation.before");
        return RULE_FAIL;
    }

    if (current_date >= compare_date) {
        sf_add_error(ctx, "validation.before");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* after_or_equal - Date must be after or equal to another date/field */
sf_rule_result_t sf_rule_after_or_equal(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    time_t current_date;
    if (!parse_date(ctx->value, &current_date)) {
        sf_add_error(ctx, "validation.after_or_equal");
        return RULE_FAIL;
    }

    zval *compare_value = sf_get_nested_value(
        rule->params.field_ref.field,
        rule->params.field_ref.len,
        ctx->data
    );

    time_t compare_date;
    if (!parse_date(compare_value, &compare_date)) {
        sf_add_error(ctx, "validation.after_or_equal");
        return RULE_FAIL;
    }

    if (current_date < compare_date) {
        sf_add_error(ctx, "validation.after_or_equal");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* before_or_equal - Date must be before or equal to another date/field */
sf_rule_result_t sf_rule_before_or_equal(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    time_t current_date;
    if (!parse_date(ctx->value, &current_date)) {
        sf_add_error(ctx, "validation.before_or_equal");
        return RULE_FAIL;
    }

    zval *compare_value = sf_get_nested_value(
        rule->params.field_ref.field,
        rule->params.field_ref.len,
        ctx->data
    );

    time_t compare_date;
    if (!parse_date(compare_value, &compare_date)) {
        sf_add_error(ctx, "validation.before_or_equal");
        return RULE_FAIL;
    }

    if (current_date > compare_date) {
        sf_add_error(ctx, "validation.before_or_equal");
        return RULE_FAIL;
    }

    return RULE_PASS;
}
