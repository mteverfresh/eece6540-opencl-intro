# Introduction to Intel OpenCL SDK for FPGAs

This repository contains the necessary files to get you started with Intel's FPGA OpenCL platform. The following source files are included:

1. **host** This directory contains the host code for the OpenCL application. Understanding the major parts of this is important, we will go over this in class.
2. **device** This directory contains the OpenCL kernel that the host program will execute. The kernel executes a vector addition on two input vectors and stores the result in an output vector.

Henceforth, the directory that contains this file will be referred to as the top-level directory. To run the code, do the following:

1. In the top-level directory, run `make`. This will build the host program.
2. Now we can build the device code for the emulator. This will typically be the first step in the debugging process once you have your host code written. Go to the `device` subdirectory and run the following command:

```
env=CL_CONTEXT_EMULATOR_DEVICE_ALTERA aoc vector_add.cl -o vector_add.aocx --board de5net_a7 -march=emulator -v
```

3. Copy the resulting device file from this directory into the top-level directory with:

```
cp ./vector_add.aocx ../
```

4. Run the program with `./host`. You should see no errors appear after the program executes, indicating that the vector add completed successfully!

Each section will be described by the instructors as they go through them, so if you have any questions do not hesitate to ask.
