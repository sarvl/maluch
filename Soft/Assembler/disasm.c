#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// CANT COMPILE WITH MSVC!!!!!!!

typedef struct Insn {
    uint16_t src_reg : 4;
    uint16_t dst_reg : 4;
    uint16_t functor : 3;
    uint16_t immediate : 1;
    uint16_t opcode : 4;
} Insn;

char *name_lut[] = {
               /* 000 */   /* 001 */  /* 010 */  /* 011 */  /* 100 */  /* 101 */  /* 110 */  /* 111 */
    /* 0000 */ "invalid", "invalid", "invalid", "invalid", "invalid", "invalid", "invalid", "invalid",
    /* 0001 */ "mov",     "mov",     "mov",     "mov",     "mov",     "mov",     "mov",     "mov",
    /* 0010 */ "add",     "sub",     "and",     "or",      "xor",     "not",     "lsl",     "lsr",
    /* 0011 */ "rsrvd",   "cmp",     "test",    "rsrvd",   "rsrvd",   "rsrvd",   "rsrvd",   "rsrvd",
    /* 0100 */ "jmp",     "bee",     "bne",     "bge",     "ble",     "bgg",     "bll",     "boo", 
    /* 0101 */ "bbs",     "bss",     "bns",     "bae",     "bbe",     "baa",     "bbb",     "bno",
    /* 0110 */ "in",      "in",      "in",      "in",      "in",      "in",      "in",      "in", 
    /* 0111 */ "out",     "out",     "out",     "out",     "out",     "out",     "out",     "out", 
    /* 1000 */ "ldw",     "ldw",     "ldw",     "ldw",     "ldw",     "ldw",     "ldw",     "ldw", 
    /* 1001 */ "stw",     "stw",     "stw",     "stw",     "stw",     "stw",     "stw",     "stw", 
    /* 1010 */ "call",    "call",    "call",    "call",    "call",    "call",    "call",    "call",
    /* 1011 */ "ret",     "rsrvd",   "rsrvd",   "rsrvd",   "rsrvd",   "rsrvd",   "rsrvd",   "rsrvd",
    /* 1100 */ "push",    "push",    "push",    "push",    "push",    "push",    "push",    "push", 
    /* 1101 */ "pull",    "pull",    "pull",    "pull",    "pull",    "pull",    "pull",    "pull", 
    /* 1110 */ "rsrvd",   "rsrvd",   "rsrvd",   "rsrvd",   "rsrvd",   "rsrvd",   "rsrvd",   "rsrvd", 
    /* 1111 */ "rsrvd",   "rsrvd",   "rsrvd",   "rsrvd",   "rsrvd",   "rsrvd",   "rsrvd",   "rsrvd", 
};

char *devices_lut[] = {
    "timer", "kb", "gpu", "pstor", "rsrvd", "rsrvd", "rsrvd", "rsrvd"
};

// TODO: add string heurestics i dunno
int main(int argc, char *argv[argc]) {
    if(argc != 2) {
        printf("USAGE: %s path/to/file\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE* f = fopen(argv[1], "rb");
    if(!f) {
        fprintf(stderr, "ERROR: Couldn't open file %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    fseek(f, 0, SEEK_END);
    const size_t sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    Insn buf[sz/2];
    fread(buf, 2, sz/2, f);
    fclose(f);

    uint16_t *const a = (uint16_t*)buf;
    // SWAP BYTES
    for(size_t i = 0; i < sizeof(buf)/2; i++) {
        a[i] = __builtin_bswap16((uint16_t) a[i]);
    }

    char out_name[1024];
    sprintf(out_name, "%s.disasm.txt", argv[1]);
    f = fopen(out_name, "w");

#define ADD_DST()                     \
    fprintf(f, "r%d", buf[i].dst_reg)

#define ADD_SEP()     \
    fprintf(f, ", ");

#define ADD_SRC()                                      \
    do {                                               \
        if(buf[i].immediate) {                         \
            fprintf(f, "$%04X", a[i+1]);               \
        } else {                                       \
            fprintf(f, "r%d", buf[i].src_reg);         \
        }                                              \
    } while(0)

#define ADD_DEV()                                 \
    fprintf(f, "%s", devices_lut[buf[i].functor])

    for(size_t i = 0; i < sizeof(buf)/2; i++) {
        fprintf(f, "%04lX:", i);
        fprintf(f, "    %s ", name_lut[buf[i].opcode * 8 +buf[i].functor]);

        // BUG: will segfault if the final value in the binary can be interpreted as an instruction with an immediate value
        switch(buf[i].opcode) {
            case 0x1:
            case 0x2:
            case 0x3:
            case 0x8:
                ADD_DST();
                ADD_SEP();
                ADD_SRC();
                break;
            case 0x4:
            case 0x5:
            case 0xA:
            case 0xC:
            case 0xD:
                ADD_SRC();
                break;
            case 0x6:
                ADD_DST();
                ADD_SEP();
                ADD_DEV();
                break;
            case 0x7:
                ADD_DEV();
                ADD_SEP();
                ADD_SRC();
                break;
            case 0x9:
                ADD_SRC();
                ADD_SEP();
                ADD_DST();
                break;
            default: break;
        }
        fprintf(f, "\n");

        if(buf[i].immediate) i++;
    }

    return 0;
}
