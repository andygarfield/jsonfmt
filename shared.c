#pragma once
#include "root.unity.h"
#include <stdint.h>

#define internal static

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

#define UINT2VOIDP(i) (void *)(u64)(i)
#define INT2VOIDP(i) (void *)(s64)(i)

#define false 0
#define true 1
