//
// Created by 晚风吹行舟 on 2022/8/8.
//

#include <cassert>
#include <cstdio>
#include <cstdint>
#include "bit_op.h"

#define DEBUG 1


void test_get_negative() {
    int a = 2147483647; // max int
    assert(get_negative(a) == -a);
    assert(get_negative(-123) == 123);
#ifdef DEBUG
    printf("test_get_negative --------\n");
    printf("a=%d\n-a=%d\n\n", a, get_negative(a));
#endif
}

void test_low_bit() {
    assert(low_bit(0xff) == 1); // 0xff(1111 1111) -> 1(十进制为1)
    assert(low_bit(0xa) == 2); // 0xa(1010) -> 10(十进制为2)
}

void test_hex_all_letter() {
    /*
     * 对于每一个16进制数，都对应四位二进制数 x3x2x1x0
     * 其中x3可以通过 x3x2x1x0 & 1000 取出来
     * x2可以通过 x3x2x1x0 & 0100 取出来
     * x1可以通过 x3x2x1x0 & 0010 取出来
     */
    unsigned x = 0xaacd;

    // 取出x3 x2 x1
    unsigned x3 = x & 0x88888888;
    unsigned x2 = x & 0x44444444;
    unsigned x1 = x & 0x22222222;

    // 对于每一位十六进制都要进行 x3 & (x2 + x1)
    // 通过位移来对齐 即 x3>>3 x2>>2 x1>>1
    unsigned a = (x3 >> 3) & (x2 >> 2 | x1 >> 1);

    uint64_t ans1 = hex_all_letter((uint64_t)x);
    uint64_t ans2 = hex_all_letter(0xaaaaacbaaaaaaaaa);

#ifdef DEBUG
    printf("analysis ---------\n");
    printf("x = 0x%x\n", x);
    printf("x3 = 0x%x,\nx2 = 0x%x,\nx1 = 0x%x.\n", x3, x2, x1);
    printf("a = 0x%x\n", a);

    printf("test_hex_all_letter --------\n");
    printf("ans1: %llu\nans2: %llu\n", ans1, ans2);
#endif

}

int main() {
    test_get_negative();
    test_low_bit();
    test_hex_all_letter();
//    printf("%d", x & 0x7f800000);
    return 1;
}
