//
// Created by 晚风吹行舟 on 2022/9/1.
//

#include "headers/linker.h"



int main(){

    elf_t src[2];
    parse_elf("../files/sum.elf.txt", &src[0]);
    parse_elf("../files/main.elf.txt", &src[1]);

    elf_t dst;
    elf_t *srcp[2];
    srcp[0] = &src[0];
    srcp[1] = &src[1];
    link_elf((elf_t **)&srcp, 2, &dst);

    free_elf(&src[0]);
    free_elf(&src[1]);


    return 0;
}
