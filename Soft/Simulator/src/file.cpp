#include <string_view>

#include <cstdint>

#include "../../utility/src/file.h"
#include "../../utility/src/log.h"

constexpr int hexd_to_val(char const hexd)
{
	constexpr signed char translate[128] = {
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1, 
    	 -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	return translate[hexd & 0x7F];
}

int read_file(uint16_t * const dest, std::string_view const name)
{
	File::t_File file;
	if(-1 == File::create_error_handled(&file, name))
		return -1;
	char const* const data = file.data;
	int  const        size = file.size;

	int line_num = 1;
	int off = 0;
	while(off < size)
	{
		if(off + 5 > size
		|| data[off + 0] == '\n'
		|| data[off + 1] == '\n'
		|| data[off + 2] == '\n'
		|| data[off + 3] == '\n')
		{
			Log::error(
				"line does not contain full instruction, aborting",
				name,
				line_num);
			
			//not handled because main error is above
			//if something fails, will be handled by OS on cleanup
			(void)File::destroy(file);
			return -2;
		}
		if(data[off + 4] != '\n')
		{
			Log::error(
				"line contains too many characters, aborting",
				name,
				line_num);

			(void)File::destroy(file);
			return -3;
		}

		int const parts[4] = {
			hexd_to_val(data[off + 0]),
			hexd_to_val(data[off + 1]),
			hexd_to_val(data[off + 2]),
			hexd_to_val(data[off + 3])};

		if(-1 == parts[0]
		|| -1 == parts[1]
		|| -1 == parts[2]
		|| -1 == parts[3])
		{
			Log::error(
				"line contains character that is not hexadecimal digit, aborting",
				name,
				line_num);
			
			//not handled because main error is above
			//if something fails, will be handled by OS on cleanup
			(void)File::destroy(file);
			return -4;
		}

		unsigned const instruction = 0
			| (parts[0] << 12)
			| (parts[1] <<  8)
			| (parts[2] <<  4)
			| (parts[3] <<  0)
			;

		dest[line_num - 1] = instruction;

		line_num++;
		off += 5;
	}
	if(-1 == File::destroy_error_handled(file))
		return -5;

	return 0;
}
