/*
 * Presence validation rules
 */

#include "rules.h"
#include "src/condition.h"

/* Add error helper */
void sf_add_error(sf_validation_context_t *ctx, const char *key, ...)
{
    /* Create error entry */
    zval error_entry;
    array_init(&error_entry);

    /* Add key */
    add_assoc_string(&error_entry, "key", (char *)key);

    /* Add params */
    zval params;
    array_init(&params);
    add_assoc_stringl(&params, "field", ctx->field_name, ctx->field_len);
    add_assoc_zval(&error_entry, "params", &params);

    /* Get or create field errors array */
    zend_string *field_key = zend_string_init(ctx->field_name, ctx->field_len, 0);
    zval *field_errors = zend_hash_find(ctx->errors, field_key);

    if (!field_errors) {
        zval new_arr;
        array_init(&new_arr);
        field_errors = zend_hash_add(ctx->errors, field_key, &new_arr);
    }

    /* Add error to field errors */
    add_next_index_zval(field_errors, &error_entry);

    zend_string_release(field_key);
}

/* Add error with custom params */
void sf_add_error_with_params(sf_validation_context_t *ctx, const char *key, HashTable *params)
{
    /* Create error entry */
    zval error_entry;
    array_init(&error_entry);

    /* Add key */
    add_assoc_string(&error_entry, "key", (char *)key);

    /* Add params */
    zval params_zval;
    array_init(&params_zval);
    add_assoc_stringl(&params_zval, "field", ctx->field_name, ctx->field_len);

    if (params) {
        zend_string *param_key;
        zval *param_val;
        ZEND_HASH_FOREACH_STR_KEY_VAL(params, param_key, param_val) {
            if (param_key) {
                zval copy;
                ZVAL_COPY(&copy, param_val);
                zend_hash_add(Z_ARRVAL(params_zval), param_key, &copy);
            }
        } ZEND_HASH_FOREACH_END();
    }

    add_assoc_zval(&error_entry, "params", &params_zval);

    /* Get or create field errors array */
    zend_string *field_key = zend_string_init(ctx->field_name, ctx->field_len, 0);
    zval *field_errors = zend_hash_find(ctx->errors, field_key);

    if (!field_errors) {
        zval new_arr;
        array_init(&new_arr);
        field_errors = zend_hash_add(ctx->errors, field_key, &new_arr);
    }

    /* Add error to field errors */
    add_next_index_zval(field_errors, &error_entry);

    zend_string_release(field_key);
}

/* required - Field must be present and not empty */
sf_rule_result_t sf_rule_required(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->is_null_or_empty) {
        sf_add_error(ctx, "validation.required");
        return RULE_FAIL;
    }
    return RULE_PASS;
}

/* nullable - Allow null/empty values (skip other rules if null) */
sf_rule_result_t sf_rule_nullable(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->is_null_or_empty) {
        return RULE_SKIP;  /* Skip remaining rules */
    }
    return RULE_PASS;
}

/* filled - If present, must not be empty */
sf_rule_result_t sf_rule_filled(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->value == NULL) {
        /* Not present - OK */
        return RULE_SKIP;
    }

    if (sf_is_empty(ctx->value)) {
        sf_add_error(ctx, "validation.filled");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/* present - Field must exist in input (can be empty) */
sf_rule_result_t sf_rule_present(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->value == NULL) {
        sf_add_error(ctx, "validation.present");
        return RULE_FAIL;
    }
    return RULE_PASS;
}
