Implementing Own OpenCL-based Image Filter  {#MitkOpenCL_BinaryThresholdFilter}
==========================================

This example demonstrates how to implement an image-to-image filter with some computations done on
an OpenCL device ( most likely a GPU ). This particular example presents a simple threshold filter
which uses an upper and lower threshold to map the pixel to the specified inside and outside value.

Since the filter we want to implement is an image-to-image filter, we will inherit from the
mitk::OclImageToImageFilter class.

In the constructor, we set the source file of the filter, in this case is the file located in the default
directory placed in the module itself. We also se the unique filter id string which serves as an identifier
for storing and getting the associated program through the OclResourceService.

\snippet mitkOcl-binarythrfilter/main.cpp Create

\snippet mitkOcl-binarythrfilter/main.cpp Initialize

For preparing the execution we call the InitExec method
provided by the superclass. We also catch exceptions singalising some initialization errors

\snippet mitkOcl-binarythrfilter/main.cpp ExecutePrepare

In this example, the header of the OpenCL-Kernel to be executed is the following

\snippet mitkOcl-binarythrfilter/gpucode.cl GPUHeader

The InitExec methods takes care of initializing the input and output images and their OpenCL counterparts.
In addition both parameters are passed to the filter and the input data is copied (if necessary).
If the first initialization part was successfull, we continue by passing in the values for the filter
specific parameters:

\snippet mitkOcl-binarythrfilter/main.cpp SetKernelArguments

If this passes, we tell the superclass to execute our compiled kernel. The second argument specifies the dimensionality
of the OpenCL NDRange ( the parallel execution grid ). In this example we use the 3D-Grid:

\snippet mitkOcl-binarythrfilter/main.cpp Execute

Through the last line we signalize that the OpenCL-side data of the output image is modified. This is essential for getting
the result data when calling GetOutput() afterwards.
