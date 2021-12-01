#ifndef __SWITCHS_H__
#define __SWITCHS_H__

#include <string.h>
#include <regex.h>
#include <stdbool.h>

/** Begin a switch for the string x */
#define switchs(x) \
    { char *__sw = (x); bool __done = false; bool __cont = false; \
        regex_t __regex; regcomp(&__regex, ".*", 0); do {

/** Check if the string matches the cases argument (case sensitive) */
#define cases(x)    } if ( __cont || !strcmp ( __sw, x ) ) \
                        { __done = true; __cont = true;

/** Check if the string matches the icases argument (case insensitive) */
#define icases(x)    } if ( __cont || !strcasecmp ( __sw, x ) ) { \
                        __done = true; __cont = true;

/** Check if the string matches the specified regular expression using regcomp(3) */
#define cases_re(x,flags) } regfree ( &__regex ); if ( __cont || ( \
                              0 == regcomp ( &__regex, x, flags ) && \
                              0 == regexec ( &__regex, __sw, 0, NULL, 0 ) ) ) { \
                                __done = true; __cont = true;

/** Default behaviour */
#define defaults    } if ( !__done || __cont ) {

/** Close the switchs */
#define switchs_end } while ( 0 ); regfree(&__regex); }

#endif // __SWITCHS_H__

