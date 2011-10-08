/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-12 14:05:50 +0100 (Fr, 12 Mrz 2010) $
Version:   $Revision: 16010 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkTestingMacros.h>

#include <mitkImageDataItem.h>
#include <mitkPicFileReader.h>
#include <mitkToFImageRecorderFilter.h>
#include <mitkToFImageWriter.h>

#include <itkImage.h>
#include <itkImageRegionIterator.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

mitk::Image::Pointer CreateTestImage(unsigned int dimX, unsigned int dimY)
{
  typedef itk::Image<float,2> ItkImageType2D;
  typedef itk::ImageRegionIterator<ItkImageType2D> ItkImageRegionIteratorType2D;

  ItkImageType2D::Pointer image = ItkImageType2D::New();
  ItkImageType2D::IndexType start;
  start[0] = 0;
  start[1] = 0;
  ItkImageType2D::SizeType size;
  size[0] = dimX;
  size[1] = dimY;
  ItkImageType2D::RegionType region;
  region.SetSize(size);
  region.SetIndex( start);
  ItkImageType2D::SpacingType spacing;
  spacing[0] = 1.0;
  spacing[1] = 1.0;

  image->SetRegions( region );
  image->SetSpacing ( spacing );
  image->Allocate();

  //Obtaining image data from ToF camera//
  
  //Correlate inten values to PixelIndex//
  ItkImageRegionIteratorType2D imageIterator(image,image->GetLargestPossibleRegion());
  imageIterator.GoToBegin();
  itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer randomGenerator = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
  while (!imageIterator.IsAtEnd())
  {
    double pixelValue = randomGenerator->GetUniformVariate(0.0,1000.0);
    imageIterator.Set(pixelValue);
    ++imageIterator;
  }
  mitk::Image::Pointer mitkImage = mitk::Image::New();
  mitk::CastToMitkImage(image,mitkImage);
  return mitkImage;
}

static bool CompareImages(mitk::Image::Pointer image1, mitk::Image::Pointer image2)
{
  //check if epsilon is exceeded
  unsigned int sliceDimension = image1->GetDimension(0)*image1->GetDimension(1);
  bool picturesEqual = true;

  float* floatArray1 = (float*)image1->GetSliceData(0, 0, 0)->GetData();
  float* floatArray2 = (float*)image2->GetSliceData(0, 0, 0)->GetData();
  for(unsigned int i = 0; i < sliceDimension; i++)
  {
    if(!(mitk::Equal(floatArray1[i], floatArray2[i])))
    {
      picturesEqual = false;
    }
  }
  return picturesEqual;
}

/**Documentation
 *  test for the class "ToFImageRecorderFilter".
 */
int mitkToFImageRecorderFilterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFImageRecorder");
  mitk::ToFImageRecorderFilter::Pointer tofImageRecorderFilter = mitk::ToFImageRecorderFilter::New();

  MITK_TEST_OUTPUT(<< "Test SetFileName()");
  std::string testFileName = "test.pic";
  tofImageRecorderFilter->SetFileName(testFileName);
  mitk::ToFImageWriter::Pointer tofImageWriter = tofImageRecorderFilter->GetToFImageWriter();
  std::string requiredName = "test_DistanceImage.pic";
  std::string name = tofImageWriter->GetDistanceImageFileName();
  MITK_TEST_CONDITION_REQUIRED(requiredName==name,"Test for distance image file name");
  requiredName = "test_AmplitudeImage.pic";
  name = tofImageWriter->GetAmplitudeImageFileName();
  MITK_TEST_CONDITION_REQUIRED(requiredName==name,"Test for amplitude image file name");
  requiredName = "test_IntensityImage.pic";
  name = tofImageWriter->GetIntensityImageFileName();
  MITK_TEST_CONDITION_REQUIRED(requiredName==name,"Test for intensity image file name");

  mitk::Image::Pointer testDistanceImage = CreateTestImage(200,200);
  mitk::Image::Pointer testAmplitudeImage = CreateTestImage(200,200);
  mitk::Image::Pointer testIntensityImage = CreateTestImage(200,200);

  MITK_TEST_OUTPUT(<< "Apply filter");
  tofImageRecorderFilter->StartRecording();
  tofImageRecorderFilter->SetInput(0,testDistanceImage);
  tofImageRecorderFilter->SetInput(1,testAmplitudeImage);
  tofImageRecorderFilter->SetInput(2,testIntensityImage);
  tofImageRecorderFilter->Update();

  MITK_TEST_OUTPUT(<< "Test outputs of filter");
  mitk::Image::Pointer outputDistanceImage = tofImageRecorderFilter->GetOutput(0);
  MITK_TEST_CONDITION_REQUIRED(CompareImages(testDistanceImage,outputDistanceImage),"Test output 0 (distance image)");
  mitk::Image::Pointer outputAmplitudeImage = tofImageRecorderFilter->GetOutput(1);
  MITK_TEST_CONDITION_REQUIRED(CompareImages(testAmplitudeImage,outputAmplitudeImage),"Test output 1 (amplitude image)");
  mitk::Image::Pointer outputIntensityImage = tofImageRecorderFilter->GetOutput(2);
  MITK_TEST_CONDITION_REQUIRED(CompareImages(testIntensityImage,outputIntensityImage),"Test output 2 (intensity image)");
  tofImageRecorderFilter->StopRecording();

  MITK_TEST_OUTPUT(<< "Test content of written files");
  mitk::PicFileReader::Pointer imageReader = mitk::PicFileReader::New();
  imageReader->SetFileName("test_DistanceImage.pic");
  imageReader->Update();
  mitk::Image::Pointer loadedDistanceImage = imageReader->GetOutput();
  MITK_TEST_CONDITION_REQUIRED(CompareImages(testDistanceImage,loadedDistanceImage),"Test loaded image 0 (distance image)");
  imageReader->SetFileName("test_AmplitudeImage.pic");
  imageReader->Update();
  mitk::Image::Pointer loadedAmplitudeImage = imageReader->GetOutput();
  MITK_TEST_CONDITION_REQUIRED(CompareImages(testAmplitudeImage,loadedAmplitudeImage),"Test loaded image 1 (amplitude image)");
  imageReader->SetFileName("test_IntensityImage.pic");
  imageReader->Update();
  mitk::Image::Pointer loadedIntensityImage = imageReader->GetOutput();
  MITK_TEST_CONDITION_REQUIRED(CompareImages(testIntensityImage,loadedIntensityImage),"Test loaded image 2 (intensity image)");

  //clean up and delete saved image files
  if( remove( "test_DistanceImage.pic" ) != 0 )
  {
    MITK_ERROR<<"File: test_DistanceImage.pic not successfully deleted!";
  }
  if( remove( "test_AmplitudeImage.pic" ) != 0 )
  {
    MITK_ERROR<<"File: test_AmplitudeImage.pic not successfully deleted!";
  }
  if( remove( "test_IntensityImage.pic" ) != 0 )
  {
    MITK_ERROR<<"File: test_IntensityImage.pic not successfully deleted!";
  }
  MITK_TEST_END();
}


