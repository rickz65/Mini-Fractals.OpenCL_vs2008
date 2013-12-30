/**********************************************************************
Copyright ©2012 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
•	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/

#include <iostream>
using namespace std;
 
#include "..\headers\Mandelbrot.hpp"

#include <vector>
// #include <CL/cl.hpp>
// #include <iostream>
#include <fstream>
// #include <string>

using namespace streamsdk;   /// fgz 12-29-2013 was: namespace cl;

#ifndef cl
#define cl  
#endif

#ifndef min
int min(int a1, int a2)
{
    return ((a1 < a2) ? a1 : a2);
}
#endif





/**
* Class Rect
* Class Rect describes rectangles with lengths and widths.
*/
//template <typename T> class Rect;

template <typename T>
class Rect {
	T myLength; /**< length of the rectangle */
	T myWidth; /**< width of the rectangle */
public:
	Rect () {
	}
	Rect (T a, T b) {
		myLength = a;
		myWidth = b;
	}
	T get_length(){
		return myLength;
	}

	T get_width(){
		return myWidth;
	}
};



/******************************************************************************* 
*  @fn     verification 
*  @brief  This function is to check if the.   
*             
*  @param[in] area : area calculated by CPU 
*  @param[in] devArea : area calculated by GPU
*  @param[in] count : size of array 
*           
*  @return int : 0 if successful; otherwise 1. 
*******************************************************************************/

template<typename T>
int verification(T* area, T* devArea,  int count, const char* option)
{

	std::cout <<std::endl<<"The area of the first myRectangle by CPU is:      "<<area[0]<<std::endl; 
	std::cout <<"The area of the first myRectangle by GPU is:      "<<devArea[0]<< "      "<<std::endl;
	std::cout <<std::endl<< "Continue validating..." <<std::endl;

	int isCorrect = 1;
	for (int i = 1; i <= count; i++)
	{
		if((area[i-1] - devArea[i-1])>1e-4)
		{
			isCorrect = 0;
			break;
		}
	}


	if(isCorrect == 1)
	{
		std::cout <<option<<" static C++ kernel is working correctly!!!"<<std::endl<<std::endl<<std::endl;  
		return 0;
	}
	else
	{
		std::cout <<option<<" static C++ kernel is NOT working correctly!!!"<<std::endl<<std::endl<<std::endl;   
		return 1;
	}
};















int 
Mandelbrot::setupMandelbrot()
{
    cl_uint sizeBytes;

    // allocate and init memory used by host.
    sizeBytes = width * height * sizeof(cl_uchar4);
    output = (cl_uint *) malloc(sizeBytes);
    CHECK_ALLOCATION(output, "Failed to allocate host memory. (output)");

    if(verify)
    {
        verificationOutput = (cl_uint *)malloc(sizeBytes);
        CHECK_ALLOCATION(verificationOutput, "Failed to allocate host memory. (verificationOutput)");
    }

	// FGZ -- initialize PC-Freq
	
	LARGE_INTEGER pc_freq;
	if( QueryPerformanceFrequency( &pc_freq ) == FALSE ) m_PC_Freq = 1000.0 ;
	else m_PC_Freq = double(pc_freq.QuadPart);
	
	
	printf("Got here -- setup Mandel \n");
	// end fgz

    return SDK_SUCCESS;
}

int 
Mandelbrot::genBinaryImage()
{
    streamsdk::bifData binaryData;
    binaryData.kernelName = std::string("BlackMandel_Kernels.cl");
    binaryData.flagsStr = std::string("");
    if(isComplierFlagsSpecified())
        binaryData.flagsFileName = std::string(flags.c_str());

    binaryData.binaryName = std::string(dumpBinary.c_str());
    int status = sampleCommon->generateBinaryImage(binaryData);
    return status;
}

// FGZ ... 12-29-2013 this has to be defined somewhere ... seems to be int or int pointer ...

#define Platform  cl_int 

int Mandelbrot::setupCLcpp()
{
	// FGZ - CPP example insertion ...

	cl_int status = 0;
	const char* buildOption ="-x clc++ ";
	const int LIST_SIZE = 128;

	// Create and initiate arrays of class Rect <float> myRectangle and areas.

	Rect <float> myRectangleF[LIST_SIZE];
	float myAreaF[LIST_SIZE];
	float myDevAreaF[LIST_SIZE];

	Rect <int> myRectangleI[LIST_SIZE];
	int myAreaI[LIST_SIZE];
	int myDevAreaI[LIST_SIZE];

	srand (10);
	for(int i = 0; i < LIST_SIZE; i++) 
	{
		myRectangleF[i] =Rect <float> ((rand() % 15) + (float) 0.1, (rand() % 15) + (float) 0.1);
		myAreaF[i] = myRectangleF[i].get_length() * myRectangleF[i].get_width();
		myDevAreaF[i] = 0;

		myRectangleI[i] =Rect <int> ((rand() % 15) + 1, (rand() % 15) + 1);
		myAreaI[i] = myRectangleI[i].get_length() * myRectangleI[i].get_width();
		myDevAreaI[i] = 0;
	}

	// Get available platforms
	std::vector<cl_int> platforms;
	// FGZ 12-29-2013 changing to un-wrapped ...
	__int64 sz_T;
	_cl_platform_id MY_IDs ;

	
	status = clGetPlatformInfo( &MY_IDs, NULL, sizeof(cl_platform_id), &platforms, *sz_T);
	if (status != CL_SUCCESS)
	{
		std::cout<<"Error: Getting platforms!"<<std::endl;
		return SDK_FAILURE;
	}
	std::vector<Platform>::iterator iter;
	for(iter = platforms.begin(); iter != platforms.end(); ++iter)
	{
		if(!strcmp((*iter).getInfo<CL_PLATFORM_VENDOR>().c_str(), "Advanced Micro Devices, Inc."))
		{
			break;
		}
	}
	cl_context_properties cps[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(*iter) (), 0};
	Context context = cl::Context(CL_DEVICE_TYPE_GPU, cps, NULL, NULL, &status);

	if (status != CL_SUCCESS)
	{
		std::cout<<"GPU not found, falling back to CPU!"<<std::endl;
		context = cl::Context(CL_DEVICE_TYPE_CPU, cps, NULL, NULL, &status);
		if (status != CL_SUCCESS)
		{
			std::cout<<"Error: Creating context!"<<std::endl;
			return SDK_FAILURE;
		}
	}
	 
	 
	try
	{

		// Get a list of devices on this platform
		std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

		// Create a command queue and use the first device
		CommandQueue queue = CommandQueue(context, devices[0]);

		// Read source file
		std::ifstream sourceFile("Mandelbrot_cpp_Kernels.cl");
		std::string sourceCode(
			std::istreambuf_iterator<char>(sourceFile),
			(std::istreambuf_iterator<char>()));
		Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length()+1));

		// Make program of the source code in the context
		Program program = Program(context, source);

		// Build program for these specific devices
		program.build(devices, buildOption);


		////////////////////////  move to run-kernels_cpp 

		// Make kernel
		Kernel kernelF(program, "AreaCalculation_float");
		Kernel kernelI(program, "AreaCalculation_int");

		// Create memory buffers
		Buffer bufferMyRectangleF = Buffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(Rect <float>));
		Buffer bufferMyAreaF = Buffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(float));
		Buffer bufferMyRectangleI = Buffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(Rect <int>));
		Buffer bufferMyAreaI = Buffer(context, CL_MEM_READ_WRITE, LIST_SIZE * sizeof(int));

		// Copy myRectangle[LIST_SIZE] to the memory buffers
		queue.enqueueWriteBuffer(bufferMyRectangleF, CL_TRUE, 0, LIST_SIZE * sizeof(Rect <float>), myRectangleF);
		queue.enqueueWriteBuffer(bufferMyRectangleI, CL_TRUE, 0, LIST_SIZE * sizeof(Rect <int>), myRectangleI);

		// Set arguments to kernel
		kernelF.setArg(0, bufferMyRectangleF);
		kernelF.setArg(1, bufferMyAreaF);
		kernelI.setArg(0, bufferMyRectangleI);
		kernelI.setArg(1, bufferMyAreaI);

		// Run the kernel on specific ND range
		NDRange global(LIST_SIZE);  
		NDRange local(LIST_SIZE / 2);      
		queue.enqueueNDRangeKernel(kernelF, NullRange, global, local);
		queue.enqueueNDRangeKernel(kernelI, NullRange, global, local);

		// fgz - wait here ?? 

		// Read buffer back to CPU
		queue.enqueueReadBuffer(bufferMyAreaF, CL_TRUE, 0, LIST_SIZE * sizeof(float), myDevAreaF);
		queue.enqueueReadBuffer(bufferMyAreaI, CL_TRUE, 0, LIST_SIZE * sizeof(int), myDevAreaI);

		//////////////////////


	}catch(cl::Error e){
		std::cout << "Line "<< __LINE__<<": Error in "<<e.what() <<std::endl;
		return SDK_FAILURE;
	}

	return SDK_SUCCESS;
}


int 
Mandelbrot::runCLKernels_cpp(void)
{
    cl_int   status;
    cl_kernel kernel;
    cl_event events[MAX_DEVICES];
    cl_int eventStatus = CL_QUEUED;

	std::size_t globalThreads[1];
    std::size_t localThreads[1];

	// FGZ - moved these here, reason: scope and loss of variable memory before calling kernels ...		
	cl_float leftxF ; // = (float)leftx;
            cl_float topyF ; // = (float)topy;
            cl_float xstepF ; //= (float)xstep;
            cl_float ystepF ; //= (float)ystep;
            cl_float pow_Ff ; // = (float)pow_F ;
			cl_float logpowf = (float)log(pow_F) ;  // do this once, here

    benched = 0;
	globalThreads[0] = (width * height * int(2.0 * ((enableDouble)? 1 : 0.5)) ) / numDevices;    // FGZ -- NOTE: for Double x2 the * 2 is needed to complete the image !!
    localThreads[0]  = 64; // this seems to be maxed out at 256. crashes with 384.  // this is the # of local work-groups (max for ATI 7xxx)

    globalThreads[0] >>= 2;  // this is the total number of threads requested.

    for (cl_uint i = 0; i < numDevices; i++)
    {
        kernel = kernel_vector[i];

        // Check group size against kernelWorkGroupSize 
        status = clGetKernelWorkGroupInfo(kernel,
                                          devices[i],
                                          CL_KERNEL_WORK_GROUP_SIZE,
                                          sizeof(std::size_t),
                                          &kernelWorkGroupSize,
                                          0);
        CHECK_OPENCL_ERROR(status, "clGetKernelWorkGroupInfo failed.");

        if((cl_uint)(localThreads[0]) > kernelWorkGroupSize)
            localThreads[0] = kernelWorkGroupSize;
		// kernelWorkGroupSize = 64 ;  // read that workgroup size should be 64 : result 35ms down to 32ms

        double aspect = (double)width / (double)height;
        xstep = (xsize / (double)width);
        // Adjust for aspect ratio
        double ysize = xsize / aspect;
        ystep = (-(xsize / aspect) / height);
        leftx = (xpos - xsize / 2.0);
        topy = (ypos + ysize / 2.0 -((double)i * ysize) / (double)numDevices);
		 

        if(i == 0)
        {
            topy0 = topy;
        }
		 
        int np = 0 ;
		// Set appropriate arguments to the kernel 
        status = clSetKernelArg(
                        kernel, 
                        np++, 
                        sizeof(cl_mem),
                       (void *)&outputBuffer[i]);
        CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (outputBuffer)");
		

        if(enableDouble) 
        {
			status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_double),
                           (void *)&pow_F);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (pow_F)");

            status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_double),
                           (void *)&leftx);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (leftx)");

            status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_double),
                           (void *)&topy);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (topy)");

            status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_double),
                           (void *)&xstep);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (xstep)");

            status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_double),
                           (void *)&ystep);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (ystep)");
        }
        else
        {
            // fgz - float variables were defined here ... then pointers to those floats ...
			 leftxF = (float)leftx;
             topyF = (float)topy;
             xstepF = (float)xstep;
             ystepF = (float)ystep;
             pow_Ff = (float)pow_F ;
			 
			status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_float), 
                           (void *)&pow_Ff);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (pow_F)");
			/* 
			status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_float),
                           (void *)&logpowf);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (logpowf)");
			*/
			status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_float),
                           (void *)&leftxF);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (leftxF)");

            status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_float),
                           (void *)&topyF);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (topyF)");

            status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_float),
                           (void *)&xstepF);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (xstepF)");

            status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_float),
                           (void *)&ystepF);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (ystepF)");
        }
        status = clSetKernelArg(
                        kernel, 
                        np++, 
                        sizeof(cl_uint), 
                        (void *)&maxIterations);
        CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (maxIterations)");

        // width - i.e number of elements in the array 
        status = clSetKernelArg(
                        kernel, 
                        np++, 
                        sizeof(cl_int), 
                        (void *)&width);
        CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (width)");

        // bench - flag to indicate benchmark mode 
        status = clSetKernelArg(
                        kernel, 
                        np++, 
                        sizeof(cl_int), 
                        (void *)&bench);
        CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (bench)");

    
		 

		/* 
         * Enqueue a kernel run call.
         */
        status = clEnqueueNDRangeKernel(
                commandQueue[i],
                kernel,
                1,
                NULL,
                globalThreads,
                localThreads,
                0,
                NULL,
                &events[i]);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");
    }
    

	//  START --HERE
	__int64 Start64 = Get_PC_Count() ;
	
	// flush the queues to get things started 
    for (cl_uint i = 0; i < numDevices; i++)
    {
        status = clFlush(commandQueue[i]);
        CHECK_OPENCL_ERROR(status, "clFlush failed.");
    }

    // wait for the kernel call to finish execution 
    for (cl_uint i = 0; i < numDevices; i++)
    {
        status = sampleCommon->waitForEventAndRelease(&events[numDevices-i-1]);
        CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(events[numDevices - i - 1]) Failed");
    }

    if (timing && bench)
    {
        cl_ulong start;
        cl_ulong stop;
        status = clGetEventProfilingInfo(events[0],
                                         CL_PROFILING_COMMAND_SUBMIT,
                                         sizeof(cl_ulong),
                                         &start,
                                         NULL);
        CHECK_OPENCL_ERROR(status, "clGetEventProfilingInfo failed.");

        status = clGetEventProfilingInfo(events[0],
                                         CL_PROFILING_COMMAND_END,
                                         sizeof(cl_ulong),
                                         &stop,
                                         NULL);
        CHECK_OPENCL_ERROR(status, "clGetEventProfilingInfo failed.");

        time = (cl_double)(stop - start)*(cl_double)(1e-09);
    }
    for (cl_uint i = 0; i < numDevices; i++)
    {
        // Enqueue readBuffer
        status = clEnqueueReadBuffer(
                    commandQueue[i],
                    outputBuffer[i],
                    CL_FALSE,
                    0,
                    (width * height * sizeof(cl_int)) / numDevices,
                    output + (width * height / numDevices) * i,
                    0,
                    NULL,
                    &events[i]);
        CHECK_OPENCL_ERROR(status, "clEnqueueReadBuffer failed.");
    }

    for (cl_uint i = 0; i < numDevices; i++)
    {
        status = clFlush(commandQueue[i]);
        CHECK_OPENCL_ERROR(status, "clFlush failed.");
    }

    // wait for the kernel call to finish execution 
    for (cl_uint i = 0; i < numDevices; i++)
    {
        status = sampleCommon->waitForEventAndRelease(&events[numDevices - i - 1]);
        CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(events[numDevices - i - 1]) Failed"); 
    }

	// FINISH -- HERE
	__int64 Finish64 = Get_PC_Count() ;
	m_dRtime = GetTimeSpan( Start64, Finish64 );
	static int ct_cnt = 1 ;
	if(  --ct_cnt <= 0  )  {  ct_cnt = 1000 ;
		printf("Calc Time = %s Seconds \n", ConvertTimeSpan( m_dRtime ).c_str() );
	}

    if (timing && bench)
    {
        cl_ulong totalIterations = 0;
        for (int i = 0; i < (width * height); i++)
        {
            totalIterations += output[i];
        }
        cl_double flops = 7.0*totalIterations;
        printf("%lf MFLOPs\n", flops*(double)(1e-6)/time);
        printf("%lf MFLOPs according to CPU\n", flops*(double)(1e-6)/totalKernelTime);
        bench = 0;
        benched = 1;
    }
    return SDK_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////


int
Mandelbrot::setupCL(void)
{
    // FGZ comment out original 

	cl_int status = 0;
    std::size_t deviceListSize;
	/*  fgz 
    if(deviceType.compare("cpu") == 0)
    {
        dType = CL_DEVICE_TYPE_CPU;
    }
    else //deviceType = "gpu" 
		*/
    {
        dType = CL_DEVICE_TYPE_GPU;
        if(isThereGPU() == false)
        {
            std::cout << "GPU not found. Falling back to CPU device" << std::endl;
			printf("GPU not found. Falling back to CPU device\n");
            dType = CL_DEVICE_TYPE_CPU;
        }
		else
			printf("Found The GPU to be acceptable - Calc with GPU !!!\n");
    }
    
    /*
     * Have a look at the available platforms and pick either
     * the AMD one if available or a reasonable default.
     */
    cl_platform_id platform = NULL;
    int retValue = sampleCommon->getPlatform(platform, platformId, isPlatformEnabled());
    CHECK_ERROR(retValue, retValue ^ SDK_SUCCESS, "sampleCommon::getPlatform() failed");

    // Display available devices.
    retValue = sampleCommon->displayDevices(platform, dType);
    CHECK_ERROR(retValue, SDK_SUCCESS, "sampleCommon::displayDevices() failed");
    
    
    /*
     * If we could find our platform, use it. Otherwise pass a NULL and get whatever the
     * implementation thinks we should be using.
     */

    cl_context_properties cps[3] = 
    {
        CL_CONTEXT_PLATFORM, 
        (cl_context_properties)platform, 
        0
    };
    // Use NULL for backward compatibility 
    cl_context_properties* cprops = (NULL == platform) ? NULL : cps;

    context = clCreateContextFromType(
                  cprops,
                  dType,
                  NULL,
                  NULL,
                  &status);
    CHECK_OPENCL_ERROR(status, "clCreateContextFromType failed.");

    // First, get the size of device list data 
    status = clGetContextInfo(
                 context, 
                 CL_CONTEXT_DEVICES, 
                 0, 
                 NULL, 
                 &deviceListSize);
    CHECK_OPENCL_ERROR(status, "clGetContextInfo failed.");

    int deviceCount = (int)(deviceListSize / sizeof(cl_device_id));

    status = sampleCommon->validateDeviceId(deviceId, deviceCount);
    CHECK_ERROR(status, SDK_SUCCESS, "sampleCommon::validateDeviceId() failed");

    (devices) = (cl_device_id *)malloc(deviceListSize);
    CHECK_ALLOCATION((devices), "Failed to allocate memory (devices).");

    // Now, get the device list data 
    status = clGetContextInfo(context, 
                 CL_CONTEXT_DEVICES, 
                 deviceListSize, 
                 (devices), 
                 NULL);
    CHECK_OPENCL_ERROR(status, "clGetGetContextInfo failed.");



    numDevices = (cl_uint)(deviceListSize/sizeof(cl_device_id));
    numDevices = min(MAX_DEVICES, numDevices);

    if(numDevices != 1 && isLoadBinaryEnabled())
    {
        sampleCommon->expectedError("--load option is not supported if devices are more one");
        return SDK_EXPECTED_FAILURE;
    }

    if(numDevices == 3)
    {
        if(!quiet)
        {
            cout << "Number of devices must be even,"
                 << "\nChanging number of devices from three to two\n";
        }
        numDevices = 2;
    }

	// Set numDevices to 1 if devicdeId option is used
    if(isDeviceIdEnabled())
    {
        numDevices = 1;
        devices[0] = devices[deviceId];
        deviceId = 0;
    }

    std::string flagsStr = std::string("");
    if(enableDouble)
    {
        // Check whether the device supports double-precision 
        int khrFP64 = 0;
        int amdFP64 = 0;
        for (cl_uint i = 0; i < numDevices; i++)
        {
            char deviceExtensions[8192];

            // Get device extensions 
            status = clGetDeviceInfo(devices[i], 
                CL_DEVICE_EXTENSIONS, 
                sizeof(deviceExtensions), 
                deviceExtensions, 
                0);
            CHECK_OPENCL_ERROR(status, "clGetDeviceInfo failed.(extensions)");

            // Check if cl_khr_fp64 extension is supported 
            if(strstr(deviceExtensions, "cl_khr_fp64"))
            {
                khrFP64++;
            }
            else
            {
                // Check if cl_amd_fp64 extension is supported 
                if(!strstr(deviceExtensions, "cl_amd_fp64"))
                {
                    OPENCL_EXPECTED_ERROR("Device does not support cl_amd_fp64 extension!");
                }
                else
                    amdFP64++;
            }
        }

        if(khrFP64 == numDevices)
            flagsStr.append("-D KHR_DP_EXTENSION ");
        else if(amdFP64 == numDevices)
            flagsStr.append("");
        else
        {
            sampleCommon->expectedError("All devices must have same extension(either cl_amd_fp64 or cl_khr_fp64)!");
            return SDK_EXPECTED_FAILURE;
        }
    }

    for (cl_uint i = 0; i < numDevices; i++)
    {
        // The block is to move the declaration of prop closer to its use 
        cl_command_queue_properties prop = 0;
        if(timing)
            prop |= CL_QUEUE_PROFILING_ENABLE;

        commandQueue[i] = clCreateCommandQueue(
                context, 
                devices[i], 
                prop, 
                &status);
        CHECK_OPENCL_ERROR(status, "clCreateCommandQueue failed.");

        outputBuffer[i] = clCreateBuffer(
                context, 
                CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
                (sizeof(cl_uint) * width * height) / numDevices,
                NULL,
                &status);
        CHECK_OPENCL_ERROR(status, "clCreateBuffer failed. (outputBuffer)");
		
    }
	 
    // create a CL program using the kernel source 
    streamsdk::SDKFile kernelFile;

    if(enableFMA)
        flagsStr.append("-D MUL_ADD=fma ");
    else
        flagsStr.append("-D MUL_ADD=mad ");

    if(enableDouble)
        flagsStr.append("-D ENABLE_DOUBLE ");
printf("GOT HERE point 1");
    std::string kernelPath = sampleCommon->getPath();
    if(isLoadBinaryEnabled())
    {
        kernelPath.append(loadBinary.c_str());

        if(!kernelFile.readBinaryFromFile(kernelPath.c_str()))
        {
            std::cout << "Failed to load kernel file : " << kernelPath << std::endl;
            return SDK_FAILURE;
        }

        const char * binary = kernelFile.source().c_str();
        std::size_t binarySize = kernelFile.source().size();
        program = clCreateProgramWithBinary(context,
                                            1,
                                            &devices[deviceId], 
                                            (const std::size_t *)&binarySize,
                                            (const unsigned char**)&binary,
                                            NULL,
                                            &status);
    }
    else
    {
        kernelPath.append("BlackMandel_Kernels.cl");
        if(!kernelFile.open(kernelPath.c_str()))
        {
            std::cout << "Failed to load kernel file : " << kernelPath << std::endl;
            return SDK_FAILURE;
        }

        const char * source = kernelFile.source().c_str();
        std::size_t sourceSize[] = { strlen(source) };
        // clBuildProgramWithSource  ??
		program = clCreateProgramWithSource(context,
                                            1,
                                            &source,
                                            sourceSize,
                                            &status);
    }
    CHECK_OPENCL_ERROR(status,"clCreateProgramWithSource failed.");

    // Get additional options

    if(isComplierFlagsSpecified())
    {
        streamsdk::SDKFile flagsFile;
        std::string flagsPath = sampleCommon->getPath();
        flagsPath.append(flags.c_str());
        if(!flagsFile.open(flagsPath.c_str()))
        {
            std::cout << "Failed to load flags file: " << flagsPath << std::endl;
            return SDK_FAILURE;
        }
        flagsFile.replaceNewlineWithSpaces();
        const char * flags = flagsFile.source().c_str();
        flagsStr.append(flags);
    }
	
	const char* ctmp = "-x clc" ;
	flagsStr.append(ctmp);  // fgz above with clc++ ... result: Error Undefined Function Index 1024 ...
    
	if(flagsStr.size() != 0)
        std::cout << "Build Options are : " << flagsStr.c_str() << std::endl;

    /* create a cl program executable for all the devices specified */
    status = clBuildProgram(program, 
                            numDevices, 
                            devices, 
                            flagsStr.c_str(), 
                            NULL, 
                            NULL);

    if(status != CL_SUCCESS)
    {
        if(status == CL_BUILD_PROGRAM_FAILURE)
        {
            for (cl_uint i = 0; i < numDevices; i++)
            {
                cl_int logStatus;
                char * buildLog = NULL;
                std::size_t buildLogSize = 0;
                logStatus = clGetProgramBuildInfo (program, 
                    devices[deviceId], 
                    CL_PROGRAM_BUILD_LOG, 
                    buildLogSize, 
                    buildLog, 
                    &buildLogSize);
                CHECK_OPENCL_ERROR(logStatus,"clGetProgramBuildInfo failed.");

                buildLog = (char*)malloc(buildLogSize);
                CHECK_ALLOCATION(buildLog, "Failed to allocate host memory.");

                memset(buildLog, 0, buildLogSize);

                logStatus = clGetProgramBuildInfo (program, 
                    devices[deviceId], 
                    CL_PROGRAM_BUILD_LOG, 
                    buildLogSize, 
                    buildLog, 
                    NULL);
                if(sampleCommon->checkVal(
                    logStatus,
                    CL_SUCCESS,
                    "clGetProgramBuildInfo failed."))
                {
                    free(buildLog);
                    return SDK_FAILURE;
                }

                std::cout << " \n\t\t\tBUILD LOG\n";
                std::cout << " ************************************************\n";
                std::cout << buildLog << std::endl;
                std::cout << " ************************************************\n";
                free(buildLog);
            }
        }
		printf("Got past build program ...");
        CHECK_OPENCL_ERROR(status,"clBuildProgram failed.");

    }
	
    for (cl_uint i = 0; i < numDevices; i++)
    {
        // get a kernel object handle for a kernel with the given name 
		if(enableDouble)
			kernel_vector[i] = clCreateKernel(program, "ztof_vector_Complexd2", &status);
	//		kernel_vector[i] = clCreateKernel(program, "mandelbrot_vector_double", &status);
        else
             kernel_vector[i] = clCreateKernel(program, "ztof_vector_Complexf4", &status);
         //    kernel_vector[i] = clCreateKernel(program, "mandelbrot_vector_float", &status);

        CHECK_OPENCL_ERROR(status, "clCreateKernel failed.");
    }
    return SDK_SUCCESS;
}


int 
Mandelbrot::runCLKernels(void)
{
    cl_int   status;
    cl_kernel kernel;
    cl_event events[MAX_DEVICES];
    cl_int eventStatus = CL_QUEUED;

    std::size_t globalThreads[1];
    std::size_t localThreads[1];

	// FGZ - moved these here, reason: scope and loss of variable memory before calling kernels ...		
	cl_float leftxF ; // = (float)leftx;
            cl_float topyF ; // = (float)topy;
            cl_float xstepF ; //= (float)xstep;
            cl_float ystepF ; //= (float)ystep;
            cl_float pow_Ff ; // = (float)pow_F ;
			cl_float logpowf = (float)log(pow_F) ;  // do this once, here

    benched = 0;
	globalThreads[0] = (width * height * int(2.0 * ((enableDouble)? 1 : 0.5)) ) / numDevices;    // FGZ -- NOTE: for Double x2 the * 2 is needed to complete the image !!
    localThreads[0]  = 64; // this seems to be maxed out at 256. crashes with 384.  // this is the # of local work-groups (max for ATI 7xxx)

    globalThreads[0] >>= 2;  // this is the total number of threads requested.

    for (cl_uint i = 0; i < numDevices; i++)
    {
        kernel = kernel_vector[i];

        // Check group size against kernelWorkGroupSize 
        status = clGetKernelWorkGroupInfo(kernel,
                                          devices[i],
                                          CL_KERNEL_WORK_GROUP_SIZE,
                                          sizeof(std::size_t),
                                          &kernelWorkGroupSize,
                                          0);
        CHECK_OPENCL_ERROR(status, "clGetKernelWorkGroupInfo failed.");

        if((cl_uint)(localThreads[0]) > kernelWorkGroupSize)
            localThreads[0] = kernelWorkGroupSize;
		// kernelWorkGroupSize = 64 ;  // read that workgroup size should be 64 : result 35ms down to 32ms

        double aspect = (double)width / (double)height;
        xstep = (xsize / (double)width);
        // Adjust for aspect ratio
        double ysize = xsize / aspect;
        ystep = (-(xsize / aspect) / height);
        leftx = (xpos - xsize / 2.0);
        topy = (ypos + ysize / 2.0 -((double)i * ysize) / (double)numDevices);
		 

        if(i == 0)
        {
            topy0 = topy;
        }
		 
        int np = 0 ;
		// Set appropriate arguments to the kernel 
        status = clSetKernelArg(
                        kernel, 
                        np++, 
                        sizeof(cl_mem),
                       (void *)&outputBuffer[i]);
        CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (outputBuffer)");
		

        if(enableDouble) 
        {
			status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_double),
                           (void *)&pow_F);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (pow_F)");

            status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_double),
                           (void *)&leftx);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (leftx)");

            status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_double),
                           (void *)&topy);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (topy)");

            status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_double),
                           (void *)&xstep);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (xstep)");

            status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_double),
                           (void *)&ystep);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (ystep)");
        }
        else
        {
            // fgz - float variables were defined here ... then pointers to those floats ...
			 leftxF = (float)leftx;
             topyF = (float)topy;
             xstepF = (float)xstep;
             ystepF = (float)ystep;
             pow_Ff = (float)pow_F ;
			 
			status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_float), 
                           (void *)&pow_Ff);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (pow_F)");
			/* 
			status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_float),
                           (void *)&logpowf);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (logpowf)");
			*/
			status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_float),
                           (void *)&leftxF);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (leftxF)");

            status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_float),
                           (void *)&topyF);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (topyF)");

            status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_float),
                           (void *)&xstepF);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (xstepF)");

            status = clSetKernelArg(
                            kernel, 
                            np++, 
                            sizeof(cl_float),
                           (void *)&ystepF);
            CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (ystepF)");
        }
        status = clSetKernelArg(
                        kernel, 
                        np++, 
                        sizeof(cl_uint), 
                        (void *)&maxIterations);
        CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (maxIterations)");

        // width - i.e number of elements in the array 
        status = clSetKernelArg(
                        kernel, 
                        np++, 
                        sizeof(cl_int), 
                        (void *)&width);
        CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (width)");

        // bench - flag to indicate benchmark mode 
        status = clSetKernelArg(
                        kernel, 
                        np++, 
                        sizeof(cl_int), 
                        (void *)&bench);
        CHECK_OPENCL_ERROR(status, "clSetKernelArg failed. (bench)");

    
		 

		/* 
         * Enqueue a kernel run call.
         */
        status = clEnqueueNDRangeKernel(
                commandQueue[i],
                kernel,
                1,
                NULL,
                globalThreads,
                localThreads,
                0,
                NULL,
                &events[i]);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");
    }
    

	//  START --HERE
	__int64 Start64 = Get_PC_Count() ;
	
	// flush the queues to get things started 
    for (cl_uint i = 0; i < numDevices; i++)
    {
        status = clFlush(commandQueue[i]);
        CHECK_OPENCL_ERROR(status, "clFlush failed.");
    }

    // wait for the kernel call to finish execution 
    for (cl_uint i = 0; i < numDevices; i++)
    {
        status = sampleCommon->waitForEventAndRelease(&events[numDevices-i-1]);
        CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(events[numDevices - i - 1]) Failed");
    }

    if (timing && bench)
    {
        cl_ulong start;
        cl_ulong stop;
        status = clGetEventProfilingInfo(events[0],
                                         CL_PROFILING_COMMAND_SUBMIT,
                                         sizeof(cl_ulong),
                                         &start,
                                         NULL);
        CHECK_OPENCL_ERROR(status, "clGetEventProfilingInfo failed.");

        status = clGetEventProfilingInfo(events[0],
                                         CL_PROFILING_COMMAND_END,
                                         sizeof(cl_ulong),
                                         &stop,
                                         NULL);
        CHECK_OPENCL_ERROR(status, "clGetEventProfilingInfo failed.");

        time = (cl_double)(stop - start)*(cl_double)(1e-09);
    }
    for (cl_uint i = 0; i < numDevices; i++)
    {
        // Enqueue readBuffer
        status = clEnqueueReadBuffer(
                    commandQueue[i],
                    outputBuffer[i],
                    CL_FALSE,
                    0,
                    (width * height * sizeof(cl_int)) / numDevices,
                    output + (width * height / numDevices) * i,
                    0,
                    NULL,
                    &events[i]);
        CHECK_OPENCL_ERROR(status, "clEnqueueReadBuffer failed.");
    }

    for (cl_uint i = 0; i < numDevices; i++)
    {
        status = clFlush(commandQueue[i]);
        CHECK_OPENCL_ERROR(status, "clFlush failed.");
    }

    // wait for the kernel call to finish execution 
    for (cl_uint i = 0; i < numDevices; i++)
    {
        status = sampleCommon->waitForEventAndRelease(&events[numDevices - i - 1]);
        CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(events[numDevices - i - 1]) Failed"); 
    }

	// FINISH -- HERE
	__int64 Finish64 = Get_PC_Count() ;
	m_dRtime = GetTimeSpan( Start64, Finish64 );
	static int ct_cnt = 1 ;
	if(  --ct_cnt <= 0  )  {  ct_cnt = 1000 ;
		printf("Calc Time = %s Seconds \n", ConvertTimeSpan( m_dRtime ).c_str() );
	}

    if (timing && bench)
    {
        cl_ulong totalIterations = 0;
        for (int i = 0; i < (width * height); i++)
        {
            totalIterations += output[i];
        }
        cl_double flops = 7.0*totalIterations;
        printf("%lf MFLOPs\n", flops*(double)(1e-6)/time);
        printf("%lf MFLOPs according to CPU\n", flops*(double)(1e-6)/totalKernelTime);
        bench = 0;
        benched = 1;
    }
    return SDK_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
// FGZ - adding in timing functions here from VS C++ 2012 (2008) 


LONGLONG Mandelbrot::Get_PC_Count()
{
	LARGE_INTEGER pc_cnt;
	if( m_PC_Freq == 1000.0 || QueryPerformanceCounter( &pc_cnt ) == FALSE ) 
	{ 
		m_PC_Freq = 1000.0 ;
		return (LONGLONG)GetTickCount();
	}
	return pc_cnt.QuadPart ;
}

double Mandelbrot::GetTimeSpan(LONGLONG Start, LONGLONG Finish)
{
	return double(Finish - Start) / m_PC_Freq ;
}

std::string Mandelbrot::ConvertTimeSpan(double Time)
{
	std::string mys, myfs ;
	char cbuf[256];
	double pw10 ;
	int pw = max(0, int(log(Time)/log(10.0)) ) , fpw;
	 
	if( m_PC_Freq == 1000.0 ) 
	{
		myfs = "%.3f";
		pw10 = 1000.0;
	}
	else
	{
		fpw = max(4, int(log(m_PC_Freq)/log(10.0)) ) ;
		pw10 = pow(10.0, int(fpw - pw)) ;
		if( pw10 > 1.0 ) 
			sprintf(cbuf, "%%.%df", fpw - pw - ((Time < 1.0)? 0 : 1 ) ) ;
		myfs = cbuf ;
	}
	 
	if( pw10 > 1.0 )
		sprintf(cbuf, myfs.c_str(), double(Round64(Time * pw10))/pw10 );
	else
		sprintf(cbuf, "%I64u", __int64(double(Round64(Time * pw10))/pw10) );  // this has a slight error at/above 100 billion seconds (or about 3168 years)
	mys = cbuf ; 
	return mys;
}


// END FGZ ADD timing functions


int Mandelbrot::initialize()
{
    if( output != NULL ) cleanup() ;

	// Call base class Initialize to get default configuration
    if(this->SDKSample::initialize() != SDK_SUCCESS)
        return SDK_FAILURE;

    streamsdk::Option* image_width = new streamsdk::Option;
    CHECK_ALLOCATION(image_width, "Memory allocation error.\n");

    image_width->_sVersion = "W";
    image_width->_lVersion = "width";
    image_width->_description = "width of the mandelbrot image";
    image_width->_type = streamsdk::CA_ARG_INT;
    image_width->_value = &width;
    sampleArgs->AddOption(image_width);
    delete image_width;

    streamsdk::Option* image_height = new streamsdk::Option;
    CHECK_ALLOCATION(image_height, "Memory allocation error.\n");

    image_height->_sVersion = "H";
    image_height->_lVersion = "height";
    image_height->_description = "height of the mandelbrot image";
    image_height->_type = streamsdk::CA_ARG_INT;
    image_height->_value = &height;
    sampleArgs->AddOption(image_height);
    delete image_height;

    streamsdk::Option* xpos_param = new streamsdk::Option;
    CHECK_ALLOCATION(xpos_param, "Memory allocation error.\n");

    xpos_param->_sVersion = "x";
    xpos_param->_lVersion = "xpos";
    xpos_param->_description = "xpos to generate the mandelbrot fractal";
    xpos_param->_type = streamsdk::CA_ARG_STRING;
    xpos_param->_value = &xpos_str;
    sampleArgs->AddOption(xpos_param);
    delete xpos_param;

    streamsdk::Option* ypos_param = new streamsdk::Option;
    CHECK_ALLOCATION(ypos_param, "Memory allocation error.\n");

    ypos_param->_sVersion = "y";
    ypos_param->_lVersion = "ypos";
    ypos_param->_description = "ypos to generate the mandelbrot fractal";
    ypos_param->_type = streamsdk::CA_ARG_STRING;
    ypos_param->_value = &ypos_str;
    sampleArgs->AddOption(ypos_param);
    delete ypos_param;

    streamsdk::Option* xsize_param = new streamsdk::Option;
    CHECK_ALLOCATION(xsize_param, "Memory allocation error.\n");

    xsize_param->_sVersion = "xs";
    xsize_param->_lVersion = "xsize";
    xsize_param->_description = "Width of window for the mandelbrot fractal";
    xsize_param->_type = streamsdk::CA_ARG_STRING;
    xsize_param->_value = &xsize_str;
    sampleArgs->AddOption(xsize_param);
    delete xsize_param;

    streamsdk::Option* num_iterations = new streamsdk::Option;
    CHECK_ALLOCATION(num_iterations, "Memory allocation error.\n");

    num_iterations->_sVersion = "i";
    num_iterations->_lVersion = "iterations";
    num_iterations->_description = "Number of iterations for kernel execution";
    num_iterations->_type = streamsdk::CA_ARG_INT;
    num_iterations->_value = &iterations;
    sampleArgs->AddOption(num_iterations);
    delete num_iterations;


    streamsdk::Option* num_double = new streamsdk::Option;
    CHECK_ALLOCATION(num_double, "Memory allocation error.\n");

    num_double->_lVersion = "double";
    num_double->_description = "Enable double data type.(Default : float)";
    num_double->_type = streamsdk::CA_NO_ARGUMENT;
    num_double->_value = &enableDouble;
    sampleArgs->AddOption(num_double);
    delete num_double;

    streamsdk::Option* num_FMA = new streamsdk::Option;
    CHECK_ALLOCATION(num_FMA, "Memory allocation error.\n");

    num_FMA->_lVersion = "fma";
    num_FMA->_description = "Enable Fused Multiply-Add(FMA).(Default : Multiply-Add)";
    num_FMA->_type = streamsdk::CA_NO_ARGUMENT;
    num_FMA->_value = &enableFMA;
    sampleArgs->AddOption(num_FMA);
    delete num_FMA;


    if (xpos_str != "")
    {
        xpos = atof(xpos_str.c_str());
    }
    if (ypos_str != "")
    {
        ypos = atof(ypos_str.c_str());
    }
    if (xsize_str != "")
    {
        xsize = atof(xsize_str.c_str());
    }
    else
    {
        xsize = 4.0;
    }
    return SDK_SUCCESS;
}

int Mandelbrot::setup()
{
    // Make sure width is a multiple of 4
    width = (width + 3) & ~(4 - 1);

    iterations = 1;

    if(setupMandelbrot()!=SDK_SUCCESS)
        return SDK_FAILURE;

    int timer = sampleCommon->createTimer();
    sampleCommon->resetTimer(timer);
    sampleCommon->startTimer(timer);

    int returnVal = setupCL();
    if(returnVal != SDK_SUCCESS)
        return returnVal;

    sampleCommon->stopTimer(timer);

    setupTime = (cl_double)sampleCommon->readTimer(timer);

    return SDK_SUCCESS;
}


int Mandelbrot::run()
{
    int timer = sampleCommon->createTimer();
    sampleCommon->resetTimer(timer);
    sampleCommon->startTimer(timer);

    for(int i = 0; i < iterations; i++)
    {
        // Arguments are set and execution call is enqueued on command buffer 
        if(runCLKernels()!=SDK_SUCCESS)
            return SDK_FAILURE;
    }

    sampleCommon->stopTimer(timer);
    totalKernelTime = (double)(sampleCommon->readTimer(timer)) / iterations;

    return SDK_SUCCESS;
}

int Mandelbrot::verifyResults()
{  /*
    if(verify)
    {
        /* reference implementation
         * it overwrites the input array with the output
         */
   /*      if(enableDouble)
            mandelbrotRefDouble(
                verificationOutput, 
                leftx, 
                topy0, 
                xstep, 
                ystep, 
                maxIterations, 
                width, 
                bench);
        else
            mandelbrotRefFloat(
                verificationOutput, 
                (cl_float)leftx, 
                (cl_float)topy0, 
                (cl_float)xstep, 
                (cl_float)ystep, 
                maxIterations, 
                width, 
                bench);

        int i, j;
        int counter = 0;

        for(j = 0; j < height; j++)
        {
            for(i = 0; i < width; i++)
            {
                uchar4 temp_ver, temp_out;
                temp_ver.num = verificationOutput[j * width + i];
                temp_out.num = output[j * width + i];

                unsigned char threshold = 2;

                if( ((temp_ver.ch.s0 - temp_out.ch.s0) > threshold) ||
                    ((temp_out.ch.s0 - temp_ver.ch.s0) > threshold) ||

                    ((temp_ver.ch.s1 - temp_out.ch.s1) > threshold) ||
                    ((temp_out.ch.s1 - temp_ver.ch.s1) > threshold) ||

                    ((temp_ver.ch.s2 - temp_out.ch.s2) > threshold) ||
                    ((temp_out.ch.s2 - temp_ver.ch.s2) > threshold) ||

                    ((temp_ver.ch.s3 - temp_out.ch.s3) > threshold) ||
                    ((temp_out.ch.s3 - temp_ver.ch.s3) > threshold))
                {
                    counter++;
                }

            }
        }

        int numPixels = height * width;
        double ratio = (double)counter / numPixels;

        // compare the results and see if they match 

        if( ratio < 0.002)
        {
            std::cout<<"Passed!\n" << std::endl;
            return SDK_SUCCESS;
        }
        else
        {
            std::cout<<"Failed\n" << std::endl;
            return SDK_FAILURE;
        }

    }
	*/ 
    return SDK_SUCCESS;
}

void Mandelbrot::printStats()
{
    std::string strArray[4] = {"Width", "Height", "Time(sec)", "KernelTime(sec)"};
    std::string stats[4];

    totalTime = setupTime + totalKernelTime;

    stats[0] = sampleCommon->toString(width, std::dec);
    stats[1] = sampleCommon->toString(height, std::dec);
    stats[2] = sampleCommon->toString(totalTime, std::dec);
    stats[3] = sampleCommon->toString(totalKernelTime, std::dec);

    this->SDKSample::printStats(strArray, stats, 4);
}

int Mandelbrot::cleanup()
{
    // Releases OpenCL resources (Context, Memory etc.) 
    cl_int status;

    status = clReleaseProgram(program);
    CHECK_OPENCL_ERROR(status, "clReleaseProgram failed.(program)");

    for (cl_uint i = 0; i < numDevices; i++)
    {
        status = clReleaseKernel(kernel_vector[i]);
        CHECK_OPENCL_ERROR(status, "clReleaseKernel failed.");

        status = clReleaseMemObject(outputBuffer[i]);
        CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed.");

        status = clReleaseCommandQueue(commandQueue[i]);
        CHECK_OPENCL_ERROR(status, "clReleaseCommandQueue failed.");
    }

    status = clReleaseContext(context);
    CHECK_OPENCL_ERROR(status, "clReleaseContext failed.(context)");

    // release program resources (input memory etc.) 
    FREE(output);
    FREE(verificationOutput);
    FREE(devices);

    return SDK_SUCCESS;
}
 
cl_uint Mandelbrot::getWidth(void)
{
    return width;
}

cl_uint Mandelbrot::getHeight(void)
{
    return height;
}

void Mandelbrot::setWidth(UINT nWidth)
{
	// if( cleanup() != SDK_SUCCESS ) {  // doing clean-up here is bad ...
	//	cout << "clean up failed at setWidth" << endl ;
		// // return SDK_FAILURE ;
	// }
	width = max( 4, (min( 16384, nWidth ) + 3) & ~(4 - 1) ) ;    // Ensure multiple of 4 and min 4 ...
}

void Mandelbrot::setHeight(UINT nHeight)
{
	// if( cleanup() != SDK_SUCCESS ) {
	//	cout << "clean up failed at setHeight" << endl ;
		// // return SDK_FAILURE ;
	// }
	height = max( 4, min( 16384, nHeight ) ) ;
}


cl_uint Mandelbrot::setSize(UINT nWidth, UINT nHeight) 
{ 
	// if( cleanup() != SDK_SUCCESS ) {
	//	cout << "clean up failed at setSize" << endl ;
	//	return SDK_FAILURE ;
	// }

	width = max( 4, (min( 16384, nWidth ) + 3) & ~(4 - 1) ) ;    // Ensure multiple of 4 and min 4 ...
	height = max( 4, min( 16384, nHeight ) ) ;
	
	// if( setup() != SDK_SUCCESS ) {
	// 	cout << "setup failure at setSize" << endl ;
	// 	return SDK_FAILURE ;
	// }
	return SDK_SUCCESS ;
}


cl_uint * Mandelbrot::getPixels(void)
{
    return output;
}

cl_bool Mandelbrot::showWindow(void)
{
    return !quiet && !verify;
}
