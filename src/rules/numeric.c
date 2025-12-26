/*
 * Numeric validation rules
 */

#include "rules.h"
#include "src/condition.h"

/* Get numeric value from zval */
static zend_bool get_numeric_value(zval *value, double *result)
{
    if (!value) return 0;

    switch (Z_TYPE_P(value)) {
        case IS_LONG:
            *result = (double)Z_LVAL_P(value);
            return 1;

        case IS_DOUBLE:
            *result = Z_DVAL_P(value);
            return 1;

        case IS_STRING: {
            zend_long lval;
            double dval;
            int type = is_numeric_string(Z_STRVAL_P(value), Z_STRLEN_P(value), &lval, &dval, 0);
            if (type == IS_LONG) {
                *result = (double)lval;
                return 1;
            } else if (type == IS_DOUBLE) {
                *result = dval;
                return 1;
            }
            return 0;
        }

        default:
            return 0;
    }
}

/* gt - Greater than */
sf_rule_result_t sf_rule_gt(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    double val;
    if (!get_numeric_value(ctx->value, &val)) {
        sf_add_error(ctx, "validation.gt");
        return RULE_FAIL;
    }

    if (val <= (double)rule->params.size.value) {
        HashTable params;
        zend_hash_init(&params, 2, NULL, ZVAL_PTR_DTOR, 0);

        zval limit;
        ZVAL_LONG(&limit, rule->params.size.value);
        zend_hash_str_add(&params, "value", 5, &limit);

        sf_add_error_with_params(ctx, "validation.gt", &params);
        zend_hash_destroy(&params);
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* gte - Greater than or equal */
sf_rule_result_t sf_rule_gte(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    double val;
    if (!get_numeric_value(ctx->value, &val)) {
        sf_add_error(ctx, "validation.gte");
        return RULE_FAIL;
    }

    if (val < (double)rule->params.size.value) {
        HashTable params;
        zend_hash_init(&params, 2, NULL, ZVAL_PTR_DTOR, 0);

        zval limit;
        ZVAL_LONG(&limit, rule->params.size.value);
        zend_hash_str_add(&params, "value", 5, &limit);

        sf_add_error_with_params(ctx, "validation.gte", &params);
        zend_hash_destroy(&params);
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* lt - Less than */
sf_rule_result_t sf_rule_lt(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    double val;
    if (!get_numeric_value(ctx->value, &val)) {
        sf_add_error(ctx, "validation.lt");
        return RULE_FAIL;
    }

    if (val >= (double)rule->params.size.value) {
        HashTable params;
        zend_hash_init(&params, 2, NULL, ZVAL_PTR_DTOR, 0);

        zval limit;
        ZVAL_LONG(&limit, rule->params.size.value);
        zend_hash_str_add(&params, "value", 5, &limit);

        sf_add_error_with_params(ctx, "validation.lt", &params);
        zend_hash_destroy(&params);
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* lte - Less than or equal */
sf_rule_result_t sf_rule_lte(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    double val;
    if (!get_numeric_value(ctx->value, &val)) {
        sf_add_error(ctx, "validation.lte");
        return RULE_FAIL;
    }

    if (val > (double)rule->params.size.value) {
        HashTable params;
        zend_hash_init(&params, 2, NULL, ZVAL_PTR_DTOR, 0);

        zval limit;
        ZVAL_LONG(&limit, rule->params.size.value);
        zend_hash_str_add(&params, "value", 5, &limit);

        sf_add_error_with_params(ctx, "validation.lte", &params);
        zend_hash_destroy(&params);
        return RULE_FAIL;
    }

    return RULE_PASS;
}
