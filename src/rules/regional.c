/*
 * Regional validation rules
 */

#include "rules.h"
#include "src/condition.h"
#include <ctype.h>

/*
 * Croatian OIB (Osobni identifikacijski broj) validation.
 *
 * OIB is an 11-digit personal identification number used in Croatia.
 * Validation uses the ISO 7064, MOD 11-10 checksum algorithm.
 */
static bool validate_oib(const char *oib, size_t len)
{
    if (len != SF_OIB_LENGTH) {
        return 0;
    }

    /* Check all digits */
    for (size_t i = 0; i < 11; i++) {
        if (oib[i] < '0' || oib[i] > '9') {
            return 0;
        }
    }

    /* ISO 7064, MOD 11-10 algorithm */
    int t = 10;
    for (int i = 0; i < 10; i++) {
        t = ((oib[i] - '0') + t) % 10;
        if (t == 0) {
            t = 10;
        }
        t = (t * 2) % 11;
    }

    int control = (11 - t) % 10;
    return control == (oib[10] - '0');
}

/* oib - Croatian personal identification number */
sf_rule_result_t sf_rule_oib(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.oib");
        return RULE_FAIL;
    }

    if (!validate_oib(Z_STRVAL_P(ctx->value), Z_STRLEN_P(ctx->value))) {
        sf_add_error(ctx, "validation.oib");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/*
 * Basic phone number validation.
 *
 * Validates structural format only - does not verify the number is dialable.
 * Accepts digits, optional leading +, and common separators (space, dash, parens).
 * Requires minimum 7 digits to be considered valid.
 */
static bool validate_phone(const char *phone, size_t len)
{
    if (len < SF_PHONE_MIN_DIGITS || len > SF_PHONE_MAX_LENGTH) {
        return 0;
    }

    size_t digit_count = 0;
    bool has_plus = 0;

    for (size_t i = 0; i < len; i++) {
        char c = phone[i];

        if (c >= '0' && c <= '9') {
            digit_count++;
        } else if (c == '+') {
            if (i != 0) return 0;  /* + must be at start */
            has_plus = 1;
        } else if (c == ' ' || c == '-' || c == '(' || c == ')') {
            /* Allow common separators */
        } else {
            return 0;
        }
    }

    /* Must have at least 7 digits */
    return digit_count >= 7;
}

/* phone - Valid phone number */
sf_rule_result_t sf_rule_phone(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.phone");
        return RULE_FAIL;
    }

    if (!validate_phone(Z_STRVAL_P(ctx->value), Z_STRLEN_P(ctx->value))) {
        sf_add_error(ctx, "validation.phone");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/*
 * IBAN validation using ISO 7064 MOD 97-10 algorithm.
 *
 * The algorithm works by:
 * 1. Moving the first 4 characters (country code + check digits) to the end
 * 2. Converting letters to numbers (A=10, B=11, ..., Z=35)
 * 3. Calculating MOD 97 of the resulting number
 * 4. Valid if remainder equals 1
 *
 * Buffer sizing: Maximum IBAN is 34 chars. Each letter becomes 2 digits.
 * Worst case: 34 letters = 68 digits + null = 69 bytes.
 * We use 80 bytes for safety margin.
 */
#define IBAN_NUMERIC_BUFFER_SIZE 80

static bool validate_iban(const char *iban, size_t len)
{
    if (len < SF_IBAN_MIN_LENGTH || len > SF_IBAN_MAX_LENGTH) {
        return 0;
    }

    /* Convert to numeric string (letters A-Z become 10-35) */
    char numeric[IBAN_NUMERIC_BUFFER_SIZE];
    size_t numeric_len = 0;

    /* Move first 4 characters to end for validation */
    for (size_t i = 4; i < len; i++) {
        char c = toupper((unsigned char)iban[i]);
        if (c >= 'A' && c <= 'Z') {
            int val = c - 'A' + 10;
            /* Each letter becomes at most 2 digits */
            if (numeric_len + 2 >= IBAN_NUMERIC_BUFFER_SIZE) {
                return 0; /* Buffer would overflow */
            }
            int written = snprintf(numeric + numeric_len,
                IBAN_NUMERIC_BUFFER_SIZE - numeric_len, "%d", val);
            if (written < 0 || (size_t)written >= IBAN_NUMERIC_BUFFER_SIZE - numeric_len) {
                return 0;
            }
            numeric_len += (size_t)written;
        } else if (c >= '0' && c <= '9') {
            if (numeric_len + 1 >= IBAN_NUMERIC_BUFFER_SIZE) {
                return 0;
            }
            numeric[numeric_len++] = c;
        } else if (c != ' ') {
            return 0;  /* Invalid character */
        }
    }

    /* Add first 4 characters */
    for (size_t i = 0; i < 4 && i < len; i++) {
        char c = toupper((unsigned char)iban[i]);
        if (c >= 'A' && c <= 'Z') {
            int val = c - 'A' + 10;
            if (numeric_len + 2 >= IBAN_NUMERIC_BUFFER_SIZE) {
                return 0;
            }
            int written = snprintf(numeric + numeric_len,
                IBAN_NUMERIC_BUFFER_SIZE - numeric_len, "%d", val);
            if (written < 0 || (size_t)written >= IBAN_NUMERIC_BUFFER_SIZE - numeric_len) {
                return 0;
            }
            numeric_len += (size_t)written;
        } else if (c >= '0' && c <= '9') {
            if (numeric_len + 1 >= IBAN_NUMERIC_BUFFER_SIZE) {
                return 0;
            }
            numeric[numeric_len++] = c;
        } else {
            return 0;
        }
    }
    numeric[numeric_len] = '\0';

    /* Calculate MOD 97 using iterative approach to avoid big integer math */
    int remainder = 0;
    for (size_t i = 0; i < numeric_len; i++) {
        remainder = (remainder * 10 + (numeric[i] - '0')) % 97;
    }

    return remainder == 1;
}

/* iban - Valid IBAN */
sf_rule_result_t sf_rule_iban(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.iban");
        return RULE_FAIL;
    }

    if (!validate_iban(Z_STRVAL_P(ctx->value), Z_STRLEN_P(ctx->value))) {
        sf_add_error(ctx, "validation.iban");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/*
 * EU VAT number format validation.
 *
 * Validates that the number has a 2-letter country code prefix followed
 * by alphanumeric characters. Does not validate against VIES database
 * or check country-specific formats.
 */
static bool validate_vat_eu(const char *vat, size_t len)
{
    if (len < SF_VAT_EU_MIN_LENGTH || len > SF_VAT_EU_MAX_LENGTH) {
        return 0;
    }

    /* First 2 characters must be country code (letters) */
    if (!((vat[0] >= 'A' && vat[0] <= 'Z') || (vat[0] >= 'a' && vat[0] <= 'z'))) {
        return 0;
    }
    if (!((vat[1] >= 'A' && vat[1] <= 'Z') || (vat[1] >= 'a' && vat[1] <= 'z'))) {
        return 0;
    }

    /* Rest must be alphanumeric */
    for (size_t i = 2; i < len; i++) {
        char c = vat[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
            return 0;
        }
    }

    return 1;
}

/* vat_eu - EU VAT number */
sf_rule_result_t sf_rule_vat_eu(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    if (ctx->has_nullable && ctx->is_null_or_empty) {
        return RULE_PASS;
    }

    if (!ctx->value || Z_TYPE_P(ctx->value) != IS_STRING) {
        sf_add_error(ctx, "validation.vat_eu");
        return RULE_FAIL;
    }

    if (!validate_vat_eu(Z_STRVAL_P(ctx->value), Z_STRLEN_P(ctx->value))) {
        sf_add_error(ctx, "validation.vat_eu");
        return RULE_FAIL;
    }

    return RULE_PASS;
}

/*
 * Shared conditional (RULE_WHEN) handler. Evaluates the condition and
 * recursively applies the then/else rule list. Used by both the top-level
 * validator loop AND by sf_execute_rule when a when-rule appears inside a
 * nested then/else branch — without this second path, nested conditionals
 * silently PASS without executing (bug #8).
 *
 * Depth-bounded to protect against adversarial rule trees.
 *
 * Returns:
 *   RULE_PASS if no inner rule failed (or the branch was empty)
 *   RULE_FAIL if any inner rule failed
 *   RULE_SKIP if an inner rule returned SKIP (propagate to outer loop)
 */
sf_rule_result_t sf_execute_conditional(
    sf_validation_context_t *ctx,
    sf_parsed_rule_t *rule,
    unsigned int depth)
{
    if (depth > SF_MAX_CONDITION_EVAL_DEPTH) {
        /* Fail safe — don't stack-overflow on pathological rule sets. */
        sf_add_error(ctx, "validation.when.depth_exceeded");
        return RULE_FAIL;
    }

    bool condition_met = sf_evaluate_condition(
        rule->params.conditional.condition,
        ctx->value,
        ctx->data,
        ctx->field_name,
        ctx->validator
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

    if (!rules_to_apply) {
        return RULE_PASS;
    }

    sf_rule_result_t branch_result = RULE_PASS;
    for (size_t j = 0; j < rules_count; j++) {
        sf_parsed_rule_t *inner = rules_to_apply[j];

        /* Key fix: if the inner rule is itself a RULE_WHEN, we must recurse
         * into sf_execute_conditional directly. Before this, the inner when
         * hit the `case RULE_WHEN: return RULE_PASS` fallthrough and no inner
         * rules ever ran. */
        sf_rule_result_t r;
        if (inner->type == RULE_WHEN) {
            r = sf_execute_conditional(ctx, inner, depth + 1);
        } else {
            r = sf_execute_rule(ctx, inner);
        }

        if (r == RULE_FAIL) {
            branch_result = RULE_FAIL;
            if (ctx->bail) {
                break;
            }
        } else if (r == RULE_SKIP) {
            /* SKIP means "stop processing further rules in this chain" — we
             * propagate so the outer loop can stop too. */
            return RULE_SKIP;
        }
    }

    return branch_result;
}

/* Rule dispatcher - execute a rule by type */
sf_rule_result_t sf_execute_rule(sf_validation_context_t *ctx, sf_parsed_rule_t *rule)
{
    switch (rule->type) {
        /* Presence */
        case RULE_REQUIRED:     return sf_rule_required(ctx, rule);
        case RULE_NULLABLE:     return sf_rule_nullable(ctx, rule);
        case RULE_FILLED:       return sf_rule_filled(ctx, rule);
        case RULE_PRESENT:      return sf_rule_present(ctx, rule);

        /* Types */
        case RULE_STRING:       return sf_rule_string(ctx, rule);
        case RULE_INTEGER:      return sf_rule_integer(ctx, rule);
        case RULE_NUMERIC:      return sf_rule_numeric(ctx, rule);
        case RULE_BOOLEAN:      return sf_rule_boolean(ctx, rule);
        case RULE_ARRAY:        return sf_rule_array(ctx, rule);

        /* String */
        case RULE_MIN:          return sf_rule_min(ctx, rule);
        case RULE_MAX:          return sf_rule_max(ctx, rule);
        case RULE_BETWEEN:      return sf_rule_between(ctx, rule);
        case RULE_REGEX:        return sf_rule_regex(ctx, rule);
        case RULE_NOT_REGEX:    return sf_rule_not_regex(ctx, rule);
        case RULE_ALPHA:        return sf_rule_alpha(ctx, rule);
        case RULE_ALPHA_NUM:    return sf_rule_alpha_num(ctx, rule);
        case RULE_ALPHA_DASH:   return sf_rule_alpha_dash(ctx, rule);
        case RULE_LOWERCASE:    return sf_rule_lowercase(ctx, rule);
        case RULE_UPPERCASE:    return sf_rule_uppercase(ctx, rule);
        case RULE_STARTS_WITH:  return sf_rule_starts_with(ctx, rule);
        case RULE_ENDS_WITH:    return sf_rule_ends_with(ctx, rule);
        case RULE_CONTAINS:     return sf_rule_contains(ctx, rule);

        /* Numeric */
        case RULE_GT:           return sf_rule_gt(ctx, rule);
        case RULE_GTE:          return sf_rule_gte(ctx, rule);
        case RULE_LT:           return sf_rule_lt(ctx, rule);
        case RULE_LTE:          return sf_rule_lte(ctx, rule);

        /* Array */
        case RULE_DISTINCT:     return sf_rule_distinct(ctx, rule);

        /* Format */
        case RULE_EMAIL:        return sf_rule_email(ctx, rule);
        case RULE_URL:          return sf_rule_url(ctx, rule);
        case RULE_IP:           return sf_rule_ip(ctx, rule);
        case RULE_UUID:         return sf_rule_uuid(ctx, rule);
        case RULE_JSON:         return sf_rule_json(ctx, rule);
        case RULE_DATE:         return sf_rule_date(ctx, rule);
        case RULE_DATE_FORMAT:  return sf_rule_date_format(ctx, rule);
        case RULE_AFTER:        return sf_rule_after(ctx, rule);
        case RULE_BEFORE:       return sf_rule_before(ctx, rule);
        case RULE_AFTER_OR_EQUAL:   return sf_rule_after_or_equal(ctx, rule);
        case RULE_BEFORE_OR_EQUAL:  return sf_rule_before_or_equal(ctx, rule);

        /* Comparison */
        case RULE_IN:           return sf_rule_in(ctx, rule);
        case RULE_NOT_IN:       return sf_rule_not_in(ctx, rule);
        case RULE_SAME:         return sf_rule_same(ctx, rule);
        case RULE_DIFFERENT:    return sf_rule_different(ctx, rule);
        case RULE_CONFIRMED:    return sf_rule_confirmed(ctx, rule);

        /* Regional */
        case RULE_OIB:          return sf_rule_oib(ctx, rule);
        case RULE_PHONE:        return sf_rule_phone(ctx, rule);
        case RULE_IBAN:         return sf_rule_iban(ctx, rule);
        case RULE_VAT_EU:       return sf_rule_vat_eu(ctx, rule);

        /* Conditional. Historically this returned RULE_PASS on the assumption
         * that only the top-level validator loop ever saw RULE_WHEN — but
         * nested when-in-then rule sets DO land here via recursion, and the
         * old early-return silently dropped them. Delegate to the shared
         * helper so nested conditionals actually execute. */
        case RULE_WHEN:
            return sf_execute_conditional(ctx, rule, 1);

        default:
            return RULE_PASS;
    }
}
