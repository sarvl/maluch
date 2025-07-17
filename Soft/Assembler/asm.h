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

using t_reg = reg::Type;

//assembler stuff
void label(std::string_view const lab);

//no operand
void i_ret ();
void i_iret();

//reg-reg variants
void i_mov (t_reg const Rd, t_reg const src);
void i_ldw (t_reg const Rd, t_reg const src);
void i_stw (t_reg const Rd, t_reg const src);

void i_add (t_reg const Rd, t_reg const src);
void i_sub (t_reg const Rd, t_reg const src);
void i_and (t_reg const Rd, t_reg const src);
void i_or  (t_reg const Rd, t_reg const src);
void i_xor (t_reg const Rd, t_reg const src);
void i_not (t_reg const Rd, t_reg const src);
void i_lsl (t_reg const Rd, t_reg const src);
void i_lsr (t_reg const Rd, t_reg const src);
void i_cmp (t_reg const Rd, t_reg const src);
void i_test(t_reg const Rd, t_reg const src);

//reg variants

void i_jmp (t_reg const src);
void i_bee (t_reg const src);
void i_bne (t_reg const src);
void i_bge (t_reg const src);
void i_ble (t_reg const src);
void i_bgg (t_reg const src);
void i_bll (t_reg const src);
void i_boo (t_reg const src);
void i_bbs (t_reg const src);
void i_bss (t_reg const src);
void i_bns (t_reg const src);
void i_bae (t_reg const src);
void i_bbe (t_reg const src);
void i_baa (t_reg const src);
void i_bbb (t_reg const src);
void i_bno (t_reg const src);
void i_call(t_reg const src);

void i_push(t_reg const src);
void i_pull(t_reg const Rd);

//funct-reg variants
void i_in  (uint8_t const funct, t_reg const src);
void i_out (uint8_t const funct, t_reg const src);


//reg-imm variants
void i_mov (t_reg const Rd, uint16_t const src);
void i_ldw (t_reg const Rd, uint16_t const src);
void i_stw (t_reg const Rd, uint16_t const src);

void i_add (t_reg const Rd, uint16_t const src);
void i_sub (t_reg const Rd, uint16_t const src);
void i_and (t_reg const Rd, uint16_t const src);
void i_or  (t_reg const Rd, uint16_t const src);
void i_xor (t_reg const Rd, uint16_t const src);
void i_not (t_reg const Rd, uint16_t const src);
void i_lsl (t_reg const Rd, uint16_t const src);
void i_lsr (t_reg const Rd, uint16_t const src);
void i_cmp (t_reg const Rd, uint16_t const src);
void i_test(t_reg const Rd, uint16_t const src);

//imm variants

void i_jmp (uint16_t const src);
void i_bee (uint16_t const src);
void i_bne (uint16_t const src);
void i_bge (uint16_t const src);
void i_ble (uint16_t const src);
void i_bgg (uint16_t const src);
void i_bll (uint16_t const src);
void i_boo (uint16_t const src);
void i_bbs (uint16_t const src);
void i_bss (uint16_t const src);
void i_bns (uint16_t const src);
void i_bae (uint16_t const src);
void i_bbe (uint16_t const src);
void i_baa (uint16_t const src);
void i_bbb (uint16_t const src);
void i_bno (uint16_t const src);
void i_call(uint16_t const src);

void i_push(uint16_t const src);

//funct-imm variants
void i_in  (uint8_t const funct, uint16_t const src);
void i_out (uint8_t const funct, uint16_t const src);


//reg-lab variants
void i_mov (t_reg const Rd, std::string_view const src);
void i_ldw (t_reg const Rd, std::string_view const src);
void i_stw (t_reg const Rd, std::string_view const src);

void i_add (t_reg const Rd, std::string_view const src);
void i_sub (t_reg const Rd, std::string_view const src);
void i_and (t_reg const Rd, std::string_view const src);
void i_or  (t_reg const Rd, std::string_view const src);
void i_xor (t_reg const Rd, std::string_view const src);
void i_lsl (t_reg const Rd, std::string_view const src);
void i_lsr (t_reg const Rd, std::string_view const src);
void i_cmp (t_reg const Rd, std::string_view const src);
void i_test(t_reg const Rd, std::string_view const src);

//lab variants
void i_not (std::string_view const src);

void i_jmp (std::string_view const src);
void i_bee (std::string_view const src);
void i_bne (std::string_view const src);
void i_bge (std::string_view const src);
void i_ble (std::string_view const src);
void i_bgg (std::string_view const src);
void i_bll (std::string_view const src);
void i_boo (std::string_view const src);
void i_bbs (std::string_view const src);
void i_bss (std::string_view const src);
void i_bns (std::string_view const src);
void i_bae (std::string_view const src);
void i_bbe (std::string_view const src);
void i_baa (std::string_view const src);
void i_bbb (std::string_view const src);
void i_bno (std::string_view const src);
void i_call(std::string_view const src);

void i_push(std::string_view const src);

//funct-imm variants
void i_in  (uint8_t const funct, std::string_view const src);
void i_out (uint8_t const funct, std::string_view const src);

//pseudoinstructions
void i_nand(t_reg const dst, t_reg const src);
void i_nand(t_reg const dst, uint16_t const imm);
void i_nand(t_reg const dst, std::string_view const lab);
