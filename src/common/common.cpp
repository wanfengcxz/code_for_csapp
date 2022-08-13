//
// Created by 晚风吹行舟 on 2022/8/9.
//
#include <cstdio>
#include "common.h"

// P31
void show_bytes(byte_pointer start_p, size_t len) {
    size_t i;
    printf("0x");
    for (i = 0; i < len; i++) {
        printf("%.2x", start_p[i]);
    }
    printf("\n");
}

void show_int(int x) {
    printf("int: ");
    show_bytes((byte_pointer) &x, sizeof(int));
}

void show_float(float x) {
    printf("float: ");
    show_bytes((byte_pointer) &x, sizeof(float));
}

void show_pointer(void *x) {
    printf("void *: ");
    show_bytes((byte_pointer) &x, sizeof(void *));
}

// P38
void inplace_swap(int &x, int &y) {
    y = x ^ y;
    x = y ^ x;
    y = y ^ x;
}

// Arithmetic shift right
void arithmetic_sr(){
    int a = -12312;
    printf("%x\n", a); // 0xffffcfe8 -> ffff 1100 1111 1110 1000
    printf("%x", a>>2); //   0xfffff3fa -> fffff 0011 1111 1010
}
