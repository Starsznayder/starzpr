#ifndef EASYCL_H_INCLUDED
#define EASYCL_H_INCLUDED
#include<CL/cl.h>
#include "fft/clFFT.h"
#include <sstream>
#include <cmath>


//Klasa bazowa dla obiektow odpowierzialnych za operacje w oCL
//Zawiera najbardziej podstawowe metody
class EasyCL
{
	protected://zmienne dziedziczne kluczowe dla dzialania programu
		cl_uint numPlatforms;
		cl_context context;
		cl_command_queue cmdQueue; //wybitnie wazny: kolejka, chodzi o to zeby nie tworzyc jej wtrakcie obliczen tylko raz podczas uruchamiania 

		cl_program program;
		cl_uint numDevices;
		cl_uint numDevicestest[4];

		clfftPlanHandle planHandle;

		clfftDim dim = CLFFT_1D; //wszystkie liczone FFTy sa jednowymiarowe
		
		//kilka zmiennych pozawalajacych na nie wykorzystywanie pelnych mozliwosci obiektu i oszczednosc pamieci
		//gdyby ich nie bylo to bylyby klasy dziedziczace... duzo klas, flagi wynikaja z lenistwa autora programu
		bool fft_mode;
		bool simple_mode;

		std::string infoOut; //informacje o sprzecie dla GUI

		int testPass; //czy udalo sie utworzyc obiekt - wtedy jest 0, jesli nie to -1

		size_t globalWorkSize;
		size_t localWorkSize;

		cl_int testWorkSpace();
		//prosta metoda pozwalajaca na sprawdzenie czy obiekt moze powstac i jakie moze miec konfiguracje (wzgledy sprzetowe)

		int createFFTWorkSpace(int NFFT, int plat, int dev, const char* source);
		//utworzenie podstawowej wersji srodowiska pozwalajacej np. na liczenie FFT... wlasciwie to tylko FFT
		void deleteWorkSpace();//metoda destruktora
		void clearWorkSpace(bool c_buff) {};//czyszczenie buforow tymczasowych, w tej klasie bezuzyteczna
		
public:
	//tzw konstruktor probny - sprawdzenie czy da sie utworzyc obiekt
	//autor nie rzyczy sobie konstruktorow bezargumentowych - dlatego parametr
	EasyCL()
	{
		testPass = testWorkSpace();
		fft_mode = false;
		simple_mode = true;
	}
	//powstal dla celow testowych: do usuniecia


	//Tworzy w pelni sprawny obiekt tej prostej klasy ->umozliwia liczenie FFT o wymiarez NFFT, na platformie plat i urzadzeniu dev
	EasyCL(const char* source, int NFFT, int plat, int dev, size_t gl, size_t sl) {
		fft_mode = true;
		simple_mode = false;
		globalWorkSize = gl;
		localWorkSize = sl;

		testPass = createFFTWorkSpace(NFFT, plat, dev, source);
	}


	virtual ~EasyCL() {
		deleteWorkSpace();
	}
	


	cl_int testGPU();	//nie potrzebne
	std::string getCharInfo(); //zwraca informacje o sprzecie
	int* getHardInfo(int* numPlat); //zwraca info o konkretnym urzadzeniu
	

	int getWorkSpaceStatus() //info o sukcesie utworzenia obiektu
	{
		return testPass;
	}


	int calcfft(cl_float2* b, int N, const char* direction);//oblicza FFT w kierunku direction w rozmiarze N, w miejscu
	virtual int superCorrFFT(float*** out, int N, int len, int deph, int vel, int dec) { return 0; };
	virtual int clutter_filter(cl_float2 *a, cl_float2 *b, int N, int deph) { return 0; };
	virtual void brake() {};
	//int mapTempBuffers(int NFFT) {};
	

};
#endif // EASYCL_H_INCLUDED