/*
 * UTF-8 string utilities
 */

#ifndef SIGNALFORGE_UTF8_H
#define SIGNALFORGE_UTF8_H

#include "php.h"

/* Get the length of a UTF-8 string in characters (not bytes) */
size_t sf_utf8_strlen(const char *str, size_t byte_len);

/* Check if a string is valid UTF-8 */
zend_bool sf_utf8_is_valid(const char *str, size_t byte_len);

/* Get byte offset for a character position */
size_t sf_utf8_char_to_byte_offset(const char *str, size_t byte_len, size_t char_pos);

#endif /* SIGNALFORGE_UTF8_H */
