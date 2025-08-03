#include "./asm.h"
#include "./assemble.h"

#include <cstdio> //printf

#include <vector>

extern uint16_t code_position;
extern bool label_only;

t_Label::t_Label(){}
t_Label::t_Label(std::string_view const n_name, uint16_t const n_pos)
		: name(n_name), pos(n_pos) {}

std::vector<t_Label> labels;
std::vector<t_Instr> instructions;

t_Instr::t_Instr(uint16_t const n_cp, uint16_t const n_d)
	: code_pos(n_cp), data(n_d)
{}


[[nodiscard]] int label_to_pos(std::string_view const lab)
{
	//necessary for simplicity of trampolines
	//they WILL call this function but the result does nott matter
	if(label_only)
		return 0;

	for(auto const& l : labels)
		if(l.name == lab)
			return l.pos;


	printf("ERROR: label %s undefined\ninstruction in memory: 0x%04X\n", &lab[0], code_position);
	return -1;
}

//assembler stuff
uint16_t label(std::string_view const lab)
{
	if(label_only)
		labels.emplace_back(lab, code_position);

	return code_position;
}

//no operand
#define INSTR_NOARG(name, opcode, funct)\
void name ()\
{\
	code_position++;\
	if(label_only) return;\
\
	uint16_t const instr = (opcode << 12)\
	                     | (funct  <<  8);\
	\
	instructions.emplace_back(static_cast<uint16_t>(code_position - 1), instr);\
	return;\
}
#define INSTR_RR(name, opcode, funct)\
void name (t_reg const Rd, t_reg const src)\
{\
	code_position++;\
	if(label_only) return;\
\
	uint16_t const instr = (opcode << 12)\
	                     | (funct  <<  8)\
	                     | (Rd  << 4)\
	                     | (src << 0);\
	\
	instructions.emplace_back(static_cast<uint16_t>(code_position - 1), instr);\
	return;\
}
#define INSTR_RS(name, opcode, funct)\
void name (t_reg const src)\
{\
	code_position++;\
	if(label_only) return;\
\
	uint16_t const instr = (opcode << 12)\
	                     | (funct  <<  8)\
	                     | (src << 0);\
	\
	instructions.emplace_back(static_cast<uint16_t>(code_position - 1), instr);\
	return;\
}
#define INSTR_RD(name, opcode, funct)\
void name (t_reg const Rd)\
{\
	code_position++;\
	if(label_only) return;\
\
	uint16_t const instr = (opcode << 12)\
	                     | (funct  <<  8)\
	                     | (Rd << 4);\
	\
	instructions.emplace_back(static_cast<uint16_t>(code_position - 1), instr);\
	return;\
}

#define INSTR_FRS(name, opcode)\
void name (uint8_t const funct, t_reg const src)\
{\
	code_position++;\
	if(label_only) return;\
\
	uint16_t const instr = (opcode << 12)\
	                     | (funct  <<  8)\
	                     | (src << 0);\
	\
	instructions.emplace_back(static_cast<uint16_t>(code_position - 1), instr);\
	return;\
}
#define INSTR_FRD(name, opcode)\
void name (uint8_t const funct, t_reg const Rd)\
{\
	code_position++;\
	if(label_only) return;\
\
	uint16_t const instr = (opcode << 12)\
	                     | (funct  <<  8)\
	                     | (Rd << 4);\
	\
	instructions.emplace_back(static_cast<uint16_t>(code_position - 1), instr);\
	return;\
}

#define INSTR_RI(name, opcode, funct)\
void name (t_reg const Rd, uint16_t const src)\
{\
	code_position += 2;\
	if(label_only) return;\
\
	uint16_t const instr = (opcode << 12)\
	                     | (0b1 << 11)\
	                     | (funct << 8)\
	                     | (Rd  << 4);\
	\
	instructions.emplace_back(static_cast<uint16_t>(code_position - 2), instr);\
	instructions.emplace_back(static_cast<uint16_t>(code_position - 1), src);\
	return;\
}
#define INSTR_I(name, opcode, funct)\
void name (uint16_t const src)\
{\
	code_position += 2;\
	if(label_only) return;\
\
	uint16_t const instr = (opcode << 12)\
	                     | (0b1 << 11)\
	                     | (funct  <<  8);\
	\
	instructions.emplace_back(static_cast<uint16_t>(code_position - 2), instr);\
	instructions.emplace_back(static_cast<uint16_t>(code_position - 1), src);\
	return;\
}

#define INSTR_FI(name, opcode)\
void name (uint8_t const funct, uint16_t const src)\
{\
	code_position += 2;\
	if(label_only) return;\
\
	uint16_t const instr = (opcode << 12)\
	                     | (0b1 << 11)\
	                     | (funct  <<  8);\
	\
	instructions.emplace_back(static_cast<uint16_t>(code_position - 2), instr);\
	instructions.emplace_back(static_cast<uint16_t>(code_position - 1), src);\
	return;\
}

INSTR_NOARG(i_ret , 0b1011, 0b000)
INSTR_NOARG(i_iret, 0b1011, 0b001)

//reg-reg variants
INSTR_RR(i_mov , 0b0001, 0b000)
INSTR_RR(i_ldw , 0b1000, 0b000)
INSTR_RR(i_stw , 0b1001, 0b000)

INSTR_RR(i_add , 0b0010, 0b000)
INSTR_RR(i_sub , 0b0010, 0b001)
INSTR_RR(i_and , 0b0010, 0b010)
INSTR_RR(i_or  , 0b0010, 0b011)
INSTR_RR(i_xor , 0b0010, 0b100)
INSTR_RR(i_not , 0b0010, 0b101)
INSTR_RR(i_lsl , 0b0010, 0b110)
INSTR_RR(i_lsr , 0b0010, 0b111)

INSTR_RR(i_cmp , 0b0011, 0b001)
INSTR_RR(i_test, 0b0011, 0b010)

//reg variants

INSTR_RS(i_jmp , 0b0100, 0b000)
INSTR_RS(i_bee , 0b0100, 0b001)
INSTR_RS(i_bne , 0b0100, 0b010)
INSTR_RS(i_bge , 0b0100, 0b011)
INSTR_RS(i_ble , 0b0100, 0b100)
INSTR_RS(i_bgg , 0b0100, 0b101)
INSTR_RS(i_bll , 0b0100, 0b110)
INSTR_RS(i_boo , 0b0100, 0b111)
INSTR_RS(i_bbs , 0b0101, 0b000)
INSTR_RS(i_bss , 0b0101, 0b001)
INSTR_RS(i_bns , 0b0101, 0b010)
INSTR_RS(i_bae , 0b0101, 0b011)
INSTR_RS(i_bbe , 0b0101, 0b100)
INSTR_RS(i_baa , 0b0101, 0b101)
INSTR_RS(i_bbb , 0b0101, 0b110)
INSTR_RS(i_bno , 0b0101, 0b111)

INSTR_RS(i_call, 0b1010, 0b000)

INSTR_RS(i_push, 0b1100, 0b000)

INSTR_RD(i_pull, 0b1101, 0b000)

//funct-reg variants
INSTR_FRD(i_in , 0b0110);
INSTR_FRS(i_out, 0b0111);

//reg-imm variants
INSTR_RI(i_mov , 0b0001, 0b000)
INSTR_RI(i_ldw , 0b1000, 0b000)
INSTR_RI(i_stw , 0b1001, 0b000)

INSTR_RI(i_add , 0b0010, 0b000)
INSTR_RI(i_sub , 0b0010, 0b001)
INSTR_RI(i_and , 0b0010, 0b010)
INSTR_RI(i_or  , 0b0010, 0b011)
INSTR_RI(i_xor , 0b0010, 0b100)
INSTR_RI(i_not , 0b0010, 0b101)
INSTR_RI(i_lsl , 0b0010, 0b110)
INSTR_RI(i_lsr , 0b0010, 0b111)

INSTR_RI(i_cmp , 0b0011, 0b001)
INSTR_RI(i_test, 0b0011, 0b010)

//imm variants

INSTR_I(i_jmp , 0b0100, 0b000)
INSTR_I(i_bee , 0b0100, 0b001)
INSTR_I(i_bne , 0b0100, 0b010)
INSTR_I(i_bge , 0b0100, 0b011)
INSTR_I(i_ble , 0b0100, 0b100)
INSTR_I(i_bgg , 0b0100, 0b101)
INSTR_I(i_bll , 0b0100, 0b110)
INSTR_I(i_boo , 0b0100, 0b111)
INSTR_I(i_bbs , 0b0101, 0b000)
INSTR_I(i_bss , 0b0101, 0b001)
INSTR_I(i_bns , 0b0101, 0b010)
INSTR_I(i_bae , 0b0101, 0b011)
INSTR_I(i_bbe , 0b0101, 0b100)
INSTR_I(i_baa , 0b0101, 0b101)
INSTR_I(i_bbb , 0b0101, 0b110)
INSTR_I(i_bno , 0b0101, 0b111)

INSTR_I(i_call, 0b1010, 0b000)

INSTR_I(i_push, 0b1100, 0b000)

//funct-imm variants
INSTR_FI(i_out, 0b0111);

//all lab variants are just trampolines 
//reg-lab variants
void i_mov (t_reg const Rd, std::string_view const src) { i_mov (Rd, label_to_pos(src)); return; }
void i_ldw (t_reg const Rd, std::string_view const src) { i_ldw (Rd, label_to_pos(src)); return; }
void i_stw (t_reg const Rd, std::string_view const src) { i_stw (Rd, label_to_pos(src)); return; }

void i_add (t_reg const Rd, std::string_view const src) { i_add (Rd, label_to_pos(src)); return; }
void i_sub (t_reg const Rd, std::string_view const src) { i_sub (Rd, label_to_pos(src)); return; }
void i_and (t_reg const Rd, std::string_view const src) { i_and (Rd, label_to_pos(src)); return; }
void i_or  (t_reg const Rd, std::string_view const src) { i_or  (Rd, label_to_pos(src)); return; }
void i_xor (t_reg const Rd, std::string_view const src) { i_xor (Rd, label_to_pos(src)); return; }
void i_not (t_reg const Rd, std::string_view const src) { i_not (Rd, label_to_pos(src)); return; }
void i_lsl (t_reg const Rd, std::string_view const src) { i_lsl (Rd, label_to_pos(src)); return; }
void i_lsr (t_reg const Rd, std::string_view const src) { i_lsr (Rd, label_to_pos(src)); return; }
void i_cmp (t_reg const Rd, std::string_view const src) { i_cmp (Rd, label_to_pos(src)); return; }
void i_test(t_reg const Rd, std::string_view const src) { i_test(Rd, label_to_pos(src)); return; }

void i_out(uint8_t const funct, std::string_view const src) { i_out(funct, label_to_pos(src)); return; }

//lab variants
void i_jmp (std::string_view const src) { i_jmp (label_to_pos(src)); return; }
void i_bee (std::string_view const src) { i_bee (label_to_pos(src)); return; }
void i_bne (std::string_view const src) { i_bne (label_to_pos(src)); return; }
void i_bge (std::string_view const src) { i_bge (label_to_pos(src)); return; }
void i_ble (std::string_view const src) { i_ble (label_to_pos(src)); return; }
void i_bgg (std::string_view const src) { i_bgg (label_to_pos(src)); return; }
void i_bll (std::string_view const src) { i_bll (label_to_pos(src)); return; }
void i_boo (std::string_view const src) { i_boo (label_to_pos(src)); return; }
void i_bbs (std::string_view const src) { i_bbs (label_to_pos(src)); return; }
void i_bss (std::string_view const src) { i_bss (label_to_pos(src)); return; }
void i_bns (std::string_view const src) { i_bns (label_to_pos(src)); return; }
void i_bae (std::string_view const src) { i_bae (label_to_pos(src)); return; }
void i_bbe (std::string_view const src) { i_bbe (label_to_pos(src)); return; }
void i_baa (std::string_view const src) { i_baa (label_to_pos(src)); return; }
void i_bbb (std::string_view const src) { i_bbb (label_to_pos(src)); return; }
void i_bno (std::string_view const src) { i_bno (label_to_pos(src)); return; }
void i_call(std::string_view const src) { i_call(label_to_pos(src)); return; }

void i_push(std::string_view const src) { i_push(label_to_pos(src)); return; }

//pseudoinstructions
void i_nand(t_reg const dst, t_reg const src)
{
	i_and(dst, src);
	i_not(dst, dst);
	return;
}
void i_nand(t_reg const dst, uint16_t const imm)
{
	i_and(dst, imm);
	i_not(dst, dst);
	return;
}
void i_nand(t_reg const dst, std::string_view const lab)
{
	i_nand(dst, label_to_pos(lab));
	return;
}


void word(uint16_t const data)
{
	code_position++;

	if(label_only)
		return;

	instructions.emplace_back(static_cast<uint16_t>(code_position - 1), data);
	return;
}

