/*

Filename: main.cpp
Author: Zach Sherer

*/

#include <stdio.h>
#include <iostream>
#include <cstring>
#include <CL/opencl.h>
#include "AOCLUtils/aocl_utils.h"
#include <stdlib.h>

using namespace aocl_utils;

#define MEM_SIZE 		262144
#define MAX_SOURCE_SIZE		(0x100000)

//OpenCL runtime structures
cl_platform_id 		platform = NULL;
cl_device_id 		device = NULL;
cl_context		context = NULL;
cl_program		program = NULL;
cl_command_queue	queue = NULL;
// Declare kernels here. //
// These should be of the type cl_kernel.
cl_kernel		kernel = NULL;

// Declare memory buffers here. //
// These should be of the type cl_mem.
cl_mem			a_buf = NULL;
cl_mem			b_buf = NULL;
cl_mem			out_buf = NULL;

//function prototypes
bool init_env(); 		//Sets up the OpenCL enviroment in this program.
void run_kernel();		//Setting of kernel args and launching of kernels occurs here.
void cleanup();			//Frees OpenCL constructs like kernels, buffers, etc.

// Declare memory constructs here. //
// These are the host-side constructs that you will load into your openCL buffers for processing.

unsigned		*a;
unsigned		*b;
unsigned 		*out;

//main: The high-level flow of the program.
int main()
{
	init_env();
	if(!init_env())
	{
		return -1;
	}

	//allocate the buffers
	a		= (unsigned*)alignedMalloc(sizeof(unsigned) * MEM_SIZE);
	b		= (unsigned*)alignedMalloc(sizeof(unsigned) * MEM_SIZE);
	out		= (unsigned*)alignedMalloc(sizeof(unsigned) * MEM_SIZE);

	//fill the buffers with predictable data, for testing purposes
	for(unsigned i = 0; i < MEM_SIZE; i++)
	{
		a[i] = i;
		b[i] = MEM_SIZE-i;
	}

	run_kernel();

	//check result
	for(unsigned i = 0; i < MEM_SIZE; i++)
	{
		if(out[i] != MEM_SIZE)
		{
			printf("Mismatch in results, check the kernel.\n");
			return -1;
		}
	}

	cleanup();

	return 0;
}
/*
init_env: The initialization step.

	This is where the OpenCL constructs get initialized or created. Most of this function will not change
	with each program, just the part at the end where the kernels and buffers are created.

	Things to note:

	Be sure to change the binary file name whenever you change it outside the program. 
	Similarly, changes to the kernel names will also require changes in the clCreateKernel function calls.

*/
bool init_env()
{
	cl_int status; //holds status of each operation for error checking

	//Get platform
	//This is an aocl_utils function
	platform = findPlatform("Intel(R) FPGA");
	if (platform == NULL)
	{
		printf("Unable to find FPGA OpenCL platform. Exiting.");
		return false;
	}

	//Get device ID. Since this is only running on the DE5NET for now, we only need to get one device.
	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
	checkError(status, "Failed to get devices");

	//Display platform and device names for a visual check of correctness. These are aocl_utils as well.
	printf("Platform: %s\n", getPlatformName(platform).c_str());
	printf("Using %s for calculation.\n", getDeviceName(device).c_str());

	//Create context
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &status);
	checkError(status, "Unable to create OpenCL context.");

	//Create Program
	std::string binary_file = getBoardBinaryFile("vector_add", device); 
	printf("Using binary %s to program FPGA\n", binary_file.c_str());
	program = createProgramFromBinary(context, binary_file.c_str(), &device, 1);

	//Build program
	status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
	checkError(status, "Failed to build program");

	//Create cmd queue
	queue = clCreateCommandQueue(context, device, 0, &status);
	checkError(status, "Failed to create queue");

	//Create kernels
	// Place your kernel creation function calls here.
	kernel = clCreateKernel(program, "vector_add", &status);
	checkError(status, "Failed to create kernel");

	//Create buffers
	// Place your buffer creation function calls here.
	// Be aware of the buffer size! This can affect results.
	a_buf = clCreateBuffer(context, CL_MEM_READ_WRITE, MEM_SIZE*sizeof(unsigned), NULL, &status);
	checkError(status, "Failed to create buffer a");
	b_buf = clCreateBuffer(context, CL_MEM_READ_WRITE, MEM_SIZE*sizeof(unsigned), NULL, &status);
	checkError(status, "Failed to create buffer b");
	out_buf = clCreateBuffer(context, CL_MEM_READ_WRITE, MEM_SIZE*sizeof(unsigned), NULL, &status);
	checkError(status, "Failed to create output buffer");

	return true;
}


/*
run_kernel: the actual execution component

	This is where the kernels will be launched and where data will be gathered from them.
	Here you will probably use the following functions:
	
	clEnqueueNDRangeKernel()
	clEnqueueWriteBuffer()
	clEnqueueReadBuffer()
	clSetKernelArg()
	clFinish()

	You will also declare your global and local work sizes here, which you will pass to the
	kernel with clEnqueueNDRangeKernel(). Also be sure to remember that if you are passing a 
	buffer as an argument do not pass the actual data, but rather the buffer itself 
	(the cl_mem variable).

	This function has been left empty because every program will have a different run_kernel 
	function.
*/
void run_kernel()
{
	char string[MEM_SIZE];

	cl_int status;

	clEnqueueWriteBuffer(queue, a_buf, CL_TRUE, 0, MEM_SIZE*sizeof(unsigned), a, NULL, NULL);
	clEnqueueWriteBuffer(queue, b_buf, CL_TRUE, 0, MEM_SIZE*sizeof(unsigned), b, NULL, NULL);
	clEnqueueWriteBuffer(queue, out_buf, CL_TRUE, 0, MEM_SIZE*sizeof(unsigned), out, NULL, NULL);

	//Set the kernel args
	status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&buffer);
	checkError(status, "Failed to set kernel arg 0");

	//Execute OpenCL Kernel
	status = clEnqueueTask(queue, kernel, 0, NULL, NULL);

	//Enqueue output buffer
	clEnqueueReadBuffer(queue, a_buf, CL_TRUE, 0, MEM_SIZE*sizeof(unsigned), a, 0, NULL, NULL);
	clEnqueueReadBuffer(queue, b_buf, CL_TRUE, 0, MEM_SIZE*sizeof(unsigned), b, 0, NULL, NULL);
	clEnqueueReadBuffer(queue, out_buf, CL_TRUE, 0, MEM_SIZE*sizeof(unsigned), out, 0, NULL, NULL);
}

//Required function for AOCL_utils
// The only functions left in here are things that you will definitely need to release, the rest
// is application dependent.
void cleanup()
{
	if(queue)
		clReleaseCommandQueue(queue);
	if(program)
		clReleaseProgram(program);
	if(context)
		clReleaseContext(context);
}
