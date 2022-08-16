//
// Created by 晚风吹行舟 on 2022/8/13.
//
#include <cstdarg>
#include <cstdio>
#include <cassert>
#include <common.h>

// wrapper of stdio printf
// controlled by the debug verbose bit set
uint64_t debug_printf(uint64_t open_set, const char *format, ...){
    if ((open_set & DEBUG_VERB) == 0x0){
        return 0x1;
    }

    va_list  argptr;
    // format: %[flags][width][.precision][length]specifier
    // specifier: d,c,x,f...
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);

    return 0x1;
}
