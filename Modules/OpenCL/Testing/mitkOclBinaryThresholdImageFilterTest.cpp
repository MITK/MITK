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
#include <mitkImageCast.h>

#include <mitkOclBinaryThresholdImageFilter.h>

// itk filter for reference computation
#include <itkBinaryThresholdImageFilter.h>
#include <itkSubtractImageFilter.h>
#include <itkStatisticsImageFilter.h>

using namespace mitk;

/**
  This function is testing the class mitk::OclContextManager.
  */
int mitkOclBinaryThresholdImageFilterTest( int /*argc*/, char**/* argv[]*/ )
{
  MITK_TEST_BEGIN("mitkOclBinaryThresholdImageFilterTest");

  ServiceReference ref = GetModuleContext()->GetServiceReference<OclResourceService>();
  MITK_TEST_CONDITION_REQUIRED( ref != 0, "Got valid ServiceReference" );

  OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);
  MITK_TEST_CONDITION_REQUIRED( resources != NULL, "OpenCL Resource service available." );

  cl_context gpuContext = resources->GetContext();
  MITK_TEST_CONDITION_REQUIRED( gpuContext != NULL, "Got not-null OpenCL context.");

  cl_device_id gpuDevice = resources->GetCurrentDevice();
  MITK_TEST_CONDITION_REQUIRED( gpuDevice != NULL, "Got not-null OpenCL device.");

  //Create a random reference image
  mitk::Image::Pointer inputImage = mitk::ImageGenerator::GenerateRandomImage<unsigned char>(119, 204, 52, 1, // dimension
                                                                                    1.0f, 1.0f, 1.0f, // spacing
                                                                                    255, 0); // max, min
  MITK_TEST_CONDITION_REQUIRED( inputImage.IsNotNull(), "Input (random) mitk::Image object instantiated.");

  // FIXME: could also be random values
  int upperThr = 255;
  int lowerThr = 60;

  int outsideVal = 0;
  int insideVal = 100;

  mitk::OclBinaryThresholdImageFilter* oclFilter = new mitk::OclBinaryThresholdImageFilter;
  MITK_TEST_CONDITION_REQUIRED( oclFilter != NULL, "Filter was created. ");

  oclFilter->SetInput( inputImage );
  oclFilter->SetUpperThreshold( upperThr );
  oclFilter->SetLowerThreshold( lowerThr );
  oclFilter->SetOutsideValue( outsideVal );
  oclFilter->SetInsideValue( insideVal );

  oclFilter->Update();

  mitk::Image::Pointer outputImage = mitk::Image::New();
  outputImage = oclFilter->GetOutput();

  MITK_TEST_CONDITION_REQUIRED( outputImage.IsNotNull(), "Filter returned an not-NULL image. ");

  // reference computation
  typedef itk::Image< unsigned char, 3> ImageType;
  typedef itk::BinaryThresholdImageFilter< ImageType, ImageType > ThresholdFilterType;

  ImageType::Pointer itkInputImage = ImageType::New();
  CastToItkImage( inputImage, itkInputImage );

  ThresholdFilterType::Pointer refThrFilter = ThresholdFilterType::New();
  refThrFilter->SetInput( itkInputImage );
  refThrFilter->SetLowerThreshold( lowerThr );
  refThrFilter->SetUpperThreshold( upperThr );
  refThrFilter->SetOutsideValue( outsideVal );
  refThrFilter->SetInsideValue( insideVal );

  typedef itk::SubtractImageFilter< ImageType, ImageType > SubtractFilterType;
  SubtractFilterType::Pointer subFilt = SubtractFilterType::New();

  ImageType::Pointer gpuReferenceImage = ImageType::New();
  CastToItkImage( oclFilter->GetOutput() ,gpuReferenceImage );

  subFilt->SetInput1( refThrFilter->GetOutput() );
  subFilt->SetInput2( gpuReferenceImage );

  typedef itk::StatisticsImageFilter< ImageType > StatFilterType;
  StatFilterType::Pointer stats = StatFilterType::New();
  stats->SetInput( subFilt->GetOutput() );
  stats->Update();

  MITK_TEST_CONDITION( stats->GetMaximum() == 0, "Maximal value in the difference image is 0.");
  MITK_TEST_CONDITION( stats->GetMinimum() == 0, "Minimal value in the difference image is 0.")

  MITK_TEST_END();
}
