#include "./file.h"

#include <fcntl.h> //open, close
#include <sys/mman.h> //mmap, munmap
#include <unistd.h> //lseek
#include <string>

#include "./log.h"


namespace File{

	//out is modified ONLY when Error::no_error is returned
	//on success, returns Error::no_error and puts data into out
	//on error,   returns Error::create.*
	Error create(
		t_File*                out,
		std::string_view const name
		)
	{
		//check if file_name is null terminated
		if('\0' != name[name.size()])
			return Error::create_filename;

		//mmap file, more efficient than c++ way
		int const fd = open(name.data(), O_RDONLY);
		if(-1 == fd)
			return Error::create_open;
	
		int const size = lseek(fd, 0, SEEK_END);
		if(0 == size)
		{	
			close(fd);
			return Error::create_empty;
		}

		
		char* data = static_cast<char*>(mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0));
		if(MAP_FAILED == data)
		{
			close(fd);
			return Error::create_allocate;
		}
		
		out->fd   = fd;
		out->size = size;
		out->data = data;

		return Error::no_error;
	}

	//error handled version of create, logs error using Log::
	//should be used by default
	//out is modified ONLY when no_error is returned
	//on success, returns  0 and puts data into out 
	//on error,   returns -1
	int create_error_handled(
		t_File*                out,
		std::string_view const name
		)
	{
		switch(create(out, name))
		{
		case Error::no_error:
			return 0;

		case Error::create_filename:
		{
			std::string err = "file name \"";
			err.append(name);
			err.append("\" is invalid, not null terminated");
			Log::error(err);
			Log::info("how did we get here?");
			return -1;	
		}
		case Error::create_open:
		{
			std::string err = "could not open file ";
			err.append(name);
			Log::perror(err);
			return -1;
		}
		case Error::create_empty:
		{
			std::string err(name);
			err.append(" is empty file");
			Log::warning(err);
			close(out->fd);
			return -1;
		}
		case Error::create_allocate:
			Log::perror("failed to allocate memory");
			close(out->fd);
			return -1;
		default:
		{
			std::string err = "invalid error returned fom File::create() for ";
			err.append(name);
			Log::error(err);
			return -1;
		}
		}
	}

	//file may become invalid even if error is returned
	//on success, returns Error::no_error 
	//on error,   returns Error::destroy.*
	Error destroy(
		t_File          const& file
		)
	{
		if(-1 == munmap(file.data, file.size))
		{
			//hope this does not fail
			close(file.fd);
			return Error::destroy_munmap;
		}
		if(-1 == close(file.fd))
			return Error::destroy_close;
			
		return Error::no_error;
	}

	//error handled version of destroy, logs error using Log::
	//should be used by defaut
	//file may become invalid even if error is returned
	//on success, returns  0
	//on error,   returns -1
	int destroy_error_handled(
		t_File          const& file
		)
	{
		switch(destroy(file))
		{
		case Error::no_error:
			return 0;
		case Error::destroy_munmap:
			Log::perror("could not unmap file");
			return -1;
		case Error::destroy_close:
			Log::perror("could not close file");
			return -1;
		default:
		{
			std::string err = "invalid error returned fom File::destroy() for fd = ";
			err.append(std::to_string(file.fd));
			Log::error(err);
			return -1;
		}
		}
	}

};
