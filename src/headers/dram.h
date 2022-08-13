//
// Created by 晚风吹行舟 on 2022/8/12.
//

#ifndef dram_guard
#define dram_guard

#include <cstdint>

#define MM_LEN 1000

extern uint8_t mm[MM_LEN]; // physical memory


void print_stack();

uint64_t read64bits_dram(uint64_t paddr);
void write64bits_dram(uint64_t paddr, uint64_t data);

#endif