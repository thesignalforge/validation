/*
 * Array validation rules
 */

#include "rules.h"
#include "src/condition.h"

/* distinct - All array values must be unique */
sf_rule_result_t sf_rule_distinct(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_ARRAY) {
        sf_add_error(ctx, "validation.distinct");
        return RULE_FAIL;
    }

    HashTable *arr = Z_ARRVAL_P(ctx->value);
    HashTable seen;
    zend_hash_init(&seen, zend_hash_num_elements(arr), NULL, NULL, 0);

    zval *item;
    zend_bool has_duplicate = 0;

    ZEND_HASH_FOREACH_VAL(arr, item) {
        /* Convert value to string for comparison */
        zend_string *key_str = NULL;

        switch (Z_TYPE_P(item)) {
            case IS_STRING:
                key_str = Z_STR_P(item);
                break;

            case IS_LONG: {
                char buf[32];
                int len = snprintf(buf, sizeof(buf), ZEND_LONG_FMT, Z_LVAL_P(item));
                key_str = zend_string_init(buf, len, 0);
                break;
            }

            case IS_DOUBLE: {
                char buf[64];
                int len = snprintf(buf, sizeof(buf), "%g", Z_DVAL_P(item));
                key_str = zend_string_init(buf, len, 0);
                break;
            }

            case IS_TRUE:
                key_str = zend_string_init("1", 1, 0);
                break;

            case IS_FALSE:
                key_str = zend_string_init("0", 1, 0);
                break;

            case IS_NULL:
                key_str = zend_string_init("", 0, 0);
                break;

            default:
                continue;  /* Skip complex types */
        }

        if (key_str) {
            if (zend_hash_exists(&seen, key_str)) {
                has_duplicate = 1;
                if (Z_TYPE_P(item) != IS_STRING) {
                    zend_string_release(key_str);
                }
                break;
            }

            zval dummy;
            ZVAL_TRUE(&dummy);
            zend_hash_add(&seen, key_str, &dummy);

            if (Z_TYPE_P(item) != IS_STRING) {
                zend_string_release(key_str);
            }
        }
    } ZEND_HASH_FOREACH_END();

    zend_hash_destroy(&seen);

    if (has_duplicate) {
        sf_add_error(ctx, "validation.distinct");
        return RULE_FAIL;
    }

    return RULE_PASS;
}
