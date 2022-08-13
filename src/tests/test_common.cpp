//
// Created by 晚风吹行舟 on 2022/8/9.
//
#include <cassert>
#include <cstdio>

#include "common.h"

void test_show_bytes() {
    int ix = 2;
    float fx = (float) ix;
    int * px = &ix;
    show_int(ix);
    show_float(fx); // 0x00000040 -> 0x40000000(0x400=0100 0000 0000)
    show_pointer(px);

    fx = 0.5;
    show_float(fx); // 0x0000003f -> 0x3f000000(0x3f0=0011 1111 0000)

    char ch[5] = "abcd";
    show_bytes((byte_pointer)ch, 5);
}

void test_inplace_swap(){
    int x = 3, y = 1223;
    inplace_swap(x, y);
    assert(y == 3);
    assert(x == 1223);
}

int main() {
    test_show_bytes();
//    uint2float(2);
    return 1;

}