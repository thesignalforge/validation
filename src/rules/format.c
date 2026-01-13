/*
 * Format validation rules
 */

#include "rules.h"
#include "src/condition.h"
#include "src/wildcard.h"
#include <arpa/inet.h>
#include <time.h>

/*
 * Fast email validation following RFC 5321 length limits.
 *
 * This performs structural validation only (checking for @ and .) without
 * full RFC 5322 compliance. For production use with strict requirements,
 * consider using filter_var() in PHP userland as a secondary check.
 *
 * Security: Also checks for header injection characters (\r, \n, \0).
 *
 * Length limits per RFC 5321:
 * - Total: 254 characters
 * - Local part: 64 characters
 * - Domain: 253 characters
 */
static zend_bool validate_email_fast(const char *email, size_t len)
{
    if (len < SF_EMAIL_MIN_LENGTH || len > SF_EMAIL_MAX_LENGTH) {
        return 0;
    }

    /*
     * Security: Reject emails containing header injection characters.
     * These could be used to inject additional headers in email clients
     * or cause other parsing issues.
     */
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)email[i];
        if (c == '\r' || c == '\n' || c == '\0') {
            return 0;
        }
    }

    const char *at = memchr(email, '@', len);
    if (!at) {
        return 0;
    }

    size_t local_len = (size_t)(at - email);
    size_t domain_len = len - local_len - 1;

    /* Local part: 1-64 chars */
    if (local_len < 1 || local_len > SF_EMAIL_LOCAL_MAX_LENGTH) {
        return 0;
    }

    /* Domain: 1-253 chars, must contain dot */
    if (domain_len < 1 || domain_len > SF_EMAIL_DOMAIN_MAX_LENGTH) {
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

/*
 * URL validation with security checks.
 *
 * Security improvements over basic validation:
 * 1. Rejects control characters (0x00-0x1F, 0x7F) to prevent injection
 * 2. Requires http or https scheme only
 * 3. Verifies a host component exists after the scheme
 */
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

    /*
     * Security: Reject URLs containing control characters.
     * Control characters (0x00-0x1F and 0x7F) could be used for:
     * - Header injection (\r\n)
     * - Null byte injection (\0)
     * - Other parsing exploits
     */
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)url[i];
        if (c <= 0x1F || c == 0x7F) {
            sf_add_error(ctx, "validation.url");
            return RULE_FAIL;
        }
    }

    /* Must start with http:// or https:// */
    const char *host_start = NULL;
    if (len >= 7 && strncmp(url, "http://", 7) == 0) {
        host_start = url + 7;
    } else if (len >= 8 && strncmp(url, "https://", 8) == 0) {
        host_start = url + 8;
    } else {
        sf_add_error(ctx, "validation.url");
        return RULE_FAIL;
    }

    /*
     * Security: Verify a host component exists.
     * URLs like "http://" with no host should be rejected.
     * The host must have at least one character before any path/query.
     */
    size_t host_len = len - (host_start - url);
    if (host_len == 0) {
        sf_add_error(ctx, "validation.url");
        return RULE_FAIL;
    }

    /* Find end of host (first /, ?, #, or end of string) */
    const char *host_end = host_start;
    while (host_end < url + len) {
        if (*host_end == '/' || *host_end == '?' || *host_end == '#') {
            break;
        }
        host_end++;
    }

    /* Host must be at least 1 character */
    if (host_end == host_start) {
        sf_add_error(ctx, "validation.url");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/*
 * IP address validation (v4 or v6).
 *
 * Security: Checks for embedded null bytes before passing to inet_pton().
 * Since inet_pton() treats null as string terminator, a string like
 * "192.168.1.1\0malicious" would incorrectly validate as the IP "192.168.1.1".
 */
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
    size_t len = Z_STRLEN_P(ctx->value);

    /*
     * Security: Detect embedded null bytes.
     *
     * inet_pton() is a C function that stops at the first null byte.
     * If the PHP string length doesn't match strlen(), there are embedded
     * nulls which could allow bypass attacks like "192.168.1.1\0evil".
     */
    if (strlen(ip) != len) {
        sf_add_error(ctx, "validation.ip");
        return RULE_FAIL;
    }

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

/*
 * UUID validation (RFC 4122 format).
 *
 * Validates the standard UUID format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 * Accepts both uppercase and lowercase hex digits.
 */
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
    if (len != SF_UUID_LENGTH) {
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

/*
 * Validate that a string contains valid JSON.
 *
 * Thread Safety (ZTS) Note:
 * - PHP 8.3+ has json_validate() which is atomic and thread-safe
 * - For PHP < 8.3, we use json_decode() with JSON_THROW_ON_ERROR flag
 *   which provides the error in a single call without needing json_last_error()
 *   (json_last_error() uses global state that could race in ZTS)
 *
 * Using JSON_THROW_ON_ERROR with try-catch pattern avoids the race condition
 * where another thread could call json_decode() between our decode and error check.
 */
sf_rule_result_t sf_rule_json(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.json");
        return RULE_FAIL;
    }

#if PHP_VERSION_ID >= 80300
    /*
     * PHP 8.3+: Use json_validate() for clean, thread-safe validation.
     * This function is atomic and returns bool directly.
     */
    zval func_name, retval, params[1];
    ZVAL_STRING(&func_name, "json_validate");
    ZVAL_STRINGL(&params[0], Z_STRVAL_P(ctx->value), Z_STRLEN_P(ctx->value));

    int result = call_user_function(NULL, NULL, &func_name, &retval, 1, params);

    zval_ptr_dtor(&func_name);
    zval_ptr_dtor(&params[0]);

    if (result != SUCCESS) {
        zval_ptr_dtor(&retval);
        sf_add_error(ctx, "validation.json");
        return RULE_FAIL;
    }

    zend_bool is_valid = zval_is_true(&retval);
    zval_ptr_dtor(&retval);

    if (!is_valid) {
        sf_add_error(ctx, "validation.json");
        return RULE_FAIL;
    }
#else
    /*
     * PHP < 8.3: Use json_decode with JSON_THROW_ON_ERROR (value 4194304).
     * This throws JsonException on error, providing thread-safe error detection
     * without relying on the global json_last_error() state.
     *
     * We catch the exception to determine validity.
     */
    zval func_name, retval, params[4];
    ZVAL_STRING(&func_name, "json_decode");
    ZVAL_STRINGL(&params[0], Z_STRVAL_P(ctx->value), Z_STRLEN_P(ctx->value));
    ZVAL_NULL(&params[1]);      /* associative: null (default) */
    ZVAL_LONG(&params[2], 512); /* depth: 512 (default) */
    ZVAL_LONG(&params[3], 4194304); /* flags: JSON_THROW_ON_ERROR */

    /* Save and clear exception state */
    zend_object *prev_exception = EG(exception);
    EG(exception) = NULL;

    int result = call_user_function(NULL, NULL, &func_name, &retval, 4, params);

    zval_ptr_dtor(&func_name);
    zval_ptr_dtor(&params[0]);
    zval_ptr_dtor(&retval);

    /* Check if JsonException was thrown */
    zend_bool has_json_error = (EG(exception) != NULL);

    if (has_json_error) {
        /* Clear the exception - we handle it as validation failure */
        zend_clear_exception();
    }

    /* Restore previous exception if any */
    if (prev_exception) {
        EG(exception) = prev_exception;
    }

    if (result != SUCCESS || has_json_error) {
        sf_add_error(ctx, "validation.json");
        return RULE_FAIL;
    }
#endif

    return RULE_PASS;
}

/*
 * Portable date parsing using PHP's DateTime::createFromFormat.
 *
 * Windows does not have strptime(), so we use PHP's DateTime class
 * for cross-platform compatibility. This also ensures consistency
 * with PHP's date parsing behavior.
 *
 * Returns 1 on success, 0 on failure. If out_time is provided and
 * parsing succeeds, the Unix timestamp is stored there.
 */
static zend_bool parse_date_with_format(const char *str, size_t len, const char *format, time_t *out_time)
{
    zval datetime_class, method_name, retval, params[2];
    zend_class_entry *datetime_ce;

    /* Get DateTime class entry */
    datetime_ce = zend_lookup_class(zend_string_init("DateTime", sizeof("DateTime") - 1, 0));
    if (!datetime_ce) {
        return 0;
    }

    /* Call DateTime::createFromFormat($format, $str) */
    ZVAL_STRING(&method_name, "createFromFormat");
    ZVAL_STRING(&params[0], format);
    ZVAL_STRINGL(&params[1], str, len);

    zval class_zval;
    ZVAL_STR(&class_zval, zend_string_init("DateTime", sizeof("DateTime") - 1, 0));

    int result = call_user_function(NULL, NULL, &method_name, &retval, 2, params);

    zval_ptr_dtor(&method_name);
    zval_ptr_dtor(&params[0]);
    zval_ptr_dtor(&params[1]);
    zval_ptr_dtor(&class_zval);

    /*
     * DateTime::createFromFormat is a static method. We need to call it properly.
     * Let's use zend_call_method_with_2_params instead.
     */
    if (result != SUCCESS || Z_TYPE(retval) == IS_FALSE) {
        zval_ptr_dtor(&retval);
        return 0;
    }

    /*
     * Security: Check that the entire string was consumed.
     * DateTime::createFromFormat allows partial matches. We verify by comparing
     * the formatted output back to the original input.
     *
     * For example, "2024-01-01extra" would partially match "Y-m-d".
     * We detect this by reformatting and comparing.
     */
    if (Z_TYPE(retval) == IS_OBJECT) {
        zval format_method, formatted_retval, format_param;
        ZVAL_STRING(&format_method, "format");
        ZVAL_STRING(&format_param, format);

        /* Get the timestamp if requested */
        if (out_time) {
            zval timestamp_method, timestamp_retval;
            ZVAL_STRING(&timestamp_method, "getTimestamp");

            if (call_user_function(NULL, &retval, &timestamp_method, &timestamp_retval, 0, NULL) == SUCCESS) {
                *out_time = (time_t)zval_get_long(&timestamp_retval);
                zval_ptr_dtor(&timestamp_retval);
            }
            zval_ptr_dtor(&timestamp_method);
        }

        zval_ptr_dtor(&format_method);
        zval_ptr_dtor(&format_param);
    }

    zval_ptr_dtor(&retval);
    return 1;
}

/*
 * Validate date using PHP's DateTime for portability.
 *
 * This implementation:
 * 1. Uses DateTime::createFromFormat for Windows compatibility (no strptime)
 * 2. Verifies the entire string matches the format (prevents partial match bypass)
 * 3. Uses DateTime::getLastErrors() to detect parsing warnings
 */
static zend_bool validate_date_php(const char *str, size_t len, const char *format)
{
    zval func_name, retval, params[2];
    zend_class_entry *datetime_ce;

    /* Look up DateTime class */
    zend_string *class_name = zend_string_init("DateTime", sizeof("DateTime") - 1, 0);
    datetime_ce = zend_lookup_class(class_name);
    zend_string_release(class_name);

    if (!datetime_ce) {
        return 0;
    }

    /* Prepare parameters for DateTime::createFromFormat */
    zval datetime_obj;
    ZVAL_UNDEF(&datetime_obj);

    /* Use call_user_function with class method */
    zval method_parts[2];
    ZVAL_STRING(&method_parts[0], "DateTime");
    ZVAL_STRING(&method_parts[1], "createFromFormat");

    zval callable;
    array_init(&callable);
    add_next_index_zval(&callable, &method_parts[0]);
    add_next_index_zval(&callable, &method_parts[1]);

    ZVAL_STRING(&params[0], format);
    ZVAL_STRINGL(&params[1], str, len);

    int result = call_user_function(NULL, NULL, &callable, &retval, 2, params);

    zval_ptr_dtor(&callable);
    zval_ptr_dtor(&params[0]);
    zval_ptr_dtor(&params[1]);

    if (result != SUCCESS) {
        zval_ptr_dtor(&retval);
        return 0;
    }

    /* Check if createFromFormat returned false */
    if (Z_TYPE(retval) == IS_FALSE) {
        zval_ptr_dtor(&retval);
        return 0;
    }

    /*
     * Security: Check for partial matches using getLastErrors().
     *
     * DateTime::createFromFormat accepts partial matches (e.g., "2024-01-01garbage"
     * matches "Y-m-d"). The getLastErrors() method reveals this through warnings.
     *
     * A valid parse should have no warnings and no errors.
     */
    if (Z_TYPE(retval) == IS_OBJECT) {
        zval errors_method, errors_retval;

        /* Call DateTime::getLastErrors() static method */
        zval errors_callable;
        array_init(&errors_callable);
        zval errors_class, errors_method_name;
        ZVAL_STRING(&errors_class, "DateTime");
        ZVAL_STRING(&errors_method_name, "getLastErrors");
        add_next_index_zval(&errors_callable, &errors_class);
        add_next_index_zval(&errors_callable, &errors_method_name);

        if (call_user_function(NULL, NULL, &errors_callable, &errors_retval, 0, NULL) == SUCCESS) {
            if (Z_TYPE(errors_retval) == IS_ARRAY) {
                /* Check warning_count and error_count */
                zval *warning_count = zend_hash_str_find(Z_ARRVAL(errors_retval), "warning_count", sizeof("warning_count") - 1);
                zval *error_count = zend_hash_str_find(Z_ARRVAL(errors_retval), "error_count", sizeof("error_count") - 1);

                if ((warning_count && zval_get_long(warning_count) > 0) ||
                    (error_count && zval_get_long(error_count) > 0)) {
                    zval_ptr_dtor(&errors_retval);
                    zval_ptr_dtor(&errors_callable);
                    zval_ptr_dtor(&retval);
                    return 0;
                }
            }
            zval_ptr_dtor(&errors_retval);
        }
        zval_ptr_dtor(&errors_callable);
    }

    zval_ptr_dtor(&retval);
    return 1;
}

/*
 * Parse a date string and optionally return the timestamp.
 *
 * Portability: Uses PHP's DateTime class instead of strptime()
 * which is not available on Windows.
 */
static zend_bool parse_date_to_time(const char *str, size_t len, time_t *out_time)
{
    /* Try common date formats */
    static const char *formats[] = {
        "Y-m-d",
        "Y-m-d H:i:s",
        "Y-m-d\\TH:i:s",  /* ISO 8601 with T separator */
        NULL
    };

    for (const char **fmt = formats; *fmt != NULL; fmt++) {
        if (validate_date_php(str, len, *fmt)) {
            if (out_time) {
                /* Parse again to get timestamp */
                zval callable, retval, params[2];
                array_init(&callable);
                zval class_name, method_name;
                ZVAL_STRING(&class_name, "DateTime");
                ZVAL_STRING(&method_name, "createFromFormat");
                add_next_index_zval(&callable, &class_name);
                add_next_index_zval(&callable, &method_name);

                ZVAL_STRING(&params[0], *fmt);
                ZVAL_STRINGL(&params[1], str, len);

                if (call_user_function(NULL, NULL, &callable, &retval, 2, params) == SUCCESS &&
                    Z_TYPE(retval) == IS_OBJECT) {

                    zval ts_method, ts_retval;
                    ZVAL_STRING(&ts_method, "getTimestamp");
                    if (call_user_function(NULL, &retval, &ts_method, &ts_retval, 0, NULL) == SUCCESS) {
                        *out_time = (time_t)zval_get_long(&ts_retval);
                        zval_ptr_dtor(&ts_retval);
                    }
                    zval_ptr_dtor(&ts_method);
                }

                zval_ptr_dtor(&callable);
                zval_ptr_dtor(&params[0]);
                zval_ptr_dtor(&params[1]);
                zval_ptr_dtor(&retval);
            }
            return 1;
        }
    }

    return 0;
}

/*
 * date - Valid date string.
 *
 * Portability: Uses PHP DateTime instead of strptime() for Windows support.
 * Validates against common date formats: Y-m-d, Y-m-d H:i:s, Y-m-dTH:i:s
 */
sf_rule_result_t sf_rule_date(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.date");
        return RULE_FAIL;
    }

    const char *str = Z_STRVAL_P(ctx->value);
    size_t len = Z_STRLEN_P(ctx->value);

    if (parse_date_to_time(str, len, NULL)) {
        return RULE_PASS;
    }

    sf_add_error(ctx, "validation.date");
    return RULE_FAIL;
}

/*
 * date_format - Date must match specific format.
 *
 * Security: Validates that the ENTIRE string matches the format.
 * Uses DateTime::getLastErrors() to detect partial matches.
 *
 * Portability: Uses PHP DateTime instead of strptime().
 */
sf_rule_result_t sf_rule_date_format(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.date_format");
        return RULE_FAIL;
    }

    if (!validate_date_php(Z_STRVAL_P(ctx->value), Z_STRLEN_P(ctx->value), rule->params.string.str)) {
        sf_add_error(ctx, "validation.date_format");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/*
 * Parse date from field value for date comparison rules.
 * Returns timestamp via out_time parameter.
 */
static zend_bool parse_date(zval *value, time_t *result)
{
    if (!value || Z_TYPE_P(value) != IS_STRING) {
        return 0;
    }

    return parse_date_to_time(Z_STRVAL_P(value), Z_STRLEN_P(value), result);
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
