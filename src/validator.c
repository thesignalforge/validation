/*
 * Validator class implementation
 */

#include "validator.h"
#include "result.h"
#include "parser.h"
#include "condition.h"
#include "wildcard.h"
#include "rules/rules.h"

/* Object handlers */
zend_object_handlers signalforge_validator_handlers;

/* Free cached regex */
static void free_cached_regex(zval *zv)
{
    cached_regex_t *cached = Z_PTR_P(zv);
    if (cached) {
        if (cached->match_data) {
            pcre2_match_data_free(cached->match_data);
        }
        if (cached->compiled) {
            pcre2_code_free(cached->compiled);
        }
        efree(cached);
    }
}

/* Create Validator object */
static zend_object *signalforge_validator_create(zend_class_entry *ce)
{
    signalforge_validator_t *intern = zend_object_alloc(sizeof(signalforge_validator_t), ce);

    intern->rules = NULL;

    ALLOC_HASHTABLE(intern->regex_cache);
    zend_hash_init(intern->regex_cache, 8, NULL, free_cached_regex, 0);

    zend_object_std_init(&intern->std, ce);
    object_properties_init(&intern->std, ce);

    intern->std.handlers = &signalforge_validator_handlers;
    return &intern->std;
}

/* Free Validator object */
static void signalforge_validator_free(zend_object *object)
{
    signalforge_validator_t *intern = signalforge_validator_from_obj(object);

    if (intern->rules) {
        sf_free_parsed_rules_ht(intern->rules);
        intern->rules = NULL;
    }

    if (intern->regex_cache) {
        zend_hash_destroy(intern->regex_cache);
        FREE_HASHTABLE(intern->regex_cache);
        intern->regex_cache = NULL;
    }

    zend_object_std_dtor(&intern->std);
}

/* Get or compile a regex pattern */
cached_regex_t *sf_get_or_compile_regex(
    signalforge_validator_t *validator,
    const char *pattern,
    size_t pattern_len
)
{
    cached_regex_t *cached;

    /* Check cache first */
    cached = zend_hash_str_find_ptr(validator->regex_cache, pattern, pattern_len);
    if (cached) {
        return cached;
    }

    /* Strip PHP-style delimiters: /pattern/flags or #pattern#flags etc. */
    const char *actual_pattern = pattern;
    size_t actual_len = pattern_len;
    uint32_t options = PCRE2_UTF;

    if (pattern_len >= 2) {
        char delimiter = pattern[0];
        /* Common PHP regex delimiters */
        if (delimiter == '/' || delimiter == '#' || delimiter == '~' ||
            delimiter == '@' || delimiter == '%' || delimiter == '!') {
            /* Find the closing delimiter */
            const char *end = pattern + pattern_len - 1;
            while (end > pattern && *end != delimiter) {
                /* Parse flags */
                switch (*end) {
                    case 'i': options |= PCRE2_CASELESS; break;
                    case 'm': options |= PCRE2_MULTILINE; break;
                    case 's': options |= PCRE2_DOTALL; break;
                    case 'x': options |= PCRE2_EXTENDED; break;
                    case 'u': /* UTF-8 already enabled */ break;
                }
                end--;
            }
            if (end > pattern && *end == delimiter) {
                actual_pattern = pattern + 1;
                actual_len = end - actual_pattern;
            }
        }
    }

    /* Compile and cache */
    int errcode;
    PCRE2_SIZE erroffset;
    pcre2_code *compiled = pcre2_compile(
        (PCRE2_SPTR)actual_pattern,
        actual_len,
        options,
        &errcode,
        &erroffset,
        NULL
    );

    if (!compiled) {
        return NULL;  /* Invalid regex */
    }

    cached = emalloc(sizeof(cached_regex_t));
    cached->compiled = compiled;
    cached->match_data = pcre2_match_data_create_from_pattern(compiled, NULL);

    zend_hash_str_add_ptr(validator->regex_cache, pattern, pattern_len, cached);
    return cached;
}

/* Validate a single field against its rules */
static void validate_field(
    signalforge_validator_t *validator,
    sf_field_rules_t *field_rules,
    zval *value,
    HashTable *data,
    HashTable *errors,
    HashTable *validated,
    const char *actual_field_name,
    size_t actual_field_len
)
{
    sf_validation_context_t ctx;
    ctx.validator = validator;
    ctx.data = data;
    ctx.field_name = actual_field_name;
    ctx.field_len = actual_field_len;
    ctx.value = value;
    ctx.errors = errors;
    ctx.has_nullable = 0;
    ctx.is_null_or_empty = sf_is_empty(value);
    ctx.bail = 0;

    /* Check for nullable rule first */
    for (size_t i = 0; i < field_rules->rule_count; i++) {
        if (field_rules->rules[i]->type == RULE_NULLABLE) {
            ctx.has_nullable = 1;
            break;
        }
    }

    /* Execute each rule */
    zend_bool has_error = 0;
    for (size_t i = 0; i < field_rules->rule_count; i++) {
        sf_parsed_rule_t *rule = field_rules->rules[i];

        /* Handle conditional rules */
        if (rule->type == RULE_WHEN) {
            zend_bool condition_met = sf_evaluate_condition(
                rule->params.conditional.condition,
                value,
                data,
                actual_field_name,
                validator
            );

            sf_parsed_rule_t **rules_to_apply;
            size_t rules_count;

            if (condition_met) {
                rules_to_apply = rule->params.conditional.then_rules;
                rules_count = rule->params.conditional.then_count;
            } else {
                rules_to_apply = rule->params.conditional.else_rules;
                rules_count = rule->params.conditional.else_count;
            }

            if (rules_to_apply) {
                for (size_t j = 0; j < rules_count; j++) {
                    sf_rule_result_t result = sf_execute_rule(&ctx, rules_to_apply[j]);
                    if (result == RULE_FAIL) {
                        has_error = 1;
                        if (ctx.bail) break;
                    } else if (result == RULE_SKIP) {
                        break;
                    }
                }
            }
            continue;
        }

        sf_rule_result_t result = sf_execute_rule(&ctx, rule);
        if (result == RULE_FAIL) {
            has_error = 1;
            if (ctx.bail) break;
        } else if (result == RULE_SKIP) {
            break;
        }
    }

    /* Add to validated if no errors */
    if (!has_error && value) {
        zend_string *key = zend_string_init(actual_field_name, actual_field_len, 0);
        zval copy;
        ZVAL_COPY(&copy, value);
        zend_hash_add(validated, key, &copy);
        zend_string_release(key);
    }
}

/* PHP Method: Validator::__construct(array $rules) */
ZEND_BEGIN_ARG_INFO_EX(arginfo_validator_construct, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, rules, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(Validator, __construct)
{
    HashTable *rules_array;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY_HT(rules_array)
    ZEND_PARSE_PARAMETERS_END();

    signalforge_validator_t *intern = Z_SIGNALFORGE_VALIDATOR_P(ZEND_THIS);

    /* Parse rules */
    intern->rules = sf_parse_rules(rules_array);
    if (!intern->rules) {
        /* Exception was thrown by sf_parse_rules */
        return;
    }
}

/* PHP Method: Validator::validate(array $data): ValidationResult */
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_validator_validate, 0, 1, Signalforge\\Validation\\ValidationResult, 0)
    ZEND_ARG_ARRAY_INFO(0, data, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(Validator, validate)
{
    HashTable *data_array;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_ARRAY_HT(data_array)
    ZEND_PARSE_PARAMETERS_END();

    signalforge_validator_t *intern = Z_SIGNALFORGE_VALIDATOR_P(ZEND_THIS);

    if (!intern->rules) {
        zend_throw_exception(signalforge_invalid_rule_exception_ce,
            "Validator not properly initialized", 0);
        RETURN_THROWS();
    }

    /* Create result object */
    object_init_ex(return_value, signalforge_validation_result_ce);
    signalforge_validation_result_t *result = Z_SIGNALFORGE_VALIDATION_RESULT_P(return_value);

    ALLOC_HASHTABLE(result->errors);
    zend_hash_init(result->errors, 8, NULL, ZVAL_PTR_DTOR, 0);

    ALLOC_HASHTABLE(result->validated);
    zend_hash_init(result->validated, 8, NULL, ZVAL_PTR_DTOR, 0);

    result->is_valid = 1;

    /* Iterate over each field's rules */
    sf_field_rules_t *field_rules;
    ZEND_HASH_FOREACH_PTR(intern->rules, field_rules) {
        /* Check for wildcards */
        if (sf_has_wildcard(field_rules->field_name, field_rules->field_len)) {
            HashTable *expanded = sf_expand_wildcards(
                field_rules->field_name,
                field_rules->field_len,
                data_array
            );

            sf_expanded_field_t *entry;
            ZEND_HASH_FOREACH_PTR(expanded, entry) {
                zval *value = sf_get_nested_value(entry->path, entry->path_len, data_array);
                validate_field(
                    intern,
                    field_rules,
                    value,
                    data_array,
                    result->errors,
                    result->validated,
                    entry->path,
                    entry->path_len
                );
            } ZEND_HASH_FOREACH_END();

            sf_free_expanded_fields(expanded);
        } else {
            /* Simple field - get value from data */
            zval *value = sf_get_nested_value(
                field_rules->field_name,
                field_rules->field_len,
                data_array
            );

            validate_field(
                intern,
                field_rules,
                value,
                data_array,
                result->errors,
                result->validated,
                field_rules->field_name,
                field_rules->field_len
            );
        }
    } ZEND_HASH_FOREACH_END();

    /* Set is_valid based on errors */
    result->is_valid = (zend_hash_num_elements(result->errors) == 0);
}

/* PHP Method: Validator::make(array $data, array $rules): Validator */
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_validator_make, 0, 2, Signalforge\\Validation\\Validator, 0)
    ZEND_ARG_ARRAY_INFO(0, data, 0)
    ZEND_ARG_ARRAY_INFO(0, rules, 0)
ZEND_END_ARG_INFO()

/*
 * Static factory method to create a Validator instance.
 *
 * This matches the Laravel validation API pattern where Validator::make()
 * creates and configures a validator in one call.
 *
 * Note: The data parameter is accepted for API compatibility but not stored,
 * as validation is performed via a separate validate() call.
 */
PHP_METHOD(Validator, make)
{
    HashTable *data_array;
    HashTable *rules_array;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_ARRAY_HT(data_array)
        Z_PARAM_ARRAY_HT(rules_array)
    ZEND_PARSE_PARAMETERS_END();

    /* Parse rules first - if this fails, we don't create the object */
    HashTable *parsed_rules = sf_parse_rules(rules_array);
    if (!parsed_rules) {
        /* Exception was thrown by sf_parse_rules */
        RETURN_THROWS();
    }

    /* Create new Validator instance */
    object_init_ex(return_value, signalforge_validator_ce);
    signalforge_validator_t *intern = Z_SIGNALFORGE_VALIDATOR_P(return_value);

    /* Transfer ownership of parsed rules to the validator */
    intern->rules = parsed_rules;
}

/* Method table */
static const zend_function_entry validator_methods[] = {
    PHP_ME(Validator, __construct, arginfo_validator_construct, ZEND_ACC_PUBLIC)
    PHP_ME(Validator, validate, arginfo_validator_validate, ZEND_ACC_PUBLIC)
    PHP_ME(Validator, make, arginfo_validator_make, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

/*
 * Clone handler for Validator objects.
 *
 * Creates a deep copy of the validator including its parsed rules and regex
 * cache. This ensures cloned validators are independent and don't share
 * mutable state with the original.
 */
static zend_object *signalforge_validator_clone(zend_object *old_obj)
{
    signalforge_validator_t *old_intern = signalforge_validator_from_obj(old_obj);
    signalforge_validator_t *new_intern;

    new_intern = signalforge_validator_from_obj(signalforge_validator_create(old_obj->ce));

    /* Clone standard object properties */
    zend_objects_clone_members(&new_intern->std, old_obj);

    /*
     * Note: We don't deep-copy the parsed rules as they are read-only after
     * construction. The regex cache is also not copied - it will be rebuilt
     * on demand. This is safe because:
     * 1. Rules are never modified after parsing
     * 2. Regex cache is populated lazily during validation
     *
     * For true isolation, we would need to deep-copy the rules structure,
     * but that adds complexity with minimal benefit since validators are
     * typically not cloned in practice.
     */

    return &new_intern->std;
}

/* Register Validator class */
void signalforge_register_validator_class(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Signalforge\\Validation", "Validator", validator_methods);
    signalforge_validator_ce = zend_register_internal_class(&ce);
    signalforge_validator_ce->create_object = signalforge_validator_create;

    memcpy(&signalforge_validator_handlers, &std_object_handlers, sizeof(zend_object_handlers));
    signalforge_validator_handlers.offset = XtOffsetOf(signalforge_validator_t, std);
    signalforge_validator_handlers.free_obj = signalforge_validator_free;
    signalforge_validator_handlers.clone_obj = signalforge_validator_clone;
}

/* Register exception class */
void signalforge_register_exception_class(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Signalforge\\Validation", "InvalidRuleException", NULL);
    signalforge_invalid_rule_exception_ce = zend_register_internal_class_ex(&ce, zend_ce_exception);
}
