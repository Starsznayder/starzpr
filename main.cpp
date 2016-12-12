#include <iostream>
#include <CL/cl.h>
#include <sstream>
#include <ctime>
#include <thread>

#include "gnuplot-iostream.h"
#include "envCL.h"
#include "FileOut.h"
#include "FileIn.h"



struct oneSmallStep {
	int FFT_len;//probek po decymacji
	int dec_step;//krok decymacji
	double vel_multiplier;//przelicznik predkasci
	double distance_multiplier;//przelicznik odleglosci
};

static const int LIGHT_SPEED = 300;//przedkosc swiatla w nieznanych jednastkach - wszystkie liczby sa strasznie dlugie, mozna zjesc pare zer

float*** outputV;//tablica z danymi wyjsciowymi - dla Matlaba


std::ostringstream gpuTest();
void calcStep(oneSmallStep *resoult, int sampleRate, int carrier, int vel, int decimation, int external_sample_rate);
//cl_float2* readBinaryFile(std::string path, int length, bool conjurate);

//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------MAIN-----------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{

	
	clock_t timeL;//do pomiaru czasu obliczen
	oneSmallStep steps;
	std::ifstream input;
	cl_float2* ref;
	cl_float2* surv;
	std::string lock;
	FileIn inFile;


	std::cout << gpuTest().str();

	inFile.loadConfiguration();
	timeL = clock();
	std::cout << "\nLoading signal data...\n";
	try {
		inFile.readBinaryFile("G:\\sugnaly\\syntetyczne\\refX","G:\\sugnaly\\syntetyczne\\SuvX2",inFile.getConfigUsable_samples(), false);
	}
	catch (ReadFileException &e) {
		std::cout << "Input data error\n";
	}

	ref = (cl_float2*)inFile.getReference();//readBinaryFile("G:\\sugnaly\\syntetyczne\\refX", usable_samples, false);
	surv = (cl_float2*)inFile.getSurveillance(); //readBinaryFile("G:\\sugnaly\\syntetyczne\\SuvX2", usable_samples, false);

	timeL = clock() - timeL;
	std::cout << "DONE!\nHDD data read time : "<< timeL << " [ms]\n";
	

	std::shared_ptr<EnvCL> srodowisko(new EnvCL(inFile.getConfigUsable_samples(), inFile.getConfigDecimation(), 1, 0, inFile.getConfigGlobalWorkSize(), inFile.getConfigLocalWorkSize())); //mam dwie karty, u mnie ta do obliczen to platforma 1 urzadzenie 0, jak ktos ma tylko jedna grafe to moze byc 0 i 0.
	srodowisko->brake();
	calcStep(&steps, inFile.getConfigUsable_samples(), inFile.getConfigCarrier(), inFile.getConfigMaxVelocity(), inFile.getConfigDecimation(), inFile.getConfigExternal_sample_rate());// obliczenie przlicznikow odleglosci i predkosci
	std::cout << "\nSetting up Matlab graph for parameters:\n" << "N : " << inFile.getConfigUsable_samples() << "\ncarrier : " << inFile.getConfigCarrier() << "\nFFT length : " << inFile.getConfigDecimation() << "\ndisplayed FFT length : " << steps.FFT_len << "\ndecimation step : " << steps.dec_step << "\ndistance step : " << steps.distance_multiplier << "\nvelocity step : " << steps.vel_multiplier << "\n";
	std::shared_ptr<FileOut> plikiWyjsciowe(new FileOut(inFile.getConfigDistSample(), steps.FFT_len, inFile.getConfigMaxVelocity(), steps.vel_multiplier, steps.distance_multiplier));
	try {
		plikiWyjsciowe->saveHeaderAndScripts();
	}
	catch (SaveToFileException &e)
	{
		std::cout << "\nFile output Error\n";
	}

	outputV = new float**[steps.FFT_len];

	for (int i = 0; i < steps.FFT_len; i++)
	{
		outputV[i] = new float*[inFile.getConfigDistSample()];
		for (int j = 0; j < inFile.getConfigDistSample(); j++)
		{
			outputV[i][j] = new float[2];
			outputV[i][j][0] = 0;
			outputV[i][j][1] = 0;
		}
	}

	

	timeL = clock();
		srodowisko->load(ref, surv, inFile.getConfigUsable_samples());
	timeL = clock() - timeL;
	std::cout << "Load global memory time : " << timeL << " [ms]\n";//informacja o czasie na konsole GUI
	
	

	timeL = clock();//pomiar czasu obliczen finkcji nieoznaczonosci wzajemnej
		srodowisko->superCorrFFT(outputV, inFile.getConfigUsable_samples(), inFile.getConfigDecimation(), inFile.getConfigDistSample(), steps.FFT_len, steps.dec_step);//oblicznie zbioru wartosci funkcji nieoznaczonosci w rozpatrywanym przedziale
	timeL = clock() - timeL;
	std::cout << "\nFull corr (corr x depth x vel) time : " << timeL << " [ms] \n";

	timeL = clock();//pomiar czasu rysowania
		try {
			plikiWyjsciowe->saveToFileWithHeader(outputV, 0);
		}
		catch (SaveToFileException &e)
		{
			std::cout << "\nFile output Error\n";
		}
	timeL = clock() - timeL;
	std::cout << "\nDraw time : " << timeL << " [ms] \n";//informacja o czasie rysowania na konsole

	Gnuplot gp;
	gp << "n=0"<< std::endl <<"load 'draw.gp'" << std::endl;

	//system();
	//system("draw.gp");

	std::cin >> lock;
	return 0;
}


//pobiranie info o dostepnym sprzecie (GPU)
std::ostringstream gpuTest() 
{
	std::ostringstream ss;
	EasyCL* testenv = new EasyCL(); //obiekt testujacy sprzet do obliczen

	ss << testenv->getCharInfo();//pobranie informacji na konsole
	delete testenv;//kasowanie obiektu testowego
	
	return ss;
}

void calcStep(oneSmallStep *resoult, int localSampleRate, int carrier, int vel, int decimation, int external_sample_rate)//obliczanie rzelicznikow przedkosci i odleglosci
{//przeliczniki sa dobre - spawdzone doswiadczalnie
	int FFT_len = (int)(((float)localSampleRate / (float)external_sample_rate)*(4 * (float)carrier*(float)vel / (float)LIGHT_SPEED));
	//w FFT czesotliwosc/probke bedzie 1Hz zawsze, fd ma sie do tego tak jak kot do sera. 
	int dec = localSampleRate / decimation;
	//FFT_len = sampleRate / dec;
	double distStep = (1000000 / (double)external_sample_rate)*(double)LIGHT_SPEED / 2; //jedyne co trace przy zmniejszeniu fs to rozroznialnosc odleglosci.


	resoult->FFT_len = FFT_len;
	resoult->dec_step = dec;
	resoult->distance_multiplier = distStep;
	resoult->vel_multiplier = ((float)external_sample_rate) / ((float)localSampleRate)*((float)LIGHT_SPEED) / (2 * (float)carrier);
}

