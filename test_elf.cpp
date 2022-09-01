//
// Created by 晚风吹行舟 on 2022/9/1.
//

#include "headers/linker.h"



int main(){

    elf_t elf;
    parse_elf("../files/sum.elf.txt", &elf);
    free_elf(&elf);

    return 0;
}
