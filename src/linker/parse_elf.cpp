//
// Created by 晚风吹行舟 on 2022/9/1.
//

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cassert>

#include "headers/linker.h"
#include "headers/common.h"


static int parse_table_entry(char *str, char ***ent) {

    int count_col = 1;
    int len = strlen(str);

    for (int i = 0; i < len; i++) {
        if (str[i] == ',')
            count_col++;
    }

    char **arr = (char **) malloc(count_col * sizeof(char *));
    *ent = arr;

    int col_index = 0;
    int col_width = 0;
    char col_buf[32];
    for (int i = 0; i < len + 1; i++) {
        if (str[i] == ',' || str[i] == '\0') {
            assert(col_index < count_col);

            // malloc and copy
            char *col = (char *) malloc((col_width + 1) * sizeof(char));
            for (int j = 0; j < col_width; ++j) {
                col[j] = col_buf[j];
            }
            col[col_width] = '\0';

            arr[col_index] = col;
            col_index++;
            col_width = 0;
        } else {
            assert(col_width < 32);
            col_buf[col_width] = str[i];
            col_width++;
        }
    }

    return count_col;
}

static void free_table_entry(char **ent, int n) {
    for (int i = 0; i < n; i++) {
        free(ent[i]);
    }
    free(ent);
}

static void parse_sh(char *str, sh_entry_t *sh) {
    // .text,0x0,4,22
    char **cols;
    int num_cols = parse_table_entry(str, &cols);
    assert(num_cols == 4);
    assert(sh != NULL);

    strcpy(sh->sh_name, cols[0]);
    sh->sh_addr = string2uint(cols[1]);
    sh->sh_offset = string2uint(cols[2]);
    sh->sh_size = string2uint(cols[3]);

    free_table_entry(cols, num_cols);
}

static void print_sh_entry(sh_entry_t *sh) {
    debug_printf(DEBUG_LINKER, "%s\t", sh->sh_name);
    debug_printf(DEBUG_LINKER, "%x\t", sh->sh_addr);
    debug_printf(DEBUG_LINKER, "%d\t", sh->sh_offset);
    debug_printf(DEBUG_LINKER, "%d\n", sh->sh_size);
}

static void parse_symtab(char *str, st_entry_t *ste) {
    // sum,STB_GLOBAL,STT_FUNCTION,.text,0,22
    char **cols;
    int num_cols = parse_table_entry(str, &cols);
    assert(num_cols == 6);

    assert(ste != NULL);
    strcpy(ste->st_name, cols[0]);

    // select symbol bind
    if (strcmp(cols[1], "STB_LOCAL") == 0) {
        ste->bind = STB_LOCAL;
    } else if (strcmp(cols[1], "STB_GLOBAL") == 0) {
        ste->bind = STB_GLOBAL;
    } else if (strcmp(cols[1], "STB_WEAK") == 0) {
        ste->bind = STB_WEAK;
    } else {
        printf("symbol bind is neiter LOCAL, GLOBAL, nor WEAK\n");
        exit(0);
    }

    // select symbol type
    if (strcmp(cols[2], "STT_NOTYPE") == 0) {
        ste->type = STT_NOTYPE;
    } else if (strcmp(cols[2], "STT_OBJECT") == 0) {
        ste->type = STT_OBJECT;
    } else if (strcmp(cols[2], "STT_FUNCTION") == 0) {
        ste->type = STT_FUNC;
    } else {
        printf("symbol type is neiter NOTYPE, OBJECT, nor FUNC\n");
        exit(0);
    }

    strcpy(ste->st_shndx, cols[3]);

    ste->st_value = string2uint(cols[4]);
    ste->st_size = string2uint(cols[5]);

    free_table_entry(cols, num_cols);
}

static void print_symtab_entry(st_entry_t *ste) {
    debug_printf(DEBUG_LINKER, "%s\t", ste->st_name);
    debug_printf(DEBUG_LINKER, "%d\t", ste->bind);
    debug_printf(DEBUG_LINKER, "%d\t", ste->type);
    debug_printf(DEBUG_LINKER, "%s\t", ste->st_shndx);
    debug_printf(DEBUG_LINKER, "%d\t", ste->st_value);
    debug_printf(DEBUG_LINKER, "%d\n", ste->st_size);
}

static int read_elf(const char *file_name, uint64_t buf_addr) {

    FILE *fp;
    fp = fopen(file_name, "r");
    if (fp == NULL) {
        debug_printf(DEBUG_LINKER, "unable to open file %s\n", file_name);
        exit(1);
    }

    // read text file line by line
    char line[MAX_ELF_FILE_WIDTH];
    int line_counter = 0;

    while (fgets(line, MAX_ELF_FILE_WIDTH, fp) != NULL) {
        int len = strlen(line);
        if ((len == 0) ||
            (len >= 1 && (line[0] == '\n' || line[0] == '\r')) ||
            (len >= 2 && (line[0] == '/' && line[1] == '/'))) {
            continue;
        }

        // check if is empty or white line
        int iswhite = 1;
        for (int i = 0; i < len; ++i) {
            iswhite = iswhite && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r');
        }
        if (iswhite == 1) {
            continue;
        }
        if (line_counter < MAX_ELF_FILE_LENGTH) {
            // store this line to buffer[line_counter]
            uint64_t addr = buf_addr + line_counter * MAX_ELF_FILE_WIDTH * sizeof(char);
            char *line_buf = (char *) addr;

            int i = 0;
            while (i < len && i < MAX_ELF_FILE_WIDTH) {
                if ((line[i] == '\n') ||
                    (line[i] == '\r') ||
                    ((i + 1 < len) && (i + 1 < MAX_ELF_FILE_WIDTH) && line[i] == '/' && line[i + 1] == '/')) {
                    break;
                }
                line_buf[i] = line[i];
                i++;
            }
            line_buf[i] = '\0';
            line_counter++;
            printf("%\s\n", line_buf);

        } else {
            debug_printf(DEBUG_LINKER, "elf file %s is too long ", file_name);
            debug_printf(DEBUG_LINKER, "(>%d)\n", MAX_ELF_FILE_LENGTH);
            fclose(fp);
            exit(1);
        }

    }
    fclose(fp);
    assert(string2uint((char *) buf_addr) == line_counter);
    return line_counter;
}

void parse_elf(char *file_name, elf_t *elf) {
    assert(elf != NULL);
    int line_count = read_elf(file_name, (uint64_t) (&(elf->buffer)));
    for (int i = 0; i < line_count; ++i) {
        printf("[%d]\t%s\n", i, elf->buffer[i]);
    }

    // parse section headers
    int sh_count = string2uint(elf->buffer[1]);
    elf->sht = (sh_entry_t *) malloc(sh_count * sizeof(sh_entry_t));

    sh_entry_t *symt_sh = NULL;
    for (int i = 0; i < sh_count; i++) {
        parse_sh(elf->buffer[2 + i], &(elf->sht[i]));
        print_sh_entry(&(elf->sht[i]));

        if (strcmp(elf->sht[i].sh_name, ".symtab") == 0) {
            // this is the section header for symbol table
            symt_sh = &(elf->sht[i]);
        }
    }

    assert(symt_sh != NULL);

    elf->symt_count = symt_sh->sh_size;
    elf->symt = (st_entry_t *) malloc(elf->symt_count * sizeof(st_entry_t));
    for (uint64_t i = 0; i < symt_sh->sh_size; i++) {
        parse_symtab(elf->buffer[i + symt_sh->sh_offset],&elf->symt[i]);
        print_symtab_entry(&elf->symt[i]);
    }
}

void free_elf(elf_t *elf)
{
    assert(elf != NULL);

    free(elf->sht);
}