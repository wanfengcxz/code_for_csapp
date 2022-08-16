//
// Created by 晚风吹行舟 on 2022/8/12.
//

#ifndef MEMORY_GUARD
#define MEMORY_GUARD

#include <cstdint>
#include "cpu.h"

/*======================================*/
/*    physical memory on dram chips     */
/*======================================*/

// physical memory space is decided by the physical address
// in this simulator, there are 4 + 6 + 6 = 16 bit physical address
// then the physical space is (1 << 16) = 65536
// total 65536B physical memory
#define PHYSICAL_MEMORY_SPACE   65536
#define MAX_PHYSICAL_BIT 16

// physical memory
// 16 physical memory pages ???
extern uint8_t pm[PHYSICAL_MEMORY_SPACE]; // physical memory

/*======================================*/
/*            memory R/W                */
/*======================================*/

uint64_t read64bits_dram(uint64_t paddr, core_t *cr);
void write64bits_dram(uint64_t paddr, uint64_t data, core_t *cr);

#endif