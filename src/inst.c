#include "inst.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "bitpat.h"
#include "cpu.h"
#include "log.h"

static uint32_t get_bits(uint32_t t, int s, int e)
{
    int bit_len = e - s;
    uint32_t bit_mask = 1;
    for (int i = 0; i < bit_len; i++) {
        bit_mask = (bit_mask << 1) + 1;
    }
    return (t >> s) & bit_mask;
}

static uint16_t sext(uint8_t nbits, uint16_t t)
{
    uint8_t sign_bit = nbits - 1;
    return t | (((t >> sign_bit) & 1) == 0 ? /* positive */ 0u
                                           : /* negative */ ~0u << nbits);
}

static uint16_t asr(uint16_t lhs, uint16_t rhs)
{
    assert(rhs <= 16 && "Too large rhs");

    // NOTE: According to N1548, the value of (((int32_t)lhs) >> rhs) is
    // implementation-defined if lhs is negative.
    return (lhs >> 15) == 0
               ? /* positive */ (uint16_t)(lhs >> rhs)
               : /* negative */ (~0u << (16u - rhs)) | (lhs >> rhs);
}

static const char *reg2str(int regno)
{
    switch (regno) {
    case 0: return "x0";
    case 1: return "x1";
    case 2: return "x2";
    case 3: return "x3";
    case 4: return "x4";
    case 5: return "x5";
    case 6: return "x6";
    case 7: return "x7";
    case 8: return "x8";
    case 9: return "x9";
    case 10: return "x10";
    case 11: return "x11";
    case 12: return "x12";
    case 13: return "x13";
    case 14: return "x14";
    case 15: return "x15";
    case 16: return "x16";
    case 17: return "x17";
    case 18: return "x18";
    case 19: return "x19";
    case 20: return "x20";
    case 21: return "x21";
    case 22: return "x22";
    case 23: return "x23";
    case 24: return "x24";
    case 25: return "x25";
    case 26: return "x26";
    case 27: return "x27";
    case 28: return "x28";
    case 29: return "x29";
    case 30: return "x30";
    case 31: return "x31";
    }

    assert(0 && "Invalid register index!");
}

static uint16_t pick_uimm11(uint32_t inst) { return get_bits(inst, 21, 31); }

static uint16_t pick_simm11(uint32_t inst)
{
    return sext(11, pick_uimm11(inst));
}

static uint16_t pick_simm11_store(uint32_t inst)
{
    return sext(11, (get_bits(inst, 26, 31) << 5) | get_bits(inst, 16, 20));
}

static uint16_t pick_simm13(uint32_t inst)
{
    return sext(13, (get_bits(inst, 18, 20) << 2) |
                        (get_bits(inst, 26, 30) << 5) |
                        (get_bits(inst, 16, 17) << 10) |
                        (get_bits(inst, 31, 31) << 12));
}

static uint16_t pick_uimm4(uint32_t inst) { return get_bits(inst, 21, 24); }

// src0 = src1 op src2
#define DEFINE_INST_ARITH(inst_name, op, src0_expr, src1_expr, src2_expr,    \
                          lhs_expr, rhs_expr, calc_expr, logfmt, src0str,    \
                          src1str, src2str)                                  \
    static void inst_##inst_name(struct cpu *c, uint32_t inst)               \
    {                                                                        \
        uint16_t src0 = (src0_expr), src1 = (src1_expr), src2 = (src2_expr); \
        uint16_t lhs = (lhs_expr), rhs = (rhs_expr);                         \
        uint16_t res = (calc_expr);                                          \
                                                                             \
        reg_write(c, src0, res);                                             \
        pc_update(c, 4);                                                     \
                                                                             \
        log_printf(#inst_name " " logfmt "\n", (src0str), (src1str),         \
                   (src2str));                                               \
        log_printf("\t%04x = %04x " #op " %04x\n", res, lhs, rhs);           \
        log_printf("\t%s <= %04x\n", (src0str), res);                        \
        log_printf("\tPC <= %04x\n", pc_read(c));                            \
    }
#define DEFINE_INST_RRR(inst_name, op, calc_expr)            \
    DEFINE_INST_ARITH(inst_name, op,          /**/           \
                      get_bits(inst, 16, 20), /* src0/rd */  \
                      get_bits(inst, 11, 15), /* src1/rs1 */ \
                      get_bits(inst, 21, 25), /* src2/rs2 */ \
                      reg_read(c, src1),      /* lhs */      \
                      reg_read(c, src2),      /* rhs */      \
                      calc_expr,              /**/           \
                      "%s, %s, %s",           /* logfmt */   \
                      reg2str(src0),          /* rd */       \
                      reg2str(src1),          /* rs1 */      \
                      reg2str(src2) /* rs2 */)
#define DEFINE_INST_RRSimm11(inst_name, op, calc_expr)       \
    DEFINE_INST_ARITH(inst_name, op,          /**/           \
                      get_bits(inst, 16, 20), /* src0/rd */  \
                      get_bits(inst, 11, 15), /* src1/rs1 */ \
                      pick_simm11(inst),      /* src2/imm */ \
                      reg_read(c, src1),      /* lhs */      \
                      src2,                   /* rhs */      \
                      calc_expr,              /**/           \
                      "%s, %s, %d",           /* logfmt */   \
                      reg2str(src0),          /* rd */       \
                      reg2str(src1),          /* rs1 */      \
                      (int16_t)src2 /* imm */)
#define DEFINE_INST_RRUimm4(inst_name, op, calc_expr)        \
    DEFINE_INST_ARITH(inst_name, op,          /**/           \
                      get_bits(inst, 16, 20), /* src0/rd */  \
                      get_bits(inst, 11, 15), /* src1/rs1 */ \
                      pick_uimm4(inst),       /* src2/imm */ \
                      reg_read(c, src1),      /* lhs */      \
                      src2,                   /* rhs */      \
                      calc_expr,              /**/           \
                      "%s, %s, %d",           /* logfmt */   \
                      reg2str(src0),          /* rd */       \
                      reg2str(src1),          /* rs1 */      \
                      (int16_t)src2 /* imm */)
#define DEFINE_INST_STORE(inst_name, mem_write_expr)                           \
    static void inst_##inst_name(struct cpu *c, uint32_t inst)                 \
    {                                                                          \
        uint16_t rs = get_bits(inst, 21, 25), rd = get_bits(inst, 11, 15);     \
        uint16_t imm = pick_simm11_store(inst);                                \
                                                                               \
        uint16_t base = reg_read(c, rd), disp = imm, val = reg_read(c, rs);    \
        uint16_t addr = base + disp;                                           \
        (mem_write_expr);                                                      \
        pc_update(c, 4);                                                       \
                                                                               \
        log_printf(#inst_name " %s, %d(%s)\n", reg2str(rd), (int16_t)imm,      \
                   reg2str(rs));                                               \
        log_printf("\t[%04x = %04x + %04x] <= %04x\n", addr, base, disp, val); \
        log_printf("\tPC <= %04x\n", pc_read(c));                              \
    }
#define DEFINE_INST_LOAD(inst_name, mem_read_expr)                            \
    static void inst_##inst_name(struct cpu *c, uint32_t inst)                \
    {                                                                         \
        uint16_t rd = get_bits(inst, 16, 20), rs = get_bits(inst, 11, 15);    \
        uint16_t imm = pick_simm11(inst);                                     \
                                                                              \
        uint16_t base = reg_read(c, rs), disp = imm;                          \
        uint16_t addr = base + disp;                                          \
        uint16_t val = (mem_read_expr);                                       \
                                                                              \
        reg_write(c, rd, val);                                                \
        pc_update(c, 4);                                                      \
                                                                              \
        log_printf(#inst_name " %s, %d(%s)\n", reg2str(rd), (int16_t)imm,     \
                   reg2str(rs));                                              \
        log_printf("\t%04x = [%04x = %04x + %04x]\n", val, addr, base, disp); \
        log_printf("\t%s <= %04x\n", reg2str(rd), val);                       \
        log_printf("\tPC <= %04x\n", pc_read(c));                             \
    }
#define DEFINE_INST_BCC(inst_name, op, calc_expr)                             \
    static void inst_##inst_name(struct cpu *c, uint32_t inst)                \
    {                                                                         \
        uint16_t rs2 = get_bits(inst, 21, 25), rs1 = get_bits(inst, 11, 15);  \
        uint16_t imm = pick_simm13(inst);                                     \
                                                                              \
        uint16_t lhs = reg_read(c, rs1), rhs = reg_read(c, rs2);              \
        uint16_t res = (calc_expr);                                           \
                                                                              \
        if (res)                                                              \
            pc_write(c, pc_read(c) + imm);                                    \
        else                                                                  \
            pc_update(c, 4);                                                  \
                                                                              \
        log_printf(#inst_name " %s, %s, %d\n", reg2str(rs1), reg2str(rs2),    \
                   (int16_t)imm);                                             \
        log_printf("\t%s = %04x " #op " %04x\n", res ? "true" : "false", lhs, \
                   rhs);                                                      \
        log_printf("\tPC <= %04x\n", pc_read(c));                             \
    }

#include "inst.inc"

static void inst_li(struct cpu *c, uint32_t inst)
{
    uint16_t rd = get_bits(inst, 16, 20);
    uint16_t imm = get_bits(inst, 21, 30) | (get_bits(inst, 11, 15) << 10) |
                   (get_bits(inst, 31, 31) << 15);

    reg_write(c, rd, imm);
    pc_update(c, 4);

    log_printf("li %s, %d\n", reg2str(rd), (int16_t)imm);
    log_printf("\t%s <= %04x\n", reg2str(rd), imm);
    log_printf("\tPC <= %04x\n", pc_read(c));
}

static void inst_jal(struct cpu *c, uint32_t inst)
{
    uint16_t rd = get_bits(inst, 16, 20);
    uint16_t imm = get_bits(inst, 21, 30) | (get_bits(inst, 11, 15) << 10) |
                   (get_bits(inst, 31, 31) << 15);

    uint16_t rd_val = pc_read(c) + 4;

    reg_write(c, rd, rd_val);
    pc_write(c, pc_read(c) + imm);

    log_printf("jal %s, %d\n", reg2str(rd), imm);
    log_printf("\t%s <= %04x\n", reg2str(rd), rd_val);
    log_printf("\tPC <= %04x\n", pc_read(c));
}

static void inst_jalr(struct cpu *c, uint32_t inst)
{
    uint16_t rd = get_bits(inst, 16, 20), rs1 = get_bits(inst, 11, 15);
    int16_t imm = pick_simm11(inst);

    uint16_t val = reg_read(c, rs1) + imm;
    uint16_t rd_val = pc_read(c) + 4;

    reg_write(c, rd, rd_val);
    pc_write(c, val);

    log_printf("jalr %s, %s, %d\n", reg2str(rd), reg2str(rs1), imm);
    log_printf("\t%s <= %04x\n", reg2str(rd), rd_val);
    log_printf("\tPC <= %04x\n", pc_read(c));
}

const struct inst_info inst_list[] = {
    //                            8    4    0
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x11x_1000_1001", inst_lb},   // LB
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x01x_1000_1001", inst_lbu},  // LBU
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x01x_0000_1001", inst_lw},   // LW
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x01x_1000_0010", inst_sb},   // SB
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x01x_0000_0010", inst_sw},   // SW

    {"xxxx_xxxx_xxxx_xxxx_xxxx_x000_0000_1000", inst_add},   // ADD
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x000_0001_1000", inst_sub},   // SUB
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x000_0010_1000", inst_xor},   // XOR
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x000_0011_1000", inst_or},    // OR
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x000_0100_1000", inst_and},   // AND
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x000_1001_1000", inst_sll},   // SLL
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x000_1010_1000", inst_srl},   // SRL
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x000_1100_1000", inst_sra},   // SRA
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x101_0xxx_1000", inst_slt},   // SLT
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x001_0xxx_1000", inst_sltu},  // SLTU

    {"xxxx_xxxx_xxxx_xxxx_xxxx_x001_1000_1000", inst_mul},     // MUL
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x101_1001_1000", inst_mulh},    // MULH
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x101_1011_1000", inst_mulhsu},  // MULHSU
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x001_1010_1000", inst_mulhu},   // MULHU
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x101_1101_1000", inst_div},     // DIV
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x001_1100_1000", inst_divu},    // DIVU
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x101_1111_1000", inst_rem},     // REM
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x001_1110_1000", inst_remu},    // REMU

    {"xxxx_xxxx_xxxx_xxxx_xxxx_x010_0000_1000", inst_addi},   // ADDI
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x010_0010_1000", inst_xori},   // XORI
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x010_0011_1000", inst_ori},    // ORI
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x010_0100_1000", inst_andi},   // ANDI
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x010_1001_1000", inst_slli},   // SLLI
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x010_1010_1000", inst_srli},   // SRLI
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x010_1100_1000", inst_srai},   // SRAI
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x111_0xxx_1000", inst_slti},   // SLTI
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x011_0xxx_1000", inst_sltiu},  // SLTIU
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x110_0xxx_1000", inst_li},     // LI

    {"xxxx_xxxx_xxxx_xxxx_xxxx_x010_0001_0100", inst_beq},   // BEQ
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x010_0000_0100", inst_bne},   // BNE
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x110_0100_0100", inst_blt},   // BLT
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x110_0011_0100", inst_bge},   // BGE
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x010_0100_0100", inst_bltu},  // BLTU
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x010_0011_0100", inst_bgeu},  // BGEU
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x110_1111_1100", inst_jal},   // JAL
    {"xxxx_xxxx_xxxx_xxxx_xxxx_x011_1111_1100", inst_jalr},  // JALR

    {NULL, NULL}  // Sentinel
};
