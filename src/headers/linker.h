//
// Created by 晚风吹行舟 on 2022/9/1.
//

#ifndef LINKER_GUARD
#define LINKER_GUARD

#include <cstdint>
#include <cstdlib>

#define MAX_CHAR_SECTION_NAME (32)
typedef struct {

    char sh_name[MAX_CHAR_SECTION_NAME];
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;

} sh_entry_t;   // section table entry

#define MAX_CHAR_SYMBOL_NAME (64)

typedef enum {
    STB_LOCAL,
    STB_GLOBAL,
    STB_WEAK
} st_bind_t;

typedef enum {
    STT_NOTYPE,
    STT_OBJECT,
    STT_FUNC
} st_type_t;

typedef struct {
    char st_name[MAX_CHAR_SYMBOL_NAME];
    st_bind_t bind;
    st_type_t type;
    char st_shndx[MAX_CHAR_SECTION_NAME];   // section table name of the current symbol
    uint64_t st_value;      // in-section offset
    uint64_t st_size;       // count of lines of symbol
} st_entry_t;   // symbol table entry

#define MAX_ELF_FILE_LENGTH (64)
#define MAX_ELF_FILE_WIDTH (128)

typedef struct {

    char buffer[MAX_ELF_FILE_LENGTH][MAX_ELF_FILE_WIDTH];
    uint64_t line_count;
    uint64_t sht_count;
    sh_entry_t *sht;

    uint64_t symt_count;
    st_entry_t *symt;

} elf_t;

void parse_elf(char *filename, elf_t *elf);
void free_elf(elf_t *elf);

#endif