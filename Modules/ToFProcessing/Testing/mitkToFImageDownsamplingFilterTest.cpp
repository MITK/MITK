
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

//mitk includes
#include <mitkTestingMacros.h>
#include <mitkPicFileReader.h>
#include <mitkPicFileWriter.h>
#include <mitkToFConfig.h>
#include "mitkToFImageDownsamplingFilter.h"

// creator class that provides pre-configured ToFCameraDevices



int mitkToFImageDownsamplingFilterTest(int argc , char* argv[])
{
  MITK_TEST_BEGIN("mitkToFImageDownSamplingFilterFilter");

  MITK_TEST_CONDITION_REQUIRED(argc>=1, "Missing Parameters");

  //Defining constants
  const int XDIM = 127;
  const int YDIM = 96;
  const int ZDIM = 19;

  // always start with this

  // create a new instance of filter and new image
  mitk::ToFImageDownsamplingFilter::Pointer testDownSampler = mitk::ToFImageDownsamplingFilter::New();

  // make sure new filter ins't null
  MITK_TEST_CONDITION_REQUIRED(testDownSampler.IsNotNull(), "Testing instantiation!");

  // Load ToF image
  MITK_INFO<<"Loading test image file: " << argv[1] << "\n"; // update with proper path and figure out how iti s passed from the test driver
  mitk::PicFileReader::Pointer reader = mitk::PicFileReader::New();

  std::string filename = MITK_TOF_DATA_DIR;
  filename.append("/");
  filename.append(argv[1]);
  reader->SetFileName(filename);
  reader->Update();
  mitk::Image::Pointer image = reader->GetOutput();

  MITK_TEST_CONDITION_REQUIRED(image.IsNotNull(), "Testing image reading");
  MITK_INFO << "Original image dimensions " << image->GetDimension (0)<<" " << image->GetDimension(1)<< " " << image->GetDimension(2) ;

  //call filter
  testDownSampler->SetInput(image);
  testDownSampler->SetResampledX(XDIM);
  testDownSampler->SetResampledY(YDIM);
  testDownSampler->SetResampledZ(ZDIM);

  if(image->GetDimension(0) >= XDIM && image->GetDimension(1)>=YDIM && image->GetDimension(2)>=ZDIM &&
    (image->GetDimension()==2 || image->GetDimension()==3))
  {
    testDownSampler->Update();
    mitk::Image::Pointer resultImage = testDownSampler->GetOutput();
    MITK_TEST_CONDITION_REQUIRED(resultImage->GetDimension(0) == XDIM && resultImage->GetDimension(1)==YDIM &&resultImage->GetDimension(2)==ZDIM, "Test result image dimensions with 3D image");
    MITK_INFO << "new image dimensions " << resultImage->GetDimension (0)<<" " << resultImage->GetDimension(1)<<" " << resultImage->GetDimension(2) ;
  }
  else
  {
    MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject);
    testDownSampler->Update();
    MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject);
  }



  // Mean for debugging purposes if you want to write the resutling image to a file
  //mitk::PicFileWriter::Pointer writer = mitk::PicFileWriter::New();
  //writer->SetInputImage( resultImage);
  //writer->SetFileName( "tofResult1.pic" );

  //writer->Update();


  // always end with this!
  MITK_TEST_END();
}

