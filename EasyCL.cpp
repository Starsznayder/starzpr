#include "EasyCL.h"


cl_int EasyCL::testWorkSpace()
{
	cl_int err;
	cl_platform_id *platforms;
	cl_device_id *devices;
	std::ostringstream ss;

	infoOut = "";
	//-------------------Sprawdzenie czy jest platforma-------------------------
	err = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (err != CL_SUCCESS) {
		infoOut += "clGetPlatformIDs failed\r\n";
		return err;
	}

	if (numPlatforms == 0) {
		infoOut += "No platforms detected.\r\n";
		return err;
	}

	platforms = new cl_platform_id[numPlatforms];

	clGetPlatformIDs(numPlatforms, platforms, NULL);
	ss << numPlatforms;
	infoOut += ss.str();
	ss.str("");
	infoOut += " platforms found \r\n";
	//std::cin >> g;
	for (unsigned int i = 0; i < numPlatforms; i++) {
		char buff[100];
		infoOut += "Platform: ";
		ss << i;
		infoOut += ss.str();
		ss.str("");
		infoOut += " \r\n";
		err = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(buff), buff, NULL);
		infoOut += "Vendor: ";
		infoOut += buff;
		infoOut += " \r\n";
		err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(buff), buff, NULL);
		infoOut += "Name: ";
		infoOut += buff;
		infoOut += " \r\n";
		if (err != CL_SUCCESS) {
			infoOut += "clGetPlatformInfo failed\n";
			//return -1;
		}
	}

	for (unsigned int i = 0; i < numPlatforms; i++)
	{
		infoOut += "\r\n";
		err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &numDevicestest[i]);
		if (err != CL_SUCCESS) {
			infoOut += "clGetDeviceIDs failed\r\n";
			//return -1;
		}
		if (numDevicestest == 0) {
			infoOut += "No devices found\r\n";
			//return -1;
		}

		devices =new cl_device_id[numDevicestest[i]];

		err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, numDevicestest[i], devices, NULL);
		infoOut += "Platform: ";
		ss << i;
		infoOut += ss.str();
		ss.str("");
		infoOut += " \r\n";
		infoOut += "devices found : ";
		ss << numDevicestest[i];
		infoOut += ss.str();
		ss.str("");
		infoOut += " \r\n";
		for (unsigned int j = 0; j < numDevicestest[i]; j++) {
			char buff[100];
			infoOut += "Device: ";
			ss << j;
			infoOut += ss.str();
			ss.str("");
			infoOut += " \r\n";
			err = clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, sizeof(buff), buff, NULL);
			infoOut += "Vendor: ";
			infoOut += buff;
			infoOut += " \n";
			err = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(buff), buff, NULL);
			infoOut += "Name: ";
			infoOut += buff;
			infoOut += " \r\n";
			if (err != CL_SUCCESS) {
				infoOut += "clGetDeviceInfo failed\r\n";
				//return -1;
			}
		}
		delete [] devices;
	}
	delete [] platforms;

	return err;
}

std::string EasyCL::getCharInfo()
{
	return infoOut;
}


void EasyCL::deleteWorkSpace()
{
	if (!simple_mode)
	{
		/* Release the plan. */
		clfftDestroyPlan(&planHandle);
		
		if (fft_mode)
		{
			/* Release clFFT library. */
			clfftTeardown();
			
		}
		
		clReleaseProgram(program);
		clReleaseContext(context);
		clReleaseCommandQueue(cmdQueue);
		
	}

}