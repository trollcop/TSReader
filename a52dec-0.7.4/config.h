/* config.h for a52dec on Windows/MSVC */
#ifndef A52DEC_CONFIG_H
#define A52DEC_CONFIG_H

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `memalign' function. */
/* #undef HAVE_MEMALIGN */

/* Define to 1 if the system has the type `loff_t'. */
/* #undef HAVE_LOFF_T */

/* a52 exports */
/* #undef LIBA52_DOUBLE */

/* libao Win32 output */
#define LIBAO_WIN

/* Package info */
#define PACKAGE "a52dec"
#define VERSION "0.7.4"

/* Inline keyword */
#ifndef inline
#define inline __inline
#endif

#endif /* A52DEC_CONFIG_H */
