#include "FileOut.h"

FileOut::FileOut(int maxDist, int maxVel, int usrVel, double velStep, double distStep)
{
	distRange = maxDist;
	velRange = maxVel;
	ss.str("");
	ss << "Max distance = " << maxDist << "\n";
	ss << "Max velocity = " << maxVel << "\n";
	ss << "velocity ratio= " << velStep << "\n";
	ss << "distance ratio= " << distStep << "\n";
	ss << "vel axis = ";
	for (int i = 0 - maxVel / 2; i < (maxVel + 1) / 2; i++)
	{
		ss << (static_cast<double>(i*velStep)) << ' ';
	}
	ss << "\ndistance axis = ";
	for (int i = 0; i < maxDist; i++)
	{
		ss << (static_cast<double>(i*distStep)) << ' ';
	}
	header = ss.str();
	ss.str("");
	ss << "set title \"Rdar Data\"\n"
		<< "set xlabel \"prêdkosc [m/s]\"\n"
		<< "set ylabel \"odleglosc[m]\"\n"
		<< "set ylabel \"odleglosc[m]\"\n"
		<< "set yr[" << distStep * 2 << ":*]\n"
		<< "datafile = sprintf(\"radarData[%d].dat\",n)\n"
		//<< "datafile = \"radarData[0].dat\"\n"
		<< "plot datafile every ::1 u (($1-" << maxVel / 2 + 1 << ")*" << velStep << "):(($2)*" << distStep << "):3 matrix w image\n";
	gnuplotScript = ss.str();
}

void FileOut::saveHeaderAndScripts()
{
	plikStream.open("header.dat");
	if (plikStream.good() == true)
	{
		plikStream << header;
		plikStream.close();
	}
	else throw SaveToFileException();

	plikStream.open("draw.gp");
	if (plikStream.good() == true)
	{
		plikStream << gnuplotScript;
		plikStream.close();
	}
	else throw SaveToFileException();

}

void FileOut::saveToFileWithHeader(float*** data, int index)
{
	ss.str("");
	ss << "radarData[" << index << "].dat";
	plikStream.open(ss.str());
	if (plikStream.good() == true)
	{
		for (int j = 0; j <= velRange; j++) plikStream << j << (' ');
		plikStream << ('\n');
		for (int i = 0; i < distRange; i++)
		{
			plikStream << i << (' ');
			for (int j = 0; j < velRange; j++)
			{
				plikStream << 10 * log10(sqrt(data[j][i][0] * data[j][i][0] + data[j][i][1] * data[j][i][1]));
				plikStream << (' ');
			}
			plikStream << ('\n');
		}
		plikStream.close();
	}
	else throw SaveToFileException();
}