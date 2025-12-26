/*
 * Rule array parser
 */

#include "parser.h"
#include "condition.h"

/* Rule name lookup table */
typedef struct {
    const char *name;
    size_t len;
    sf_rule_type_t type;
} sf_rule_lookup_t;

static const sf_rule_lookup_t rule_lookup[] = {
    /* Presence rules */
    {"required", 8, RULE_REQUIRED},
    {"nullable", 8, RULE_NULLABLE},
    {"filled", 6, RULE_FILLED},
    {"present", 7, RULE_PRESENT},

    /* Type rules */
    {"string", 6, RULE_STRING},
    {"integer", 7, RULE_INTEGER},
    {"numeric", 7, RULE_NUMERIC},
    {"boolean", 7, RULE_BOOLEAN},
    {"array", 5, RULE_ARRAY},

    /* String rules */
    {"min", 3, RULE_MIN},
    {"max", 3, RULE_MAX},
    {"between", 7, RULE_BETWEEN},
    {"regex", 5, RULE_REGEX},
    {"not_regex", 9, RULE_NOT_REGEX},
    {"alpha", 5, RULE_ALPHA},
    {"alpha_num", 9, RULE_ALPHA_NUM},
    {"alpha_dash", 10, RULE_ALPHA_DASH},
    {"lowercase", 9, RULE_LOWERCASE},
    {"uppercase", 9, RULE_UPPERCASE},
    {"starts_with", 11, RULE_STARTS_WITH},
    {"ends_with", 9, RULE_ENDS_WITH},
    {"contains", 8, RULE_CONTAINS},

    /* Numeric rules */
    {"gt", 2, RULE_GT},
    {"gte", 3, RULE_GTE},
    {"lt", 2, RULE_LT},
    {"lte", 3, RULE_LTE},

    /* Array rules */
    {"distinct", 8, RULE_DISTINCT},

    /* Format rules */
    {"email", 5, RULE_EMAIL},
    {"url", 3, RULE_URL},
    {"ip", 2, RULE_IP},
    {"uuid", 4, RULE_UUID},
    {"json", 4, RULE_JSON},
    {"date", 4, RULE_DATE},
    {"date_format", 11, RULE_DATE_FORMAT},
    {"after", 5, RULE_AFTER},
    {"before", 6, RULE_BEFORE},
    {"after_or_equal", 14, RULE_AFTER_OR_EQUAL},
    {"before_or_equal", 15, RULE_BEFORE_OR_EQUAL},

    /* Comparison rules */
    {"in", 2, RULE_IN},
    {"not_in", 6, RULE_NOT_IN},
    {"same", 4, RULE_SAME},
    {"different", 9, RULE_DIFFERENT},
    {"confirmed", 9, RULE_CONFIRMED},

    /* Regional rules */
    {"oib", 3, RULE_OIB},
    {"phone", 5, RULE_PHONE},
    {"iban", 4, RULE_IBAN},
    {"vat_eu", 6, RULE_VAT_EU},

    /* Conditional */
    {"when", 4, RULE_WHEN},

    {NULL, 0, RULE_UNKNOWN}
};

/* Validate rule name - must match pattern ^[a-z][a-z0-9_]*$ */
zend_bool sf_validate_rule_name(const char *name, size_t len)
{
    if (len == 0 || len > RULE_NAME_MAX_LENGTH) {
        return 0;
    }

    /* First char must be lowercase letter */
    if (name[0] < 'a' || name[0] > 'z') {
        return 0;
    }

    /* Rest must be lowercase, digit, or underscore */
    for (size_t i = 1; i < len; i++) {
        char c = name[i];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')) {
            return 0;
        }
    }

    return 1;
}

/* Get rule type from string */
sf_rule_type_t sf_get_rule_type(const char *name, size_t len)
{
    for (const sf_rule_lookup_t *entry = rule_lookup; entry->name != NULL; entry++) {
        if (entry->len == len && memcmp(entry->name, name, len) == 0) {
            return entry->type;
        }
    }
    return RULE_UNKNOWN;
}

/* Parse a single rule from PHP value */
static sf_parsed_rule_t *parse_single_rule(zval *rule_zval)
{
    sf_parsed_rule_t *rule = ecalloc(1, sizeof(sf_parsed_rule_t));

    if (Z_TYPE_P(rule_zval) == IS_STRING) {
        /* Simple rule: 'required', 'email', etc. */
        zend_string *name = Z_STR_P(rule_zval);
        rule->type = sf_get_rule_type(ZSTR_VAL(name), ZSTR_LEN(name));

        if (rule->type == RULE_UNKNOWN) {
            zend_throw_exception_ex(signalforge_invalid_rule_exception_ce, 0,
                "Unknown validation rule: %s", ZSTR_VAL(name));
            efree(rule);
            return NULL;
        }
    } else if (Z_TYPE_P(rule_zval) == IS_ARRAY) {
        /* Parameterized rule: ['min', 5], ['between', 1, 10], etc. */
        HashTable *arr = Z_ARRVAL_P(rule_zval);
        zval *first = zend_hash_index_find(arr, 0);

        if (!first || Z_TYPE_P(first) != IS_STRING) {
            zend_throw_exception_ex(signalforge_invalid_rule_exception_ce, 0,
                "Rule array must start with a rule name string");
            efree(rule);
            return NULL;
        }

        zend_string *name = Z_STR_P(first);
        rule->type = sf_get_rule_type(ZSTR_VAL(name), ZSTR_LEN(name));

        if (rule->type == RULE_UNKNOWN) {
            zend_throw_exception_ex(signalforge_invalid_rule_exception_ce, 0,
                "Unknown validation rule: %s", ZSTR_VAL(name));
            efree(rule);
            return NULL;
        }

        /* Parse parameters based on rule type */
        switch (rule->type) {
            case RULE_MIN:
            case RULE_MAX:
            case RULE_GT:
            case RULE_GTE:
            case RULE_LT:
            case RULE_LTE: {
                zval *param = zend_hash_index_find(arr, 1);
                if (!param) {
                    zend_throw_exception_ex(signalforge_invalid_rule_exception_ce, 0,
                        "Rule '%s' requires a parameter", ZSTR_VAL(name));
                    efree(rule);
                    return NULL;
                }
                rule->params.size.value = zval_get_long(param);
                break;
            }

            case RULE_BETWEEN: {
                zval *min_param = zend_hash_index_find(arr, 1);
                zval *max_param = zend_hash_index_find(arr, 2);
                if (!min_param || !max_param) {
                    zend_throw_exception_ex(signalforge_invalid_rule_exception_ce, 0,
                        "Rule 'between' requires two parameters");
                    efree(rule);
                    return NULL;
                }
                rule->params.range.min = zval_get_long(min_param);
                rule->params.range.max = zval_get_long(max_param);
                break;
            }

            case RULE_REGEX:
            case RULE_NOT_REGEX: {
                zval *pattern = zend_hash_index_find(arr, 1);
                if (!pattern || Z_TYPE_P(pattern) != IS_STRING) {
                    zend_throw_exception_ex(signalforge_invalid_rule_exception_ce, 0,
                        "Rule '%s' requires a regex pattern string", ZSTR_VAL(name));
                    efree(rule);
                    return NULL;
                }
                rule->params.regex.pattern = estrndup(Z_STRVAL_P(pattern), Z_STRLEN_P(pattern));
                rule->params.regex.len = Z_STRLEN_P(pattern);
                break;
            }

            case RULE_STARTS_WITH:
            case RULE_ENDS_WITH:
            case RULE_CONTAINS:
            case RULE_DATE_FORMAT: {
                zval *str = zend_hash_index_find(arr, 1);
                if (!str || Z_TYPE_P(str) != IS_STRING) {
                    zend_throw_exception_ex(signalforge_invalid_rule_exception_ce, 0,
                        "Rule '%s' requires a string parameter", ZSTR_VAL(name));
                    efree(rule);
                    return NULL;
                }
                rule->params.string.str = estrndup(Z_STRVAL_P(str), Z_STRLEN_P(str));
                rule->params.string.len = Z_STRLEN_P(str);
                break;
            }

            case RULE_SAME:
            case RULE_DIFFERENT:
            case RULE_AFTER:
            case RULE_BEFORE:
            case RULE_AFTER_OR_EQUAL:
            case RULE_BEFORE_OR_EQUAL: {
                zval *field = zend_hash_index_find(arr, 1);
                if (!field || Z_TYPE_P(field) != IS_STRING) {
                    zend_throw_exception_ex(signalforge_invalid_rule_exception_ce, 0,
                        "Rule '%s' requires a field name", ZSTR_VAL(name));
                    efree(rule);
                    return NULL;
                }
                rule->params.field_ref.field = estrndup(Z_STRVAL_P(field), Z_STRLEN_P(field));
                rule->params.field_ref.len = Z_STRLEN_P(field);
                break;
            }

            case RULE_IN:
            case RULE_NOT_IN: {
                zval *values = zend_hash_index_find(arr, 1);
                if (!values || Z_TYPE_P(values) != IS_ARRAY) {
                    zend_throw_exception_ex(signalforge_invalid_rule_exception_ce, 0,
                        "Rule '%s' requires an array of values", ZSTR_VAL(name));
                    efree(rule);
                    return NULL;
                }
                ALLOC_HASHTABLE(rule->params.in_list.values);
                zend_hash_init(rule->params.in_list.values, zend_hash_num_elements(Z_ARRVAL_P(values)), NULL, ZVAL_PTR_DTOR, 0);
                zend_hash_copy(rule->params.in_list.values, Z_ARRVAL_P(values), zval_add_ref);
                break;
            }

            case RULE_WHEN: {
                /* ['when', condition, then_rules, else_rules?] */
                zval *condition_zval = zend_hash_index_find(arr, 1);
                zval *then_zval = zend_hash_index_find(arr, 2);
                zval *else_zval = zend_hash_index_find(arr, 3);

                if (!condition_zval || Z_TYPE_P(condition_zval) != IS_ARRAY) {
                    zend_throw_exception_ex(signalforge_invalid_rule_exception_ce, 0,
                        "Rule 'when' requires a condition array");
                    efree(rule);
                    return NULL;
                }

                if (!then_zval || Z_TYPE_P(then_zval) != IS_ARRAY) {
                    zend_throw_exception_ex(signalforge_invalid_rule_exception_ce, 0,
                        "Rule 'when' requires a 'then' rules array");
                    efree(rule);
                    return NULL;
                }

                /* Parse condition */
                rule->params.conditional.condition = sf_parse_condition(condition_zval);
                if (!rule->params.conditional.condition) {
                    efree(rule);
                    return NULL;
                }

                /* Parse then rules */
                HashTable *then_arr = Z_ARRVAL_P(then_zval);
                size_t then_count = zend_hash_num_elements(then_arr);
                rule->params.conditional.then_rules = ecalloc(then_count, sizeof(sf_parsed_rule_t *));
                rule->params.conditional.then_count = 0;

                zval *then_rule;
                ZEND_HASH_FOREACH_VAL(then_arr, then_rule) {
                    sf_parsed_rule_t *parsed = parse_single_rule(then_rule);
                    if (!parsed) {
                        /* Cleanup and return */
                        for (size_t i = 0; i < rule->params.conditional.then_count; i++) {
                            sf_free_parsed_rule(rule->params.conditional.then_rules[i]);
                        }
                        efree(rule->params.conditional.then_rules);
                        sf_free_condition(rule->params.conditional.condition);
                        efree(rule);
                        return NULL;
                    }
                    rule->params.conditional.then_rules[rule->params.conditional.then_count++] = parsed;
                } ZEND_HASH_FOREACH_END();

                /* Parse else rules if present */
                rule->params.conditional.else_rules = NULL;
                rule->params.conditional.else_count = 0;

                if (else_zval && Z_TYPE_P(else_zval) == IS_ARRAY) {
                    HashTable *else_arr = Z_ARRVAL_P(else_zval);
                    size_t else_count = zend_hash_num_elements(else_arr);
                    rule->params.conditional.else_rules = ecalloc(else_count, sizeof(sf_parsed_rule_t *));

                    zval *else_rule;
                    ZEND_HASH_FOREACH_VAL(else_arr, else_rule) {
                        sf_parsed_rule_t *parsed = parse_single_rule(else_rule);
                        if (!parsed) {
                            /* Cleanup */
                            for (size_t i = 0; i < rule->params.conditional.then_count; i++) {
                                sf_free_parsed_rule(rule->params.conditional.then_rules[i]);
                            }
                            for (size_t i = 0; i < rule->params.conditional.else_count; i++) {
                                sf_free_parsed_rule(rule->params.conditional.else_rules[i]);
                            }
                            efree(rule->params.conditional.then_rules);
                            efree(rule->params.conditional.else_rules);
                            sf_free_condition(rule->params.conditional.condition);
                            efree(rule);
                            return NULL;
                        }
                        rule->params.conditional.else_rules[rule->params.conditional.else_count++] = parsed;
                    } ZEND_HASH_FOREACH_END();
                }
                break;
            }

            default:
                /* No parameters needed or already handled */
                break;
        }
    } else {
        zend_throw_exception_ex(signalforge_invalid_rule_exception_ce, 0,
            "Rule must be a string or array");
        efree(rule);
        return NULL;
    }

    return rule;
}

/* Parse rules from PHP array */
HashTable *sf_parse_rules(HashTable *rules_array)
{
    HashTable *parsed_rules;
    ALLOC_HASHTABLE(parsed_rules);
    zend_hash_init(parsed_rules, zend_hash_num_elements(rules_array), NULL, NULL, 0);

    zend_string *field_name;
    zval *field_rules;

    ZEND_HASH_FOREACH_STR_KEY_VAL(rules_array, field_name, field_rules) {
        if (!field_name) {
            zend_throw_exception_ex(signalforge_invalid_rule_exception_ce, 0,
                "Field names must be strings");
            sf_free_parsed_rules_ht(parsed_rules);
            return NULL;
        }

        /* Validate field name */
        if (!sf_validate_rule_name(ZSTR_VAL(field_name), ZSTR_LEN(field_name))) {
            /* Allow dot notation for nested fields: items.*.name */
            const char *p = ZSTR_VAL(field_name);
            size_t len = ZSTR_LEN(field_name);
            zend_bool valid = 1;

            for (size_t i = 0; i < len && valid; i++) {
                char c = p[i];
                if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '*')) {
                    valid = 0;
                }
            }

            if (!valid) {
                zend_throw_exception_ex(signalforge_invalid_rule_exception_ce, 0,
                    "Invalid field name: %s", ZSTR_VAL(field_name));
                sf_free_parsed_rules_ht(parsed_rules);
                return NULL;
            }
        }

        if (Z_TYPE_P(field_rules) != IS_ARRAY) {
            zend_throw_exception_ex(signalforge_invalid_rule_exception_ce, 0,
                "Rules for field '%s' must be an array", ZSTR_VAL(field_name));
            sf_free_parsed_rules_ht(parsed_rules);
            return NULL;
        }

        /* Create field rules structure */
        sf_field_rules_t *fr = ecalloc(1, sizeof(sf_field_rules_t));
        fr->field_name = estrndup(ZSTR_VAL(field_name), ZSTR_LEN(field_name));
        fr->field_len = ZSTR_LEN(field_name);

        HashTable *rules_arr = Z_ARRVAL_P(field_rules);
        size_t rule_count = zend_hash_num_elements(rules_arr);
        fr->rules = ecalloc(rule_count, sizeof(sf_parsed_rule_t *));
        fr->rule_count = 0;

        zval *rule_zval;
        ZEND_HASH_FOREACH_VAL(rules_arr, rule_zval) {
            sf_parsed_rule_t *parsed = parse_single_rule(rule_zval);
            if (!parsed) {
                /* Cleanup */
                sf_free_field_rules(fr);
                sf_free_parsed_rules_ht(parsed_rules);
                return NULL;
            }
            fr->rules[fr->rule_count++] = parsed;
        } ZEND_HASH_FOREACH_END();

        zend_hash_add_ptr(parsed_rules, field_name, fr);
    } ZEND_HASH_FOREACH_END();

    return parsed_rules;
}

/* Free a parsed rule */
void sf_free_parsed_rule(sf_parsed_rule_t *rule)
{
    if (!rule) return;

    switch (rule->type) {
        case RULE_REGEX:
        case RULE_NOT_REGEX:
            if (rule->params.regex.pattern) {
                efree(rule->params.regex.pattern);
            }
            break;

        case RULE_STARTS_WITH:
        case RULE_ENDS_WITH:
        case RULE_CONTAINS:
        case RULE_DATE_FORMAT:
            if (rule->params.string.str) {
                efree(rule->params.string.str);
            }
            break;

        case RULE_SAME:
        case RULE_DIFFERENT:
        case RULE_AFTER:
        case RULE_BEFORE:
        case RULE_AFTER_OR_EQUAL:
        case RULE_BEFORE_OR_EQUAL:
            if (rule->params.field_ref.field) {
                efree(rule->params.field_ref.field);
            }
            break;

        case RULE_IN:
        case RULE_NOT_IN:
            if (rule->params.in_list.values) {
                zend_hash_destroy(rule->params.in_list.values);
                FREE_HASHTABLE(rule->params.in_list.values);
            }
            break;

        case RULE_WHEN:
            if (rule->params.conditional.condition) {
                sf_free_condition(rule->params.conditional.condition);
            }
            if (rule->params.conditional.then_rules) {
                for (size_t i = 0; i < rule->params.conditional.then_count; i++) {
                    sf_free_parsed_rule(rule->params.conditional.then_rules[i]);
                }
                efree(rule->params.conditional.then_rules);
            }
            if (rule->params.conditional.else_rules) {
                for (size_t i = 0; i < rule->params.conditional.else_count; i++) {
                    sf_free_parsed_rule(rule->params.conditional.else_rules[i]);
                }
                efree(rule->params.conditional.else_rules);
            }
            break;

        default:
            break;
    }

    efree(rule);
}

/* Free field rules */
void sf_free_field_rules(sf_field_rules_t *field_rules)
{
    if (!field_rules) return;

    if (field_rules->field_name) {
        efree(field_rules->field_name);
    }

    if (field_rules->rules) {
        for (size_t i = 0; i < field_rules->rule_count; i++) {
            sf_free_parsed_rule(field_rules->rules[i]);
        }
        efree(field_rules->rules);
    }

    efree(field_rules);
}

/* Free parsed rules hashtable */
void sf_free_parsed_rules_ht(HashTable *rules)
{
    if (!rules) return;

    sf_field_rules_t *fr;
    ZEND_HASH_FOREACH_PTR(rules, fr) {
        sf_free_field_rules(fr);
    } ZEND_HASH_FOREACH_END();

    zend_hash_destroy(rules);
    FREE_HASHTABLE(rules);
}
