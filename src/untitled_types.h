#ifndef __UNTITLED__TYPES__H__
#define __UNTITLED__TYPES__H__

#include <stdint.h>

#define global_var      static
#define local_persist   static
#define internal        static

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef float  real32;
typedef double real64;

typedef uint8_t bool;
#define true 1
#define false 0

#endif
