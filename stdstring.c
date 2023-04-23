#include <ctype.h>
#include <stdlib.h>
#include "stdstring.h"

char *string_init(const char *src)
{
    return src ? strdup(src) : NULL;
}

char *string_to_upper(char *s)
{
    char *cs = (char *)s;
    for (; *cs != '\0'; cs++) {
        *cs = toupper(*cs);
    }
    return s;
}

char *string_to_lower(char *s)
{
    char *cs = (char *)s;
    for (; *cs != '\0'; cs++) {
        *cs = tolower(*cs);
    }
    return s;
}

char *string_ucwords(char *s)
{
   char *cs = (char *)s;
   for ( ; *cs != '\0'; cs++)
   {
      if (*cs == ' ')
         *(cs+1) = toupper(*(cs+1));
   }

   s[0] = toupper(s[0]);
   return s;
}

char *string_trim_left(char *s)
{
    if (s && *s) {
        size_t len = strlen(s);
        char *current = s;

        while(*current && isspace(*current)) {
            ++current;
            --len;
        }

        if (s != current)
            memmove(s, current, len+1);
    }
    return s;
}

char *string_trim_right(char *s)
{
    if(s && *s) {
        size_t len = strlen(s);
        char *current = s + len -1;

        while(current != s && isspace(*current)) {
            --current;
            --len;
        }
        current[isspace(*current) ? 0:1] = '\0';
    }
    return s;
}

char *string_trim(char *s)
{
    string_trim_right(s);
    string_trim_left(s);

    return s;
}

int string_split(const char *s, const char *delim, char *parts[], size_t count)
{
    int i = 0;
    char *copy = NULL, *tmp = NULL;

    if (!(copy = strdup(s)))
        return -1;
    
    char *rest = copy;
    char *token = strtok_r(rest, delim, &rest);
    if (!token) {
        free(copy);
        return 0;
    }
    
    if (!(tmp = strdup(token)))
        goto ret;

    parts[i++] = tmp;

    while(token && i < count) {
        token = strtok_r(NULL, delim, &rest);
        if (!token)
            break;
        if (!(tmp = strdup(token)))
            goto ret;
        parts[i++] = tmp;
    }

    free(copy);
    return i;

ret:
    free(copy);
    for(int j=0; j<i; j++)
        free(parts[j]);
    return -1;
}