/*
 * UTF-8 string utilities
 */

#include "utf8.h"

/* Get the length of a UTF-8 string in characters (not bytes) */
size_t sf_utf8_strlen(const char *str, size_t byte_len)
{
    size_t char_count = 0;
    const unsigned char *p = (const unsigned char *)str;
    const unsigned char *end = p + byte_len;

    while (p < end) {
        /* Count only lead bytes (not continuation bytes 10xxxxxx) */
        if ((*p & 0xC0) != 0x80) {
            char_count++;
        }
        p++;
    }

    return char_count;
}

/* Check if a string is valid UTF-8 */
zend_bool sf_utf8_is_valid(const char *str, size_t byte_len)
{
    const unsigned char *p = (const unsigned char *)str;
    const unsigned char *end = p + byte_len;

    while (p < end) {
        if (*p < 0x80) {
            /* ASCII: 0xxxxxxx */
            p++;
        } else if ((*p & 0xE0) == 0xC0) {
            /* 2-byte sequence: 110xxxxx 10xxxxxx */
            if (p + 1 >= end) return 0;
            if ((p[1] & 0xC0) != 0x80) return 0;
            /* Check for overlong encoding */
            if ((*p & 0x1E) == 0) return 0;
            p += 2;
        } else if ((*p & 0xF0) == 0xE0) {
            /* 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx */
            if (p + 2 >= end) return 0;
            if ((p[1] & 0xC0) != 0x80) return 0;
            if ((p[2] & 0xC0) != 0x80) return 0;
            /* Check for overlong encoding */
            if (*p == 0xE0 && (p[1] & 0x20) == 0) return 0;
            /* Check for surrogate pairs */
            if (*p == 0xED && (p[1] & 0x20) != 0) return 0;
            p += 3;
        } else if ((*p & 0xF8) == 0xF0) {
            /* 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
            if (p + 3 >= end) return 0;
            if ((p[1] & 0xC0) != 0x80) return 0;
            if ((p[2] & 0xC0) != 0x80) return 0;
            if ((p[3] & 0xC0) != 0x80) return 0;
            /* Check for overlong encoding */
            if (*p == 0xF0 && (p[1] & 0x30) == 0) return 0;
            /* Check for code points > U+10FFFF */
            if (*p == 0xF4 && p[1] > 0x8F) return 0;
            if (*p > 0xF4) return 0;
            p += 4;
        } else {
            /* Invalid lead byte */
            return 0;
        }
    }

    return 1;
}

/* Get byte offset for a character position */
size_t sf_utf8_char_to_byte_offset(const char *str, size_t byte_len, size_t char_pos)
{
    const unsigned char *p = (const unsigned char *)str;
    const unsigned char *end = p + byte_len;
    size_t char_count = 0;

    while (p < end && char_count < char_pos) {
        if ((*p & 0xC0) != 0x80) {
            char_count++;
        }
        p++;
    }

    return (size_t)(p - (const unsigned char *)str);
}
