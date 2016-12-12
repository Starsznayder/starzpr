#include "envCL.h"

//tworzy przestren robacza wlasciwie dla calego programu
//umozliwia korzystanie z kerneli openCL i clFFT
int EnvCL::createFFTWorkSpace(int NFFT, int N, int plat, int dev, const char* source)
{
	cl_int err;
	cl_int buildErr;

	size_t clLengths1[1] = { (size_t)NFFT };//wymiary FFTa
	//clfftDim dim = CLFFT_1D;
	cl_context_properties props[3];
	props[0] = CL_CONTEXT_PLATFORM;
	props[1] = 0;
	props[2] = 0;
	cl_platform_id *platforms;
	cl_device_id *devices;
	err = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (err != CL_SUCCESS) return -1;


	platforms = new cl_platform_id[numPlatforms];

	/* Setup OpenCL environment. */
	err = clGetPlatformIDs(numPlatforms, platforms, NULL);
	if (err != CL_SUCCESS) return -1;


	numDevices = 0;

	err = clGetDeviceIDs(platforms[plat], CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);//zmieniony indeks platforms[], bylo 0
	if (err != CL_SUCCESS) return -1;
	if (numDevices == 0) return -1;

	//devices = (cl_device_id*)malloc(numDevices*sizeof(cl_device_id));
	devices = new cl_device_id[numDevices];
	err = clGetDeviceIDs(platforms[plat], CL_DEVICE_TYPE_GPU, numDevices, devices, NULL);
	if (err != CL_SUCCESS) return -1;

	props[1] = (cl_context_properties)platforms[plat];
	context = clCreateContext(props, 1, &devices[dev], NULL, NULL, &err);
	if (err != CL_SUCCESS) return -1;
	cmdQueue = clCreateCommandQueue(context, devices[dev], 0, &err);
	if (err != CL_SUCCESS) return -1;

	/* Setup clFFT. */
	clfftSetupData fftSetup;
	err |= clfftInitSetupData(&fftSetup);
	err |= clfftSetup(&fftSetup);

	/* Create a default plan for a complex FFT. */

	err = clfftCreateDefaultPlan(&planHandle, context, dim, clLengths1);
	if (err != CL_SUCCESS) return -1;
	/* Set plan parameters. */
	err = clfftSetPlanPrecision(planHandle, CLFFT_SINGLE);
	if (err != CL_SUCCESS) return -1;
	err = clfftSetLayout(planHandle, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
	if (err != CL_SUCCESS) return -1;
	err = clfftSetResultLocation(planHandle, CLFFT_INPLACE);
	if (err != CL_SUCCESS) return -1;

	size_t tmpBufferSize = 0;

	int status = 0;
	status = clfftGetTmpBufSize(planHandle, &tmpBufferSize);
	if ((status == 0) && (tmpBufferSize > 0)) {
		tmpBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE, tmpBufferSize, 0, &err);
		if (err != CL_SUCCESS)return -1;
	}

	//clearWorkSpace(true);
	if (source != NULL)
	{
		//---------------------------------------------Tworzenie programu oCL-----------------------------------------------------
		program = clCreateProgramWithSource(context, 1, (const char**)&source, NULL, &err);
		if (err != CL_SUCCESS) return -1;


		buildErr = clBuildProgram(program, 1, &devices[dev], NULL, NULL, NULL);
		if (buildErr != CL_SUCCESS) return -1;
		//---------------------------------------------Tworzenie kerneli Korelacje------------------------------------------------

		kernelLP2 = clCreateKernel(program, "fLP2", &err);
		if (err != CL_SUCCESS) return -1;
		//---------------------------------------------Tworzenie kerneli Filtr----------------------------------------------------
		// TO DO

	}
	o_isDead = true;
	x_isDead = true;
	y_isDead = true;

	globalWorkSizeVariable = static_cast<int>(globalWorkSize);

	delete [] platforms;
	delete [] devices;
	return 0;
}


void EnvCL::brake()
{
	clfftBakePlan(planHandle, 1, &cmdQueue, NULL, NULL);
}


//Zajecie pamieci dla buforow tymczasowych
int EnvCL::mapTempBuffers(int N, int NFFT)
{
	cl_int err;
	d_o2 = clCreateBuffer(context, CL_MEM_READ_WRITE, NFFT *sizeof(cl_float2), NULL, &err);
	if (err != CL_SUCCESS) return -1;
	o_isDead = false;
	return 0;
}

int EnvCL::load(cl_float2 *a, cl_float2 *b, int N)
{
	cl_int err;
	d_x = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, N  * sizeof(cl_float2), a, &err);
	if (err != CL_SUCCESS) return -1;
	x_isDead = false;
	d_y = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, N  * sizeof(cl_float2), b, &err);
	if (err != CL_SUCCESS) return -1;
	y_isDead = false;

	return 0;
}


//Korelacje/decymacje/filtr fLP2
int EnvCL::superCorrFFT(float*** out, int N, int len, int deph, int vel, int dec)
{
	cl_float2* temp;
	cl_int err;
	temp = new cl_float2[len];//tymczasowy wektor wynikow
	int j = 0;
	int filterPASS = vel;
	int offset = 0;

//-----------------------------------------Przypisanie wskaznikow na pamiec zmiennym kerneli------------------------------------------
	clSetKernelArg(kernelLP2, 0, sizeof(cl_mem), &d_x);
	clSetKernelArg(kernelLP2, 1, sizeof(cl_mem), &d_y);
	clSetKernelArg(kernelLP2, 2, sizeof(cl_mem), &d_o2);



//------------------------------W petli dla kolejnych opoznien w korelacji - przesuniecie w odleglosci--------------------------------
	for (offset = 0; offset < deph; offset++)
	{
//-----------------------------------Przypisanie wskaznikow zmiennych globalnych parametrom kerneli-----------------------------------
		err |= clSetKernelArg(kernelLP2, 3, sizeof(int), &dec);
		err |= clSetKernelArg(kernelLP2, 4, sizeof(int), &offset);
		err |= clSetKernelArg(kernelLP2, 5, sizeof(int), &N);

		//---------------------------------------------------------------DECYMACJA--------------------------------------------------------------------
		//-----------------------------------------------------------Wstepna filtracja----------------------------------------------------------------
		//Zliczenie sum probek, mnozonych* alfa pomiedzy probkami decymowanymi (hybryda IIR/FIR)
		err |= clEnqueueNDRangeKernel(cmdQueue, kernelLP2, 1, NULL, &globalWorkSize, &localWorkSize, 0, NULL, NULL);


		
		//-----------------------------------------------------------------FFT------------------------------------------------------------------------
		//Przy decymacji stosuje sie lekki oversampling, wiec liczone FFT jest wyraznie dlozsze od potrzebnego
		err = clfftEnqueueTransform(planHandle, CLFFT_FORWARD, 1, &cmdQueue, 0, NULL, NULL, &d_o2, NULL, tmpBuffer);

		//----------------------------------------------------------------ODCZYT----------------------------------------------------------------------
		err |= clEnqueueReadBuffer(cmdQueue, d_o2, CL_TRUE, 0, len * sizeof(cl_float2), temp, 0, NULL, NULL);
		if (err != CL_SUCCESS) return -1;
		
		//przeksztalczenie zassanego wektora na forme zrozumiala dla Matlaba (taki przyjeto model macierzy wynikow funkcji nieoznaczonosci)
		for (int i = vel / 2; i < vel; i++)
		{
			out[i][offset][0] = temp[i - vel / 2].x;
			out[i][offset][1] = temp[i - vel / 2].y;
		}
		for (int i = 0; i < vel / 2; i++)
		{
			out[i][offset][0] = temp[len - vel / 2 + i].x;
			out[i][offset][1] = temp[len - vel / 2 + i].y;
		}
	}

	//-----------------------------------------Czyszczenie wektorow wejsciowych i bufora fft----------------------------------------------
	clearWorkSpace(false);
	delete[] temp;
	
	return 0;
}

//jak w nazwie 
int EnvCL::clutter_filter(cl_float2 *a, cl_float2 *b, int N, int deph) //Filtr odbic od obiektow nieruchomych
{
	//TO DO
	return 0;
}

void EnvCL::deleteWorkSpace()
{
	clearWorkSpace(true);

	if (fft_mode)
	{
		clReleaseKernel(kernelLP2);
	}
	clReleaseMemObject(tmpBuffer);
}

void EnvCL::clearWorkSpace(bool c_buff)
{
	if (!x_isDead) {//bufor wejsciowy ref
		clReleaseMemObject(d_x);
		x_isDead = true;
	}
	if (!y_isDead) {//bufor wejsciowy srv
		clReleaseMemObject(d_y);
		y_isDead = true;
	}
	if (!o_isDead && c_buff) { //bufory tymczasowe
		clReleaseMemObject(d_o2);
		o_isDead = true;
	}
}