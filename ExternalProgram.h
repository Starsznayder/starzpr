#ifndef ExternalProgram_H_INCLUDED
#define ExternalProgram_H_INCLUDED
#include <io.h>
#include <fstream>
#include <sstream>

class ExternalProgram
{
private:
	std::string code_;
	bool status;

private:
	void readALL(const char* f_ext);
	ExternalProgram();

public:
	ExternalProgram(const char* f_ext)
	{
		readALL(f_ext);
	}

	std::string get()
	{
		return code_;
	}

	bool getStat()
	{
		return status;
	}
};

#endif // ExternalProgram_H_INCLUDED
