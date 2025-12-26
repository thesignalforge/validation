/*
 * Comparison validation rules
 */

#include "rules.h"
#include "src/condition.h"
#include "src/wildcard.h"

/* Compare two zvals for equality */
static zend_bool values_equal(zval *a, zval *b)
{
    if (!a || !b) {
        return (!a && !b);
    }

    zval result;
    if (compare_function(&result, a, b) == SUCCESS) {
        return Z_LVAL(result) == 0;
    }
    return 0;
}

/* in - Value must be in a list */
sf_rule_result_t sf_rule_in(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value) {
        sf_add_error(ctx, "validation.in");
        return RULE_FAIL;
    }

    HashTable *values = rule->params.in_list.values;
    zval *item;
    zend_bool found = 0;

    ZEND_HASH_FOREACH_VAL(values, item) {
        if (values_equal(ctx->value, item)) {
            found = 1;
            break;
        }
    } ZEND_HASH_FOREACH_END();

    if (!found) {
        sf_add_error(ctx, "validation.in");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* not_in - Value must NOT be in a list */
sf_rule_result_t sf_rule_not_in(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value) {
        return RULE_PASS;  /* null is not in any list */
    }

    HashTable *values = rule->params.in_list.values;
    zval *item;

    ZEND_HASH_FOREACH_VAL(values, item) {
        if (values_equal(ctx->value, item)) {
            sf_add_error(ctx, "validation.not_in");
            return RULE_FAIL;
        }
    } ZEND_HASH_FOREACH_END();

    return RULE_PASS;
}

/* same - Value must match another field's value */
sf_rule_result_t sf_rule_same(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    zval *other_value = sf_get_nested_value(
        rule->params.field_ref.field,
        rule->params.field_ref.len,
        ctx->data
    );

    if (!values_equal(ctx->value, other_value)) {
        HashTable params;
        zend_hash_init(&params, 2, NULL, ZVAL_PTR_DTOR, 0);

        zval other_field;
        ZVAL_STRINGL(&other_field, rule->params.field_ref.field, rule->params.field_ref.len);
        zend_hash_str_add(&params, "other", 5, &other_field);

        sf_add_error_with_params(ctx, "validation.same", &params);
        zend_hash_destroy(&params);
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* different - Value must NOT match another field's value */
sf_rule_result_t sf_rule_different(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    zval *other_value = sf_get_nested_value(
        rule->params.field_ref.field,
        rule->params.field_ref.len,
        ctx->data
    );

    if (values_equal(ctx->value, other_value)) {
        HashTable params;
        zend_hash_init(&params, 2, NULL, ZVAL_PTR_DTOR, 0);

        zval other_field;
        ZVAL_STRINGL(&other_field, rule->params.field_ref.field, rule->params.field_ref.len);
        zend_hash_str_add(&params, "other", 5, &other_field);

        sf_add_error_with_params(ctx, "validation.different", &params);
        zend_hash_destroy(&params);
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* confirmed - Field must have a matching {field}_confirmation field */
sf_rule_result_t sf_rule_confirmed(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    /* Build confirmation field name: {field}_confirmation */
    size_t confirmation_len = ctx->field_len + 13;  /* "_confirmation" is 13 chars */
    char *confirmation_field = emalloc(confirmation_len + 1);
    memcpy(confirmation_field, ctx->field_name, ctx->field_len);
    memcpy(confirmation_field + ctx->field_len, "_confirmation", 13);
    confirmation_field[confirmation_len] = '\0';

    zval *confirmation_value = sf_get_nested_value(
        confirmation_field,
        confirmation_len,
        ctx->data
    );

    zend_bool match = values_equal(ctx->value, confirmation_value);
    efree(confirmation_field);

    if (!match) {
        sf_add_error(ctx, "validation.confirmed");
        return RULE_FAIL;
    }

    return RULE_PASS;
}
