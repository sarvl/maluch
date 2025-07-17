#pragma once

#include <string_view>

#include <cstdint>

namespace reg {
	enum Type {
		R0  = 0,
		R1  = 1,
		R2  = 2,
		R3  = 3,
		R4  = 4,
		R5  = 5,
		R6  = 6,
		R7  = 7,
		R8  = 8,
		R9  = 9,
		R10 = 10,
		R11 = 11,
		R12 = 12,
		R13 = 13,
		R14 = 14,
		R15 = 15
	};
};

//assembler stuff
void label(std::string_view const lab);

//no operand
void i_ret();

//reg
void i_jmp(reg::Type const src);
void i_bre(reg::Type const src);
void i_bne(reg::Type const src);
void i_bgt(reg::Type const src);
void i_bls(reg::Type const src);
void i_call(reg::Type const src);
void i_push(reg::Type const src);
void i_pull(reg::Type const src);

//reg-reg
void i_mov(reg::Type const dst, reg::Type const src);
void i_ldw(reg::Type const dst, reg::Type const src);
void i_sdw(reg::Type const dst, reg::Type const src);
void i_cmp(reg::Type const dst, reg::Type const src);

void i_add(reg::Type const dst, reg::Type const src);
void i_sub(reg::Type const dst, reg::Type const src);
void i_and(reg::Type const dst, reg::Type const src);
void i_or (reg::Type const dst, reg::Type const src);
void i_xor(reg::Type const dst, reg::Type const src);
void i_not(reg::Type const dst, reg::Type const src);
void i_lsr(reg::Type const dst, reg::Type const src);
void i_lsl(reg::Type const dst, reg::Type const src);

//imm
void i_jmp (uint16_t imm);
void i_bre (uint16_t imm);
void i_bne (uint16_t imm);
void i_bgt (uint16_t imm);
void i_bls (uint16_t imm);
void i_call(uint16_t imm);

void i_jmp (std::string_view const lab);
void i_bre (std::string_view const lab);
void i_bne (std::string_view const lab);
void i_bgt (std::string_view const lab);
void i_bls (std::string_view const lab);
void i_call(std::string_view const lab);

//reg-imm
void i_mov(reg::Type const dst, uint16_t imm);
void i_ldw(reg::Type const dst, uint16_t imm);
void i_sdw(reg::Type const dst, uint16_t imm);
void i_cmp(reg::Type const dst, uint16_t imm);

void i_add(reg::Type const dst, uint16_t imm);
void i_sub(reg::Type const dst, uint16_t imm);
void i_and(reg::Type const dst, uint16_t imm);
void i_or (reg::Type const dst, uint16_t imm);
void i_xor(reg::Type const dst, uint16_t imm);
void i_not(reg::Type const dst, uint16_t imm);
void i_lsr(reg::Type const dst, uint16_t imm);
void i_lsl(reg::Type const dst, uint16_t imm);

void i_mov(reg::Type const dst, std::string_view const lab);
void i_ldw(reg::Type const dst, std::string_view const lab);
void i_sdw(reg::Type const dst, std::string_view const lab);
void i_cmp(reg::Type const dst, std::string_view const lab);

void i_add(reg::Type const dst, std::string_view const lab);
void i_sub(reg::Type const dst, std::string_view const lab);
void i_and(reg::Type const dst, std::string_view const lab);
void i_or (reg::Type const dst, std::string_view const lab);
void i_xor(reg::Type const dst, std::string_view const lab);
void i_not(reg::Type const dst, std::string_view const lab);
void i_lsr(reg::Type const dst, std::string_view const lab);
void i_lsl(reg::Type const dst, std::string_view const lab);

//pseudoinstructions
void i_nand(reg::Type const dst, reg::Type const src);
void i_nand(reg::Type const dst, uint16_t imm);
void i_nand(reg::Type const dst, std::string_view const lab);

