//
// Created by 晚风吹行舟 on 2022/8/8.
//

#include "headers/common.h"
#include <cstdio>
#include <cstdlib>

// covert string to int64_t
uint64_t string2uint(const char *str) {
    return string2uint_range(str, 0, -1);
}

uint64_t string2uint_range(const char *str, int start, int end) {
    // 123456 0x12345 -12345 -0x12345
    // [start, end](inclusive)
    end = (end == -1) ? strlen(str) - 1 : end;
    uint64_t unsigned_v = 0;
    int sign_bit = 0;     // 0 +  1 -

    // DFA
    int state = 0;
    for (int i = start; i <= end; ++i) {
        char c = str[i];
        if (state == 0) {
            if (c == '0') {
                state = 1;
                unsigned_v = 0;
                continue;
            } else if ('1' <= c && c <= '9') {
                state = 2;
                unsigned_v = c - '0';
                continue;
            } else if (c == '-') {
                sign_bit = 1;
                state = 3;
                continue;
            } else if (c == ' ') {
                state = 0;
                continue;
            } else {
                goto fail;
            }
        } else if (state == 1) {
            if (c == 'x' || c == 'X') {
                state = 4;
                continue;
            } else if ('0' <= c && c <= '9') {
                state = 2;
                unsigned_v = unsigned_v * 10 + c - '0';
                continue;
            } else if (c == ' ') {
                state = 6;
                continue;
            } else {
                goto fail;
            }
        } else if (state == 2) {
            if ('0' <= c && c <= '9') {
                state = 2;
                uint64_t pre_v = unsigned_v;
                unsigned_v = unsigned_v * 10 + c - '0';
                if (pre_v > unsigned_v) {
                    printf("(uint64_t)%s overflow: cannot convert\n", str);
                    goto fail;
                }
            } else if (c == ' ') {
                state = 6;
                continue;
            } else {
                goto fail;
            }
        } else if (state == 3) {
            if (c == '0') {
                state = 1;
                continue;
            } else if ('1' <= c && c <= '9') {
                state = 2;
                unsigned_v = c - '0';
                continue;
            } else {
                goto fail;
            }
        } else if (state == 4) {
            if ('0' <= c && c <= '9') {
                state = 5;
                unsigned_v = unsigned_v * 16 + c - '0';
                continue;
            } else if ('a' <= c && c <= 'f') {
                state = 5;
                unsigned_v = unsigned_v * 16 + c - 'a' + 10;
                continue;
            } else {
                goto fail;
            }
        } else if (state == 5) {
            if ('0' <= c && c <= '9') {
                state = 5;
                uint64_t pre_v = unsigned_v;
                unsigned_v = unsigned_v * 16 + c - '0';
                // maybe overflow
                if (pre_v > unsigned_v) {
                    printf("(uint64_t)%s overflow: cannot convert\n", str);
                    goto fail;
                }
            } else if ('a' <= c && c <= 'f') {
                state = 5;
                uint64_t pre_v = unsigned_v;
                unsigned_v = unsigned_v * 16 + c - 'a' + 10;
                // maybe overflow
                if (pre_v > unsigned_v) {
                    printf("(uint64_t)%s overflow: cannot convert\n", str);
                    goto fail;
                }
            } else {
                goto fail;
            }
        } else if (state == 6) {
            if (c == ' ') {
                state = 6;
                continue;
            } else {
                goto fail;
            }
        }
    }
    if (sign_bit == 0) {
        return unsigned_v;
    } else {
        if ((unsigned_v & 8000000000000000) != 0) {
            printf("(int64_t)%s overflow: cannot convert\n", str);
            goto fail;
        }
        int64_t sv = -1 * (int64_t) unsigned_v;
        return *(uint64_t *) &sv;
    }
    fail:
    printf("type converter: <%s> cannot be converted to integer\n", str);
    exit(0);
}

float uint2float_1(unsigned u) {
    return 0.0;
}
