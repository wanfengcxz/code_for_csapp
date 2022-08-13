//
// Created by 晚风吹行舟 on 2022/8/8.
//

float uint2float(unsigned u);


// show bytes
typedef unsigned char *byte_pointer;

void show_bytes(byte_pointer start_p, size_t len);

void show_int(int x);

void show_float(float x);

void show_pointer(void *x);

void inplace_swap(int &x, int &y);