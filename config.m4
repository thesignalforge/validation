PHP_ARG_ENABLE([signalforge_validation],
  [whether to enable signalforge_validation support],
  [AS_HELP_STRING([--enable-signalforge-validation],
    [Enable signalforge_validation support])],
  [no])

if test "$PHP_SIGNALFORGE_VALIDATION" != "no"; then
  dnl Check for PCRE2
  PKG_CHECK_MODULES([PCRE2], [libpcre2-8 >= 10.30])

  PHP_EVAL_INCLINE($PCRE2_CFLAGS)
  PHP_EVAL_LIBLINE($PCRE2_LIBS, SIGNALFORGE_VALIDATION_SHARED_LIBADD)

  PHP_NEW_EXTENSION(signalforge_validation,
    signalforge_validation.c \
    src/validator.c \
    src/result.c \
    src/parser.c \
    src/condition.c \
    src/wildcard.c \
    src/rules/presence.c \
    src/rules/types.c \
    src/rules/string.c \
    src/rules/numeric.c \
    src/rules/array.c \
    src/rules/format.c \
    src/rules/comparison.c \
    src/rules/regional.c \
    src/util/utf8.c \
    src/util/memory.c,
    $ext_shared)

  PHP_ADD_BUILD_DIR($ext_builddir/src)
  PHP_ADD_BUILD_DIR($ext_builddir/src/rules)
  PHP_ADD_BUILD_DIR($ext_builddir/src/util)

  PHP_SUBST(SIGNALFORGE_VALIDATION_SHARED_LIBADD)
fi
