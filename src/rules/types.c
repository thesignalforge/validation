/*
 * Type validation rules
 */

#include "rules.h"
#include "src/condition.h"

/* string - Must be a string */
sf_rule_result_t sf_rule_string(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.string");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* integer - Must be an integer */
sf_rule_result_t sf_rule_integer(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value) {
        sf_add_error(ctx, "validation.integer");
        return RULE_FAIL;
    }

    switch (Z_TYPE_P(ctx->value)) {
        case IS_LONG:
            return RULE_PASS;

        case IS_STRING: {
            /* Check if string is a valid integer */
            char *endptr;
            const char *str = Z_STRVAL_P(ctx->value);
            size_t len = Z_STRLEN_P(ctx->value);

            if (len == 0) {
                sf_add_error(ctx, "validation.integer");
                return RULE_FAIL;
            }

            /* Skip leading whitespace */
            while (*str == ' ' || *str == '\t') {
                str++;
            }

            strtol(str, &endptr, 10);

            /* Skip trailing whitespace */
            while (*endptr == ' ' || *endptr == '\t') {
                endptr++;
            }

            if (*endptr == '\0') {
                return RULE_PASS;
            }
            break;
        }

        default:
            break;
    }

    sf_add_error(ctx, "validation.integer");
    return RULE_FAIL;
}

/* numeric - Must be numeric (int, float, or numeric string) */
sf_rule_result_t sf_rule_numeric(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value) {
        sf_add_error(ctx, "validation.numeric");
        return RULE_FAIL;
    }

    switch (Z_TYPE_P(ctx->value)) {
        case IS_LONG:
        case IS_DOUBLE:
            return RULE_PASS;

        case IS_STRING: {
            /* Check if string is numeric */
            const char *str = Z_STRVAL_P(ctx->value);
            size_t len = Z_STRLEN_P(ctx->value);

            if (len == 0) {
                sf_add_error(ctx, "validation.numeric");
                return RULE_FAIL;
            }

            /* Use PHP's is_numeric_string */
            zend_long lval;
            double dval;
            if (is_numeric_string(str, len, &lval, &dval, 0) != 0) {
                return RULE_PASS;
            }
            break;
        }

        default:
            break;
    }

    sf_add_error(ctx, "validation.numeric");
    return RULE_FAIL;
}

/* boolean - Must be boolean or boolean-like */
sf_rule_result_t sf_rule_boolean(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value) {
        sf_add_error(ctx, "validation.boolean");
        return RULE_FAIL;
    }

    switch (Z_TYPE_P(ctx->value)) {
        case IS_TRUE:
        case IS_FALSE:
            return RULE_PASS;

        case IS_LONG:
            if (Z_LVAL_P(ctx->value) == 0 || Z_LVAL_P(ctx->value) == 1) {
                return RULE_PASS;
            }
            break;

        case IS_STRING: {
            const char *str = Z_STRVAL_P(ctx->value);
            size_t len = Z_STRLEN_P(ctx->value);

            /* Accept "0", "1", "true", "false" */
            if (len == 1 && (str[0] == '0' || str[0] == '1')) {
                return RULE_PASS;
            }
            if (len == 4 && strncasecmp(str, "true", 4) == 0) {
                return RULE_PASS;
            }
            if (len == 5 && strncasecmp(str, "false", 5) == 0) {
                return RULE_PASS;
            }
            break;
        }

        default:
            break;
    }

    sf_add_error(ctx, "validation.boolean");
    return RULE_FAIL;
}

/* array - Must be an array */
sf_rule_result_t sf_rule_array(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_ARRAY) {
        sf_add_error(ctx, "validation.array");
        return RULE_FAIL;
    }

    return RULE_PASS;
}
