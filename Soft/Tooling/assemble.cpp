#include "./asm.h"

/* FOR NOW ONLY PRINTS INSTRUCTIONS
 * ENCODING WILL BE IMPLEMENTED ONCE IT IS FINALIZED
 */

#include <cstdio> //printf

#include <vector>

extern int code_position;
extern bool label_only;

struct t_Label{
	std::string_view name;
	uint16_t pos;

	t_Label(){}
	t_Label(std::string_view const n_name, uint16_t const n_pos)
		: name(n_name), pos(n_pos) {}
};

std::vector<t_Label> labels;

int label_to_pos(std::string_view const lab)
{
	for(auto const& l : labels)
		if(l.name == lab)
			return l.pos;


	printf("ERROR: label %s undefined\n", &lab[0]);
	return -1;
}

//assembler stuff
void label(std::string_view const lab)
{
	if(label_only)
		labels.emplace_back(lab, code_position + 1);

	return;
}

//no operand
void i_ret()
{
	if(not label_only)
		printf("x%04x: ret\n", code_position);

	code_position++;
	return;
}

//reg
#define INSTR_R(name)                                                          \
void i_ ##name (reg::Type const src)                                           \
{                                                                              \
	if(not label_only)                                                         \
		printf("x%04x: "#name" R%d\n", code_position, (src));                  \
                                                                               \
	code_position++;                                                           \
	return;                                                                    \
}

INSTR_R(jmp)
INSTR_R(bre)
INSTR_R(bne)
INSTR_R(bgt)
INSTR_R(bls)
INSTR_R(call)
INSTR_R(push)
INSTR_R(pull)

//reg-reg
#define INSTR_RR(name)                                                                                    \
void i_ ## name(reg::Type const dst, reg::Type const src)                                                 \
{                                                                                                         \
	if(not label_only)                                                                                    \
		printf("x%04x: "#name" R%d R%d\n", code_position, (dst), (src));                                  \
                                                                                                          \
	code_position++;                                                                                      \
	return;                                                                                               \
}

INSTR_RR(mov)
INSTR_RR(ldw)
INSTR_RR(sdw)
INSTR_RR(cmp)
INSTR_RR(add)
INSTR_RR(sub)
INSTR_RR(and)
INSTR_RR(or)
INSTR_RR(xor)
INSTR_RR(not)
INSTR_RR(lsr)
INSTR_RR(lsl)

//imm
#define INSTR_I(name)                                                      \
void i_ ##name (uint16_t const imm)                                        \
{                                                                          \
	if(not label_only)                                                     \
		printf("x%04x: "#name" x%04x\n", code_position, imm);              \
                                                                           \
	code_position++;                                                       \
	return;                                                                \
}

#define INSTR_L(name)                                                      \
void i_ ##name (std::string_view const lab)                                \
{                                                                          \
	if(not label_only)                                                     \
		printf("x%04x: "#name" x%04x\n", code_position, label_to_pos(lab));\
                                                                           \
	code_position++;                                                       \
	return;                                                                \
}

INSTR_I(jmp)
INSTR_I(bre)
INSTR_I(bne)
INSTR_I(bgt)
INSTR_I(bls)
INSTR_I(call)

INSTR_L(jmp)
INSTR_L(bre)
INSTR_L(bne)
INSTR_L(bgt)
INSTR_L(bls)
INSTR_L(call)

//reg-imm
#define INSTR_RI(name)                                                                                 \
void i_ ##name (reg::Type const dst, uint16_t const imm)                                               \
{                                                                                                      \
	if(not label_only)                                                                                 \
		printf("x%04x: "#name" R%d x%04x\n", code_position, (dst), imm);                               \
                                                                                                       \
	code_position++;                                                                                   \
	return;                                                                                            \
}
#define INSTR_RL(name)                                                                                 \
void i_ ##name (reg::Type const dst, std::string_view const lab)                                       \
{                                                                                                      \
	if(not label_only)                                                                                 \
		printf("x%04x: "#name" R%d x%04x\n", code_position, (dst), label_to_pos(lab));                 \
                                                                                                       \
	code_position++;                                                                                   \
	return;                                                                                            \
}

INSTR_RI(mov)
INSTR_RI(ldw)
INSTR_RI(sdw)
INSTR_RI(cmp)

INSTR_RI(add)
INSTR_RI(sub)
INSTR_RI(and)
INSTR_RI(or)
INSTR_RI(xor)
INSTR_RI(not)
INSTR_RI(lsr)
INSTR_RI(lsl)

INSTR_RL(mov)
INSTR_RL(ldw)
INSTR_RL(sdw)
INSTR_RL(cmp)

INSTR_RL(add)
INSTR_RL(sub)
INSTR_RL(and)
INSTR_RL(or)
INSTR_RL(xor)
INSTR_RL(not)
INSTR_RL(lsr)
INSTR_RL(lsl)


//pseudoinstructions
void i_nand(reg::Type const dst, reg::Type const src)
{
	i_and(dst, src);
	i_not(dst, dst);
	return;
}
void i_nand(reg::Type const dst, uint16_t imm)
{
	i_and(dst, imm);
	i_not(dst, dst);
	return;
}
void i_nand(reg::Type const dst, std::string_view const lab)
{
	i_and(dst, lab);
	i_not(dst, dst);
	return;
}

