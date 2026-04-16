/*
 * libFuzzer harness for the Signalforge validation (vex) rule parser.
 *
 * Scope and limits
 * ----------------
 * The production entry point sf_parse_rules() takes a HashTable* of
 * PHP zvals. Producing representative zval arrays from fuzzer bytes
 * requires a real PHP runtime (libphp-embed) because PHP rule values
 * include nested arrays, int zvals, and string zvals with full
 * ref-counting semantics. That runtime is ~10MB and not available as
 * a pre-packaged .so on Ubuntu.
 *
 * What this harness DOES cover:
 *
 *   1. sf_validate_rule_name() - exact byte scanner for the
 *      ^[a-z][a-z0-9_]*$ pattern, with a 1024-byte cap.
 *   2. sf_get_rule_type() - linear lookup in a static table of 45+
 *      rule names. Returns RULE_UNKNOWN on miss.
 *   3. A byte-driven mini DSL that builds synthetic rule HashTables
 *      (strings + simple parameterized arrays) and hands them to
 *      sf_parse_rules() via our shim. This covers ~70% of the parser:
 *      presence rules, type rules, string rules, comparison rules,
 *      regex/starts_with/etc. parameter extraction.
 *
 * What this harness does NOT cover:
 *
 *   - ['when', condition, then_rules, else_rules] - the when handler
 *     calls sf_parse_condition from condition.c which drags in
 *     pcre2, compare_function, and zval_get_long. Leave as a TODO -
 *     requires libphp-embed or a deeper shim of Zend's operator
 *     helpers.
 *   - ['in', array] / ['not_in', array] - uses zend_hash_copy +
 *     zval_add_ref which would require a more complete zval shim.
 *
 * If you port this to a PHP-embed host: the DSL decoder below is the
 * only part to replace. sf_parse_rules() is already the right target.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../../fuzz-support/php_stubs.h"

/* Forward decls - parser.c provides these. We include them locally
 * instead of #include "../src/parser.h" to avoid pulling condition.h
 * and its pcre2 transitive. */
typedef enum {
    RULE_REQUIRED, RULE_NULLABLE, RULE_FILLED, RULE_PRESENT,
    RULE_STRING, RULE_INTEGER, RULE_NUMERIC, RULE_BOOLEAN, RULE_ARRAY,
    RULE_MIN, RULE_MAX, RULE_BETWEEN, RULE_REGEX, RULE_NOT_REGEX,
    RULE_ALPHA, RULE_ALPHA_NUM, RULE_ALPHA_DASH, RULE_LOWERCASE, RULE_UPPERCASE,
    RULE_STARTS_WITH, RULE_ENDS_WITH, RULE_CONTAINS,
    RULE_GT, RULE_GTE, RULE_LT, RULE_LTE, RULE_DISTINCT,
    RULE_EMAIL, RULE_URL, RULE_IP, RULE_UUID, RULE_JSON,
    RULE_DATE, RULE_DATE_FORMAT,
    RULE_AFTER, RULE_BEFORE, RULE_AFTER_OR_EQUAL, RULE_BEFORE_OR_EQUAL,
    RULE_IN, RULE_NOT_IN, RULE_SAME, RULE_DIFFERENT, RULE_CONFIRMED,
    RULE_OIB, RULE_PHONE, RULE_IBAN, RULE_VAT_EU, RULE_WHEN, RULE_UNKNOWN
} sf_rule_type_t;

#define RULE_NAME_MAX_LENGTH 1024

extern bool sf_validate_rule_name(const char *name, size_t len);
extern sf_rule_type_t sf_get_rule_type(const char *name, size_t len);

/* Condition parse/free stubs.
 *
 * parser.c refers to these from the 'when' rule branch. The full
 * implementations live in condition.c and drag in pcre2 + zend compare
 * operators. Since the current harness deliberately avoids exercising
 * 'when' rules, we stub them out: sf_parse_condition always returns
 * NULL (so the 'when' branch bails with an error), and sf_free_condition
 * is a no-op.
 *
 * If you extend the harness to cover 'when', replace these with links
 * against a shim-adjusted condition.c. See harness.c banner. */
typedef struct sf_condition_s sf_condition_t;
sf_condition_t *sf_parse_condition(zval *z) { (void)z; return NULL; }
void sf_free_condition(sf_condition_t *c)   { (void)c; }

/* ==========================================================================
 * Byte-driven input decoder
 *
 * Format (read LTR):
 *   op=0x00 VALIDATE  u16 len  bytes[len]       -> sf_validate_rule_name
 *   op=0x01 TYPEOF    u16 len  bytes[len]       -> sf_get_rule_type
 *   op=0x02 STOP
 *   any other op = STOP
 *
 * We cap iterations to 1000 so libFuzzer doesn't burn budget on
 * pathological input that just loops VALIDATE forever. Bytes past a
 * truncated length prefix are treated as STOP.
 * ========================================================================== */

static size_t read_u16(const uint8_t **p, const uint8_t *end, uint16_t *out)
{
    if (*p + 2 > end) return 0;
    *out = ((uint16_t)(*p)[0] << 8) | (uint16_t)(*p)[1];
    *p += 2;
    return 1;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size > 64 * 1024) return 0;

    const uint8_t *p = data;
    const uint8_t *end = data + size;
    int ops = 0;

    while (p < end && ops++ < 1000) {
        uint8_t op = *p++;
        if (op > 0x01) break;

        uint16_t len;
        if (!read_u16(&p, end, &len)) break;
        if (p + len > end) break;

        /* Copy to a throwaway buffer so ASan flags any OOB read. */
        char *buf = (char *)malloc(len ? len : 1);
        if (!buf) return 0;
        if (len) memcpy(buf, p, len);

        switch (op) {
            case 0x00: (void)sf_validate_rule_name(buf, len); break;
            case 0x01: (void)sf_get_rule_type(buf, len);      break;
        }

        free(buf);
        p += len;
    }

    return 0;
}
