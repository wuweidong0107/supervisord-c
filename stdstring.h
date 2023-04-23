#ifndef __STDSTRING_H__
#define __STDSTRING_H__

#include <string.h>
#include <stdbool.h>

static inline bool string_is_empty(const char *data)
{
    return !data || (*data == '\0');
}

static inline bool string_is_equal(const char *a, const char *b)
{
    return (a && b) ? !strcmp(a, b) : false;
}

static inline bool string_is_equal_case(const char *a, const char *b)
{
    return (a && b) ? !strcasecmp(a, b) : false;
}

static inline bool string_starts_with(const char *str, const char *prefix)
{
    return (str && prefix) ? !strncmp(prefix, str, strlen(prefix)) : false;
}

static inline bool string_starts_with_case(const char *str, const char *prefix)
{
    return (str && prefix) ? !strncasecmp(prefix, str, strlen(prefix)) : false;
}

static inline bool string_ends_with(const char *str, const char *suffix)
{
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    return str && suffix && \
        (str_len < suffix_len) ? false : !strncmp(suffix, str + (str_len-suffix_len), suffix_len);
}

static inline bool string_ends_with_case(const char *str, const char *suffix)
{
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    return str && suffix && \
        (str_len < suffix_len) ? false : !strncasecmp(suffix, str + (str_len-suffix_len), suffix_len);
}

char *string_init(const char *src);
char *string_to_upper(char *s);
char *string_to_lower(char *s);
char *string_ucwords(char *s);

char *string_trim_left(char *s);
char *string_trim_right(char *s);
char *string_trim(char *s);

int string_split(const char *s, const char *delim, char *parts[], size_t count);
#endif