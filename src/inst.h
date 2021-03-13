#ifndef INST_H
#define INST_H

#include <stdint.h>

#include "cpu.h"

typedef void (*inst_handler)(struct cpu *c, uint32_t inst);

struct inst_info {
    char *bit_pattern;
    inst_handler func;
};

extern const struct inst_info inst_list[];

#endif
