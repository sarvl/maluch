#pragma once

#include <string_view>

namespace File{
	enum class Error{
		no_error       ,
		create_filename,
		create_open    ,
		create_empty   ,
		create_allocate,
		destroy_munmap ,
		destroy_close  

	};

	struct t_File{
		char* data;
		int   size;
		int   fd;
	};

	//out is modified ONLY when Error::no_error is returned
	//on success, returns Error::no_error and puts data into out
	//on error,   returns Error::create.*
	Error create(
		t_File*                out,
		std::string_view const name
		);

	//error handled version of create, logs error using Log::
	//should be used by default
	//out is modified ONLY when no_error is returned
	//on success, returns  0 and puts data into out 
	//on error,   returns -1
	int create_error_handled(
		t_File*                out,
		std::string_view const name
		);

	//file may become invalid even if error is returned
	//on success, returns Error::no_error 
	//on error,   returns Error::destroy.*
	Error destroy(
		t_File          const& file
		);

	//error handled version of destroy, logs error using Log::
	//should be used by defaut
	//file may become invalid even if error is returned
	//on success, returns  0
	//on error,   returns -1
	int destroy_error_handled(
		t_File          const& file
		);
};
