//
// Created by 晚风吹行舟 on 2022/8/12.
//

#include <cstdio>

#include "dram.h"
#include "mmu.h"
#include "register.h"

#define SRAM_CACHE_SETTING 0

uint8_t mm[MM_LEN]; // physical memory

void print_stack() {
    int n = 10;

    uint64_t *high = (uint64_t *) &mm[va2pa(reg.rsp)];
    high = &high[n];

    uint64_t rsp_start = reg.rsp + n * 8;

    // 以rsp为起点 分别向前向后延申10个8B
    for (int i = 0; i < 2 * n; i++) {

        uint64_t *ptr = (uint64_t *) (high - i);
        printf("0x%0llx : %16llx", rsp_start, (uint64_t) *ptr);

        if (i == n) {
            printf(" <== rsp");
        }

        if (rsp_start == reg.rbp){
            printf(" <== rbp");
        }

        rsp_start = rsp_start - 8;

        printf("\n");
    }
}

uint64_t read64bits_dram(uint64_t paddr) {
    if (SRAM_CACHE_SETTING == 1) {
        // TODO: cache
        return 0x0;
    }

    uint64_t val = 0x0;

    val += (((uint64_t) mm[paddr + 0]) << 0);
    val += (((uint64_t) mm[paddr + 1]) << 8);
    val += (((uint64_t) mm[paddr + 2]) << 16);
    val += (((uint64_t) mm[paddr + 3]) << 24);
    val += (((uint64_t) mm[paddr + 4]) << 32);
    val += (((uint64_t) mm[paddr + 5]) << 40);
    val += (((uint64_t) mm[paddr + 6]) << 48);
    val += (((uint64_t) mm[paddr + 7]) << 56);

    return val;
}


void write64bits_dram(uint64_t paddr, uint64_t data) {
    if (SRAM_CACHE_SETTING == 1) {
        // TODO: cache
        return;
    } else {
        mm[paddr + 0] = (data >> 0) & 0xff;
        mm[paddr + 1] = (data >> 8) & 0xff;
        mm[paddr + 2] = (data >> 16) & 0xff;
        mm[paddr + 3] = (data >> 24) & 0xff;
        mm[paddr + 4] = (data >> 32) & 0xff;
        mm[paddr + 5] = (data >> 40) & 0xff;
        mm[paddr + 6] = (data >> 48) & 0xff;
        mm[paddr + 7] = (data >> 56) & 0xff;
    }
}