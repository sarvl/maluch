#include <cstdio> //printf
#include <fstream>

#include <iomanip> //hex

#include <cstdint>

#include <algorithm> //sort

#include <vector>

#include "./assemble.h"

uint16_t code_position = 0;
bool label_only = false;

extern std::vector<t_Label> labels;

int main()
{

#if defined(ASSEMBLE)

	//2 passes to 1. find labels, 2. put all relevant data
	
	code_position = 0;
	label_only = true;
	code();

	code_position = 0;
	label_only = false;
	code();

	std::sort(
		instructions.begin(), instructions.end(), 
		[](auto const& L, auto const& R){return L.code_pos < R.code_pos;}
		);

	code_position = 0;
	for(auto const instr : instructions)
	{
		while(code_position != instr.code_pos)
		{
			printf("0000\n");
			code_position++;
		}

		printf("%04X\n", instr.data);
		code_position++;
	}

	std::ofstream write("symbols.txt");

	for(auto const& label : labels)
		write << label.name << ' ' << std::hex << label.pos << '\n';
#else

	#error "No option specified, use through assembly.sh"

#endif 
}
