#ifndef ENVCL_H_INCLUDED
#define ENVCL_H_INCLUDED

#include <memory>  
#include "ExternalProgram.h"
#include "EasyCL.h"


class EnvCL : public EasyCL
{
	private:
	
	cl_mem d_x;
	cl_mem d_y;
	cl_mem d_o2;

	cl_mem tmpBuffer;

	cl_kernel ecKernel;
	cl_kernel ecfKernel;
	cl_kernel hcfKernel;
	cl_kernel smfKernel;

	cl_kernel kernelLP2;


	bool x_isDead;
	bool o_isDead;
	bool y_isDead;

	int sumLoopSize;
	int globalWorkSizeVariable;

	int createFFTWorkSpace(int NFFT,int N, int plat, int dev, const char* source);
	void deleteWorkSpace();
	int mapTempBuffers(int N,int NFFT);
	EnvCL();

public:


	EnvCL(int N, int NFFT, int plat, int dev, size_t gl, size_t sl) {
		
		std::shared_ptr<ExternalProgram> programCL(new ExternalProgram("*.cl"));
		if (!programCL->getStat())fft_mode = true;
		else fft_mode = false;
		simple_mode = false;
		globalWorkSize = gl;
		localWorkSize = sl;
			testPass = createFFTWorkSpace(NFFT,N, plat, dev, programCL->get().c_str());
		if (!fft_mode)mapTempBuffers(N,NFFT);
	}

	
	~EnvCL() {
		deleteWorkSpace();
	}
	void clearWorkSpace(bool c_buff);

	int getWorkSpaceStatus()
	{
		return testPass;
	}

	int load(cl_float2 *a, cl_float2 *b, int N);
	int superCorrFFT(float*** out, int N, int len, int deph, int vel, int dec);
	int clutter_filter(cl_float2 *a, cl_float2 *b, int N, int deph);
	void brake();


};
#endif // ENVCL_H_INCLUDED