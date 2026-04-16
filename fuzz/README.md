# Signalforge validation (vex) parser fuzzer

libFuzzer + ASan + UBSan harness for rule-name and rule-type parsing
helpers in `../src/parser.c`:

- `sf_validate_rule_name(name, len)` - exact `^[a-z][a-z0-9_]*$`
  scanner with a 1024-byte length cap.
- `sf_get_rule_type(name, len)` - linear lookup over the 45+ entry
  rule table, returning `RULE_UNKNOWN` on miss.

## Running locally

```bash
make
make run
```

## Input format

The harness decodes fuzzer bytes as a tiny bytecode stream:

```
op       u8       0x00 = VALIDATE, 0x01 = TYPEOF, anything else = STOP
len      u16 BE   number of name bytes that follow
name     u8[len]  the rule name to test
```

libFuzzer mutates both the ops and the embedded strings, so the
harness naturally explores both well-formed and adversarial names.

## Scope and limitations

The production entry point is `sf_parse_rules(HashTable *)`, which
takes a PHP array of rule arrays. Building realistic zval arrays
from fuzzer bytes requires either:

(a) libphp-embed so we can call `zend_eval_string` on a PHP rule
    literal, OR

(b) a much deeper Zend shim that supports `zval_get_long`,
    `ZVAL_COPY`, `zend_hash_copy`, nested `IS_ARRAY` zvals, and
    `compare_function` - plus the `sf_parse_condition` layer from
    `condition.c`, which drags in libpcre2.

Neither is set up on the current CI image (Ubuntu `php8.5-dev`
ships without embed SAPI). So the current harness targets the
byte-level helpers that can be exercised without PHP state, and
links against `../src/parser.c` with local stubs for
`sf_parse_condition` / `sf_free_condition`. This covers:

- Every rule-name validation path
- Every entry in the rule-type lookup table
- The length-cap guard (1024 bytes)

**Not covered:**

- `['when', condition, then, else?]` - the conditional rule branch
- `['in', array]` / `['not_in', array]` - array-parameter rules
- The outer `sf_parse_rules()` wrapper that iterates over a field->
  rules hashmap
- Any regex parameter (`['regex', pattern]`) - but since
  `sf_parse_rules` just stores the pattern string, it's unlikely
  to harbour memory bugs here.

## TODO: full-parser harness

When libphp-embed becomes available in the build image:

1. Replace the DSL decoder in `harness.c` with a PHP fragment:
   ```c
   char *script = build_php_from_fuzzer_bytes(data, size);
   zend_eval_string(script, NULL, "fuzz");
   zval *rules = zend_hash_str_find(&EG(symbol_table), "rules", 5);
   HashTable *parsed = sf_parse_rules(Z_ARRVAL_P(rules));
   sf_free_parsed_rules_ht(parsed);
   ```
2. Link `condition.c` (with real `pcre2` from the system) instead
   of stubbing `sf_parse_condition`.
3. Drop the `-I../../fuzz-support` and use real PHP headers.

## Seed corpus

12 seeds covering:

- `required`, `email` - known-good rule names
- `min` - typeof lookup
- Empty name, digit-start, uppercase - rejection paths
- Length exactly 1024 (max) and 1025 (over)
- A stream of 8 valid rule names back-to-back
- Embedded NULs and UTF-8 bytes
- All 47 known rule names in sequence

## Current state

See the top-of-repo fuzz run report for the latest exec/sec and
coverage numbers.
