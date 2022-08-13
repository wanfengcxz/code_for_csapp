//
// Created by 晚风吹行舟 on 2022/8/8.
//
#include "bit_op.h"

// 求出-x
int get_negative(int x) {
    return (~x) + 1;
}

/*
在二进制下，从右往左寻找正数x中第一个1，然后截断返回其值
解释：
    x          ~x        -x
aaaaaaaa1  bbbbbbbb0  bbbbbbbb1
   x&(-x)
000000001

      x             ~x             -x
aaaaaaaa10000  bbbbbbbb01111  bbbbbbbb10000
   x&(-x)
0000000010000

----- eg. -----
x:01010
-x原:11010
-x反:00101
-x补:00110
x & (-x):10
*/
unsigned low_bit(unsigned x) {
    unsigned a = x & ((~x) + 1);
    return a;
}


/*
 * 判断以16进制表示的数中是否全为字母
 * eg: 0xabc1 -> false; 0xacbdd -> ture;
 */
uint64_t hex_all_letter(uint64_t x) {
    // TODO: 这个函数有点问题
    uint64_t x1 = (x & 0x2222222222222222) >> 1;
    uint64_t x2 = (x & 0x4444444444444444) >> 2;
    uint64_t x3 = (x & 0x8888888888888888) >> 3;

    x = x3 & (x1 | x2);
    x = x & (x >> 32);
    x = x & (x >> 16);
    x = x & (x >>  8);
    x = x & (x >>  4);
    return x;
}




