/* Custom inttypes.h - originally for VC++ 6.0 which lacked <stdint.h>.
   Modern MSVC (2010+) provides these natively. */

#if defined(_MSC_VER) && _MSC_VER >= 1600
  /* VS2010+ has <stdint.h> built-in */
  #include <stdint.h>
#else
  typedef signed char int8_t;
  typedef signed short int16_t;
  typedef signed int int32_t;

  typedef unsigned char uint8_t;
  typedef unsigned short uint16_t;
  typedef unsigned int uint32_t;
#endif
