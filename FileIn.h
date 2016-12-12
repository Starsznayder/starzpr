#ifndef FILEIN_H_INCLUDED
#define FILEIN_H_INCLUDED
#include <iostream>
#include <fstream>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
struct ReadFileException : public std::exception {
};
//struktura float2 ktora moze zostac uzyta - cl_float2 nie ma operatorow
struct float2 {
	float x, y;
	float2& operator = (float2 b){
		x = b.x;
		y = b.y;
		return *this;
	}
	float2& operator = (float b){
		x = b;
		y = b;
		return *this;
	}
	float2& operator = (int b){
		x = (float)b;
		y = (float)b;
		return *this;
	}
	float2& operator = (float* b){
		x = b[0];
		y = b[1];
		return *this;
	}
};

class FileIn{
private:
	int maxDistSample; //max odleglosc
	int maxVelocity; //max predkosc 
	int filter_depth;//gloebokosc filtru kratowego
	int decimation;//decymacja wyrazona w probkach, ktore pozostana
	int external_sample_rate;//czestotliwosc probkowania
	int usable_samples; //N, probek do obliczen
	int carrier;//nosna w MHz
	int gl; //ilosc globalnych jednostek GPU
	int sl; //ilosc lokalnych jednostek GPU
	std::vector<float2> refBuffer;
	std::vector<float2> survBuffer;
	bool ConfigOK;

public:
	FileIn() : maxDistSample(0), maxVelocity(0), filter_depth(0), decimation(0), external_sample_rate(0), usable_samples(0), carrier(0), gl(0), sl(0) {
		ConfigOK = false;
	}
	void loadConfiguration()
	{
		
		boost::property_tree::ptree pt;
		try {

			boost::property_tree::ini_parser::read_ini("config.ini", pt);
			maxVelocity = pt.get<int>("plotParams.maxVelocity");
			maxDistSample = pt.get<int>("plotParams.maxDistSample");
			filter_depth = pt.get<int>("calculationParams.filter_depth");
			decimation = pt.get<int>("calculationParams.decimation");
			carrier = pt.get<int>("calculationParams.carrier");
			external_sample_rate = pt.get<int>("calculationParams.external_sample_rate");
			usable_samples = pt.get<int>("calculationParams.usable_samples");
			gl = pt.get<int>("hardwareParams.GlobalWorkSize");
			sl = pt.get<int>("hardwareParams.LocalWorkSize");
			ConfigOK = true;
		}
		catch(std::exception &e)
		{
			ConfigOK = false;
		}

	}

	void readBinaryFile(std::string pathRef, std::string pathSurv, int length, bool conjurate)
	{
		float2 tmp;
		std::ifstream input;
		unsigned char buffer[sizeof(float)];

		input.open(pathRef, std::ios::binary | std::ios::in);
		if (!input.good()) throw ReadFileException();

			for (int i = 0; i < length; i++) {
				input.read(reinterpret_cast<char*>(buffer), sizeof(float));
				tmp.x = reinterpret_cast<float&>(buffer);

				input.read(reinterpret_cast<char*>(buffer), sizeof(float));
				if (conjurate) tmp.y = (-1) * reinterpret_cast<float&>(buffer);
				else tmp.y = reinterpret_cast<float&>(buffer);
				refBuffer.push_back(tmp);
			}
			input.close();

		input.open(pathSurv, std::ios::binary | std::ios::in);
		if (!input.good()) throw ReadFileException();
			for (int i = 0; i < length; i++) {
				input.read(reinterpret_cast<char*>(buffer), sizeof(float));
				tmp.x = reinterpret_cast<float&>(buffer);

				input.read(reinterpret_cast<char*>(buffer), sizeof(float));
				if (conjurate) tmp.y = (-1) * reinterpret_cast<float&>(buffer);
				else tmp.y = reinterpret_cast<float&>(buffer);
				survBuffer.push_back(tmp);
			}
			input.close();
	}

	int getConfigDistSample(){ return maxDistSample; }
	int getConfigMaxVelocity(){ return maxVelocity; }
	int getConfigFilter_depth(){ return filter_depth; }
	int getConfigDecimation(){ return decimation; }
	int getConfigExternal_sample_rate(){ return external_sample_rate; }
	int getConfigUsable_samples(){ return usable_samples; }
	int getConfigCarrier(){ return carrier; }
	int getConfigGlobalWorkSize(){ return gl; }
	int getConfigLocalWorkSize(){ return sl; }
	bool good() { return ConfigOK; }
	float2* getReference()
	{
		return refBuffer.data();
	}
	float2* getSurveillance()
	{
		return survBuffer.data();
	}

	~FileIn()
	{
		refBuffer.clear();
		survBuffer.clear();
	}
};

#endif // FILEIN_H_INCLUDED
