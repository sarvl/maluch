#include "./asm.h"
#include "./simulate.h"

#include <cstdio> //printf

#include <vector>

extern int code_position;
extern bool label_only;

//internally, each register is 32bit because 1) that is faster on cpu 2) simplifies flag determination
uint32_t regs[16];
uint32_t flags;
uint8_t  memory[65535]; //64kiB

struct t_Label{ 
	std::string_view name;
	uint16_t pos;

	t_Label(){}
	t_Label(std::string_view const n_name, uint16_t const n_pos)
		: name(n_name), pos(n_pos) {}
};

std::vector<t_Label> labels;

void print_regs()
{
	for(int i = 0; i < 16; i++)
		printf("R%02d : x%04x\n", i, regs[i]);

	return;
}

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
void i_mov(reg::Type const dst, reg::Type const src)
{
	regs[dst] = regs[src];
	return;
}

void i_ldw(reg::Type const dst, reg::Type const src)
{
	regs[dst] = (memory[regs[src]    ] << 8) 
	               | (memory[regs[src] + 1] << 0);
	return;
}
void i_sdw(reg::Type const dst, reg::Type const src)
{
	memory[regs[src]    ] = 0xFF & (regs[dst] >> 8);
	memory[regs[src] + 1] = 0xFF & (regs[dst] >> 0);

	return;
}

#define ARITHMETIC_RR(name, op)                                                          \
void name(reg::Type const dst, reg::Type const src)                                      \
{                                                                                        \
	uint32_t const res = regs[dst] op regs[src];                                         \
                                                                                         \
	int f_overflow  = (((regs[dst] ^ regs[src]) & ~(regs[src] ^ res)) >> 15) & 0b1;      \
	int f_sign      = (res >> 15) & 0b1;                                                 \
	int f_carry     = (res >> 16) & 0b1;                                                 \
	int f_zero      = (res & 0xFF) == 0;                                                 \
                                                                                         \
	flags = (f_overflow << 3) | (f_sign << 2) | (f_carry << 1) | (f_zero << 0);          \
	                                                                                     \
	regs[dst] = res;                                                                     \
                                                                                         \
	return;                                                                              \
}                                                                                        \

void i_cmp(reg::Type const dst, reg::Type const src)
{
	uint32_t const res = regs[dst] - regs[src];

	int f_overflow  = (((regs[dst] ^ regs[src]) & ~(regs[src] ^ res)) >> 15) & 0b1;
	int f_sign      = (res >> 15) & 0b1;
	int f_carry     = (res >> 16) & 0b1;
	int f_zero      = (res & 0xFF) == 0;

	flags = (f_overflow << 3) | (f_sign << 2) | (f_carry << 1) | (f_zero << 0);

	return;
}

ARITHMETIC_RR(i_add, +);
ARITHMETIC_RR(i_sub, -);
ARITHMETIC_RR(i_and, &);
ARITHMETIC_RR(i_or,  |);
ARITHMETIC_RR(i_xor, ^);
ARITHMETIC_RR(i_lsr, <<);
ARITHMETIC_RR(i_lsl, >>);

void i_not(reg::Type const dst, reg::Type const src)                                      
{                                                                                        
	uint32_t const res = ~regs[src];                                         
                                                                                         
	int f_overflow  = 0; //whatever
	int f_sign      = (res >> 15) & 0b1;                                                 
	int f_carry     = (res >> 16) & 0b1;                                                 
	int f_zero      = (res & 0xFF) == 0;                                                 
                                                                                         
	flags = (f_overflow << 3) | (f_sign << 2) | (f_carry << 1) | (f_zero << 0);          
	                                                                                     
	regs[dst] = res;                                                                     
                                                                                         
	return;                                                                              
}                                                                                        

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
{
	regs[dst] = imm;
	return;
}
void i_ldw(reg::Type const dst, uint16_t imm);
void i_sdw(reg::Type const dst, uint16_t imm);
void i_cmp(reg::Type const dst, uint16_t imm);
{
	uint32_t const res = regs[dst] - imm;

	int f_overflow  = (((regs[dst] ^ imm) & ~(imm ^ res)) >> 15) & 0b1;
	int f_sign      = (res >> 15) & 0b1;
	int f_carry     = (res >> 16) & 0b1;
	int f_zero      = (res & 0xFF) == 0;

	flags = (f_overflow << 3) | (f_sign << 2) | (f_carry << 1) | (f_zero << 0);

	return
}

#define ARITHMETIC_RI(name, op)                                                          \
void name(reg::Type const dst, uint16_t imm)                                             \
{                                                                                        \
	uint32_t const res = regs[dst] op imm;                                               \
                                                                                         \
	int f_overflow  = (((regs[dst] ^ imm) & ~(imm ^ res)) >> 15) & 0b1;                  \
	int f_sign      = (res >> 15) & 0b1;                                                 \
	int f_carry     = (res >> 16) & 0b1;                                                 \
	int f_zero      = (res & 0xFF) == 0;                                                 \
                                                                                         \
	flags = (f_overflow << 3) | (f_sign << 2) | (f_carry << 1) | (f_zero << 0);          \
	                                                                                     \
	regs[dst] = res;                                                                     \
                                                                                         \
	return;                                                                              \
}                                                                                        \


ARITHMETIC_RI(i_add, +);
ARITHMETIC_RI(i_sub, -);
ARITHMETIC_RI(i_and, &);
ARITHMETIC_RI(i_or,  |);
ARITHMETIC_RI(i_xor, ^);
ARITHMETIC_RI(i_lsr, <<);
ARITHMETIC_RI(i_lsl, >>);

void i_not(reg::Type const dst, uint16_t imm)                                      
{                                                                                        
	uint32_t const res = ~imm;                                         
                                                                                         
	int f_overflow  = 0; //whatever
	int f_sign      = (res >> 15) & 0b1;                                                 
	int f_carry     = (res >> 16) & 0b1;                                                 
	int f_zero      = (res & 0xFF) == 0;                                                 
                                                                                         
	flags = (f_overflow << 3) | (f_sign << 2) | (f_carry << 1) | (f_zero << 0);          
	                                                                                     
	regs[dst] = res;                                                                     
                                                                                         
	return;                                                                              
}                                                                                        

//forwarded
void i_mov(reg::Type const dst, std::string_view const lab) {i_mov(dst, label_to_pos(lab)); return; };
void i_ldw(reg::Type const dst, std::string_view const lab) {i_ldw(dst, label_to_pos(lab)); return; };
void i_sdw(reg::Type const dst, std::string_view const lab) {i_sdw(dst, label_to_pos(lab)); return; };
void i_cmp(reg::Type const dst, std::string_view const lab) {i_cmp(dst, label_to_pos(lab)); return; };
                                                                  
void i_add(reg::Type const dst, std::string_view const lab) {i_add(dst, label_to_pos(lab)); return; };
void i_sub(reg::Type const dst, std::string_view const lab) {i_sub(dst, label_to_pos(lab)); return; };
void i_and(reg::Type const dst, std::string_view const lab) {i_and(dst, label_to_pos(lab)); return; };
void i_or (reg::Type const dst, std::string_view const lab) {i_or (dst, label_to_pos(lab)); return; };
void i_xor(reg::Type const dst, std::string_view const lab) {i_xor(dst, label_to_pos(lab)); return; };
void i_not(reg::Type const dst, std::string_view const lab) {i_not(dst, label_to_pos(lab)); return; };
void i_lsr(reg::Type const dst, std::string_view const lab) {i_lsr(dst, label_to_pos(lab)); return; };
void i_lsl(reg::Type const dst, std::string_view const lab) {i_lsl(dst, label_to_pos(lab)); return; };

//pseudoinstructions
void i_nand(reg::Type const dst, reg::Type const src) { i_and(dst, src); i_not(dst, src); return; };
void i_nand(reg::Type const dst, uint16_t imm) { i_and(dst, imm); i_not(dst, imm); return; };
void i_nand(reg::Type const dst, std::string_view const lab) {i_nand(dst, label_to_pos(lab)); return; };

