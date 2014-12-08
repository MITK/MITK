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
#include <mitkOclBinaryThresholdImageFilter.h>

using namespace mitk;

/**
  This function is testing the OclFilter class and the
  OpenCL resource service. To prevent segmentation faults
  a mutexed reference counter is implemented in the resource service.
  It tracks the number of opencl program references for the corresponding filter
  and delete only the opencl programm if the reference count reaches 0.
  Every new instance of a filter increases the reference count by 1.
  This test runs successfull if the 2 filters are initialized, run
  and deleted without any crash.
  */
int mitkOclReferenceCountTest( int /*argc*/, char* /*argv*/[] )
{
  MITK_TEST_BEGIN("mitkOclReferenceCountTest");

  us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
  OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);
  resources->GetContext(); //todo why do i need to call this before GetMaximumImageSize()?
  if(resources->GetMaximumImageSize(2, CL_MEM_OBJECT_IMAGE3D) == 0)
  {
    //GPU device does not support 3D images. Skip this test.
    MITK_INFO << "Skipping test.";
    return 0;
  }

  mitk::Image::Pointer inputImage = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(119, 204, 52, 1, // dimension
                                                                                      1.0f, 1.0f, 1.0f, // spacing
                                                                                      255, 0); // max, min
  int upperThr = 255;
  int lowerThr = 60;
  int outsideVal = 0;
  int insideVal = 100;

  mitk::OclBinaryThresholdImageFilter::Pointer oclFilter1 = mitk::OclBinaryThresholdImageFilter::New();
  oclFilter1->SetInput( inputImage );
  oclFilter1->SetUpperThreshold( upperThr );
  oclFilter1->SetLowerThreshold( lowerThr );
  oclFilter1->SetOutsideValue( outsideVal );
  oclFilter1->SetInsideValue( insideVal );
  oclFilter1->Update();

  mitk::Image::Pointer outputImage1 = mitk::Image::New();
  outputImage1 = oclFilter1->GetOutput();

  mitk::OclBinaryThresholdImageFilter::Pointer oclFilter2 = mitk::OclBinaryThresholdImageFilter::New();

  oclFilter2->SetInput( inputImage );
  oclFilter2->SetUpperThreshold( upperThr );
  oclFilter2->SetLowerThreshold( lowerThr );
  oclFilter2->SetOutsideValue( outsideVal );
  oclFilter2->SetInsideValue( insideVal );
  oclFilter2->Update();

  mitk::Image::Pointer outputImage2 = mitk::Image::New();
  outputImage2 = oclFilter2->GetOutput();

  // delete filters
  oclFilter1 = NULL;
  oclFilter2 = NULL;

  // this is only visible if the delete did not cause a segmentation fault
  // it is always true and successfull if the program reaches this state
  MITK_TEST_CONDITION_REQUIRED( true, "2 Filters deleted without a crash -> success ");

  MITK_TEST_END();
}
