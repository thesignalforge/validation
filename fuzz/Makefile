# Signalforge validation (vex) rule-parser fuzzer
#
# Scope-limited: exercises sf_validate_rule_name() and sf_get_rule_type()
# directly via a byte-DSL, and links parser.c + the shim Zend API.
# Does NOT exercise the 'when' conditional branch or 'in'/'not_in' rules
# that require a full zval runtime. See harness.c banner.

CC       := clang
SHIM     := ../../fuzz-support
SRCS     := harness.c \
            $(SHIM)/php_stubs.c \
            ../src/parser.c
CFLAGS   := -fsanitize=fuzzer,address,undefined \
            -fno-sanitize-recover=all \
            -fno-omit-frame-pointer \
            -g -O1 -Wall \
            -I$(SHIM) -I../src
LDFLAGS  := -L/usr/lib/gcc/x86_64-linux-gnu/13
RUN_ARGS := corpus/ -print_final_stats=1 -artifact_prefix=artifacts/

harness: $(SRCS) $(SHIM)/php_stubs.h
	@mkdir -p artifacts
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)

run: harness
	SF_FUZZ_QUIET=1 ./harness $(RUN_ARGS) -max_total_time=300

run-long: harness
	SF_FUZZ_QUIET=1 ./harness $(RUN_ARGS) -max_total_time=3600

clean:
	rm -f harness
	rm -rf artifacts

.PHONY: run run-long clean
