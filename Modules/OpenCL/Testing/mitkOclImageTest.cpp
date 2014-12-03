/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <mitkTestingMacros.h>
#include <mitkImageGenerator.h>
#include <mitkOclImage.h>

using namespace mitk;

/**
  This function is testing the mitk::OclImage class.
  */
int mitkOclImageTest( int /*argc*/, char* /*argv*/[] )
{
  MITK_TEST_BEGIN("mitkOclImageTest");

  us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
  MITK_TEST_CONDITION_REQUIRED( ref != 0, "Got valid ServiceReference" );

  OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);
  MITK_TEST_CONDITION_REQUIRED( resources != NULL, "OpenCL Resource service available." );

  cl_context gpuContext = resources->GetContext();
  MITK_TEST_CONDITION_REQUIRED( gpuContext != NULL, "Got not-null OpenCL context.");

  cl_device_id gpuDevice = resources->GetCurrentDevice();
  MITK_TEST_CONDITION_REQUIRED( gpuDevice != NULL, "Got not-null OpenCL device.");

  //create a random image
  mitk::Image::Pointer reference;
  //check if 3D images are supported by the device and initialize image accordingly
  if( resources->GetMaximumImageSize(2, CL_MEM_OBJECT_IMAGE3D) == 0 ) //2D required
  {
    //Create a random reference image
    reference = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(119, 204, 0, 0, // dimension
                                                                         1.0f, 1.0f, 1.0f, // spacing
                                                                         255, 0); // max, min
  }
  else //3D ok
  {
    //Create a random reference image
    reference = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(119, 204, 52, 1, // dimension
                                                                         1.0f, 1.0f, 1.0f, // spacing
                                                                         255, 0); // max, min
  }
  MITK_TEST_CONDITION_REQUIRED( reference.IsNotNull(), "Reference mitk::Image object instantiated.");

  mitk::OclImage::Pointer oclTestImage = mitk::OclImage::New();
  oclTestImage->InitializeByMitkImage(reference);
  MITK_TEST_CONDITION_REQUIRED(oclTestImage.IsNotNull(), "oclImage object instantiated.");

  // test if oclImage correct initialized
  MITK_TEST_CONDITION( oclTestImage->GetMITKImage() == reference, "oclImage has the correct reference mitk::Image");
  MITK_TEST_CONDITION( oclTestImage->GetDimension() == reference->GetDimension(), "Same dimensionality.");

  cl_int clErr = 0;
  cl_command_queue cmdQueue = clCreateCommandQueue( gpuContext, gpuDevice,
                                                    0 ,&clErr);
  MITK_TEST_CONDITION_REQUIRED( clErr == CL_SUCCESS, "A command queue was created.");

  // Allocate and copy image data to GPU
  try
  {
    oclTestImage->TransferDataToGPU(cmdQueue);
    MITK_TEST_CONDITION( oclTestImage->IsModified(0), "Modified flag for GPU correctly set.");

    // check if the created GPU object is valid
    cl_mem gpuImage = oclTestImage->GetGPUImage(cmdQueue);
    MITK_TEST_CONDITION_REQUIRED( gpuImage != NULL, "oclImage returned a valid GPU memory pointer");
    size_t returned = 0;
    cl_image_format imgFmt;
    clErr = clGetImageInfo( gpuImage, CL_IMAGE_FORMAT, sizeof(cl_image_format),
                            (void*) &imgFmt, &returned );
    MITK_TEST_CONDITION( clErr == CL_SUCCESS, "oclImage has created a valid GPU image");

    // test for dimensions
    size_t imagesize = 0;
    clErr = clGetImageInfo( gpuImage, CL_IMAGE_WIDTH, sizeof(size_t),
                            (void*) &imagesize, &returned );
    MITK_TEST_CONDITION( imagesize == static_cast<size_t>(oclTestImage->GetDimension(0)), "Image width corresponds" );

    clErr = clGetImageInfo( gpuImage, CL_IMAGE_HEIGHT, sizeof(size_t),
                            (void*) &imagesize, &returned );
    MITK_TEST_CONDITION( imagesize == static_cast<size_t>(oclTestImage->GetDimension(1)), "Image height corresponds" );

    clErr = clGetImageInfo( gpuImage, CL_IMAGE_DEPTH, sizeof(size_t),
                            (void*) &imagesize, &returned );
    MITK_TEST_CONDITION( imagesize == static_cast<size_t>(oclTestImage->GetDimension(2)), "Image depth corresponds" );

    // clean up
    clReleaseCommandQueue( cmdQueue );
  }
  catch(mitk::ImageTypeIsNotSupportedByGPU& e)
  {
    MITK_ERROR << "Caught exception: " << e.what();
    //Image type is not supported -> skip test.
  }

  MITK_TEST_END();
}
