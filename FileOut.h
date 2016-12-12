#ifndef FILEOUT_H_INCLUDED
#define FILEOUT_H_INCLUDED
#include <fstream>
#include <sstream>

struct SaveToFileException : public std::exception {
};

class FileOut/*: public HeaderTable*/ {
	private:
		std::string header;
		std::string gnuplotScript;
		std::ostringstream ss;
		std::ofstream plikStream;
		int distRange;
		int velRange;
		FileOut();

	public:
		FileOut(int maxDist, int maxVel, int usrVel, double velStep, double distStep);
		void saveHeaderAndScripts();
		void saveToFileWithHeader(float*** data, int index);
};

#endif // FILEOUT_H_INCLUDED

