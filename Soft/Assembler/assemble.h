#pragma once

#include <vector>
#include <string_view>

struct t_Instr{
	uint16_t code_pos;
	uint16_t data;

	t_Instr(){}
	t_Instr(uint16_t const n_cp, uint16_t const n_d);
};
struct t_Label{
	std::string_view name;
	uint16_t pos;

	t_Label();
	t_Label(std::string_view const n_name, uint16_t const n_pos);
};

extern std::vector<t_Instr> instructions;

int code();

