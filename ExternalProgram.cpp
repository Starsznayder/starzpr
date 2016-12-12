#include "ExternalProgram.h"

void ExternalProgram::readALL(const char* f_ext)
{
	std::string buffer;
	std::ifstream plik;
	std::ostringstream ss;
	ss.str("");

	_finddata_t danePliku;
	status = false;

	intptr_t przyczlap = _findfirst(f_ext, &danePliku);
	intptr_t nastepny;
	if (przyczlap >= 0)
	{


		plik.open(danePliku.name, std::ios::in);
		if (plik.good())
		{
			status = true;
			while (!plik.eof())
			{
				getline(plik, buffer);
				ss << buffer << "\n";
			}
		}
		plik.close();
		while (nastepny = _findnext(przyczlap, &danePliku) >= 0)
		{
			plik.open(danePliku.name, std::ios::in);
			while (!plik.eof())
			{
				getline(plik, buffer);
				ss << buffer << "\n";
			}
			plik.close();
		}
		_findclose(przyczlap);
	}
	code_ = ss.str();
}