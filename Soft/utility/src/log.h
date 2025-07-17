#pragma once

#include <string_view>

namespace Log{
	enum class Level{
		error   = 0,
		warning = 1,
		info    = 2
	};

	extern Level log_level ;
	extern bool  is_error  ;
	extern bool  is_warning;
	extern bool  error_abort;

	void internal_error(
		std::string_view const str, 
		std::string_view const file_name,
		int              const line_num
		);
	void internal_error(
		std::string_view const str
		);

	void error(
		std::string_view const str, 
		std::string_view const info,
		std::string_view const file_name,
		int              const line_num
		);
	void error(
		std::string_view const str, 
		std::string_view const file_name,
		int              const line_num
		);
	void error(
		std::string_view const str
		);
	void perror(
		std::string_view const str
		);

	void warning(
		std::string_view const str, 
		std::string_view const info,
		std::string_view const file_name,
		int              const line_num
		);
	void warning(
		std::string_view const str, 
		std::string_view const file_name,
		int              const line_num
		);
	void warning(
		std::string_view const str 
		);

	void info(
		std::string_view const str, 
		std::string_view const file_name,
		int              const line_num
		);
	void info(
		std::string_view const str 
		);
};
