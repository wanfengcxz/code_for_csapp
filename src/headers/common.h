//
// Created by 晚风吹行舟 on 2022/8/8.
//
#ifndef COMMON_GUARD
#define COMMON_GUARD

#include <cstdint>
#include <cstring>

#define DEBUG_INSTRUCTION_CYCLE     0X1
#define DEBUG_REGISTERS             0X2
#define DEBUG_PRINT_STACK           0X4
#define DEBUG_CACHE_DETAILS         0X8
#define DEBUG_MMU                   0X10
#define DEBUG_LINKER                0X20
#define DEBUG_LOADER                0X80
#define DEBUG_PARSE_INST            0X100

#define DEBUG_VERBOSE_SET           0X20

// do page walk
#define DEBUG_ENABLE_PAGE_WALK      0

// use static ram cache for memory access
#define DEBUG_ENABLE_SRAM_CACHE     0

// printf wrapper
uint64_t debug_printf(uint64_t open_set, const char *format, ...);

// type convert
// uint32 to its equivalent float with rounding
uint32_t uint2float(uint32_t u);

// convert string dec or hex to the integer bitmap
uint64_t string2uint(const char *str);

uint64_t string2uint_range(const char *str, int start, int end);

float uint2float_1(unsigned u);


#endif