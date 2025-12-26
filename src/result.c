/*
 * ValidationResult class implementation
 */

#include "result.h"

/* Object handlers */
zend_object_handlers signalforge_validation_result_handlers;

/* Create ValidationResult object */
static zend_object *signalforge_validation_result_create(zend_class_entry *ce)
{
    signalforge_validation_result_t *intern = zend_object_alloc(sizeof(signalforge_validation_result_t), ce);

    intern->is_valid = 1;
    intern->errors = NULL;
    intern->validated = NULL;

    zend_object_std_init(&intern->std, ce);
    object_properties_init(&intern->std, ce);

    intern->std.handlers = &signalforge_validation_result_handlers;
    return &intern->std;
}

/* Free ValidationResult object */
static void signalforge_validation_result_free(zend_object *object)
{
    signalforge_validation_result_t *intern = signalforge_validation_result_from_obj(object);

    if (intern->errors) {
        zend_hash_destroy(intern->errors);
        FREE_HASHTABLE(intern->errors);
        intern->errors = NULL;
    }

    if (intern->validated) {
        zend_hash_destroy(intern->validated);
        FREE_HASHTABLE(intern->validated);
        intern->validated = NULL;
    }

    zend_object_std_dtor(&intern->std);
}

/* PHP Method: ValidationResult::valid(): bool */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_result_valid, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(ValidationResult, valid)
{
    ZEND_PARSE_PARAMETERS_NONE();

    signalforge_validation_result_t *intern = Z_SIGNALFORGE_VALIDATION_RESULT_P(ZEND_THIS);
    RETURN_BOOL(intern->is_valid);
}

/* PHP Method: ValidationResult::failed(): bool */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_result_failed, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(ValidationResult, failed)
{
    ZEND_PARSE_PARAMETERS_NONE();

    signalforge_validation_result_t *intern = Z_SIGNALFORGE_VALIDATION_RESULT_P(ZEND_THIS);
    RETURN_BOOL(!intern->is_valid);
}

/* PHP Method: ValidationResult::errors(): array */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_result_errors, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(ValidationResult, errors)
{
    ZEND_PARSE_PARAMETERS_NONE();

    signalforge_validation_result_t *intern = Z_SIGNALFORGE_VALIDATION_RESULT_P(ZEND_THIS);

    if (intern->errors) {
        RETURN_ARR(zend_array_dup(intern->errors));
    }
    RETURN_EMPTY_ARRAY();
}

/* PHP Method: ValidationResult::validated(): array */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_result_validated, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(ValidationResult, validated)
{
    ZEND_PARSE_PARAMETERS_NONE();

    signalforge_validation_result_t *intern = Z_SIGNALFORGE_VALIDATION_RESULT_P(ZEND_THIS);

    if (intern->validated) {
        RETURN_ARR(zend_array_dup(intern->validated));
    }
    RETURN_EMPTY_ARRAY();
}

/* PHP Method: ValidationResult::errorsFor(string $field): array */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_result_errors_for, 0, 1, IS_ARRAY, 0)
    ZEND_ARG_TYPE_INFO(0, field, IS_STRING, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(ValidationResult, errorsFor)
{
    zend_string *field;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(field)
    ZEND_PARSE_PARAMETERS_END();

    signalforge_validation_result_t *intern = Z_SIGNALFORGE_VALIDATION_RESULT_P(ZEND_THIS);

    if (intern->errors) {
        zval *field_errors = zend_hash_find(intern->errors, field);
        if (field_errors && Z_TYPE_P(field_errors) == IS_ARRAY) {
            RETURN_ARR(zend_array_dup(Z_ARRVAL_P(field_errors)));
        }
    }
    RETURN_EMPTY_ARRAY();
}

/* PHP Method: ValidationResult::hasError(string $field): bool */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_result_has_error, 0, 1, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, field, IS_STRING, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(ValidationResult, hasError)
{
    zend_string *field;

    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(field)
    ZEND_PARSE_PARAMETERS_END();

    signalforge_validation_result_t *intern = Z_SIGNALFORGE_VALIDATION_RESULT_P(ZEND_THIS);

    if (intern->errors) {
        zval *field_errors = zend_hash_find(intern->errors, field);
        if (field_errors && Z_TYPE_P(field_errors) == IS_ARRAY) {
            RETURN_BOOL(zend_hash_num_elements(Z_ARRVAL_P(field_errors)) > 0);
        }
    }
    RETURN_FALSE;
}

/* Method table */
static const zend_function_entry validation_result_methods[] = {
    PHP_ME(ValidationResult, valid, arginfo_result_valid, ZEND_ACC_PUBLIC)
    PHP_ME(ValidationResult, failed, arginfo_result_failed, ZEND_ACC_PUBLIC)
    PHP_ME(ValidationResult, errors, arginfo_result_errors, ZEND_ACC_PUBLIC)
    PHP_ME(ValidationResult, validated, arginfo_result_validated, ZEND_ACC_PUBLIC)
    PHP_ME(ValidationResult, errorsFor, arginfo_result_errors_for, ZEND_ACC_PUBLIC)
    PHP_ME(ValidationResult, hasError, arginfo_result_has_error, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

/* Register ValidationResult class */
void signalforge_register_result_class(void)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Signalforge\\Validation", "ValidationResult", validation_result_methods);
    signalforge_validation_result_ce = zend_register_internal_class(&ce);
    signalforge_validation_result_ce->create_object = signalforge_validation_result_create;

    memcpy(&signalforge_validation_result_handlers, &std_object_handlers, sizeof(zend_object_handlers));
    signalforge_validation_result_handlers.offset = XtOffsetOf(signalforge_validation_result_t, std);
    signalforge_validation_result_handlers.free_obj = signalforge_validation_result_free;
}
