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
#include <mitkToFCompositeFilter.h>

#include <mitkImage.h>
#include <mitkImageCast.h>
#include <itkBilateralImageFilter.h>
#include <mitkToFProcessingCommon.h>
#include <itkThresholdImageFilter.h>
#include <itkImageRegionIterator.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>
#include <mitkToFCompositeFilter.h>
#include <itkMedianImageFilter.h>
#include <mitkImagePixelReadAccessor.h>


/**Documentation
*  \brief test for the class "ToFCompositeFilter".
*
* Manually create filter pipeline and check if it is equivalent to composite filter
*/

typedef mitk::ToFProcessingCommon::ToFPoint2D ToFPoint2D;
typedef float ToFScalarType;
typedef itk::Image<ToFScalarType,2> ItkImageType_2D;
typedef itk::Image<ToFScalarType,3> ItkImageType_3D;
typedef itk::ImageRegionIterator<ItkImageType_2D> ItkImageRegionIteratorType2D;
typedef itk::ImageRegionIterator<ItkImageType_3D> ItkImageRegionIteratorType3D;
typedef itk::BilateralImageFilter<ItkImageType_2D,ItkImageType_2D> BilateralImageFilterType;
typedef itk::ThresholdImageFilter<ItkImageType_2D> ThresholdFilterType;
typedef itk::MedianImageFilter<ItkImageType_2D,ItkImageType_2D> MedianFilterType;


static bool ApplyTemporalMedianFilter(mitk::Image::Pointer& image, ItkImageType_2D::Pointer& itkImage2D)
{

  //initialize ITK output image
  unsigned int dimX = image->GetDimension(0);
  unsigned int dimY = image->GetDimension(1);
  unsigned int nbSlices = image->GetDimension(2);

  ItkImageType_2D::SizeType size;
  size[0] = dimX;
  size[1] = dimY;

  ItkImageType_2D::RegionType region;
  region.SetSize(size);

  ItkImageType_2D::SpacingType spacing;
  spacing[0] = 1.0;
  spacing[1] = 1.0;

  itkImage2D->SetRegions( region );
  itkImage2D->SetSpacing ( spacing );
  itkImage2D->Allocate();

  //initialize median filtering
  std::vector<ToFScalarType> allDistances;
  itk::Index<3> curIdx3D;
  ItkImageType_2D::IndexType curIdx2D;

  //compute median over time for each (x,y)
  for(unsigned int i = 0; i<dimX; i++)
  {
    for(unsigned int j = 0; j < dimY; j++)
    {
      allDistances.clear();

      curIdx3D[0] = i; curIdx3D[1] = j;
      curIdx2D[0] = i; curIdx2D[1] = j;
      //gather all distances for one pixel
      mitk::ImagePixelReadAccessor<ToFScalarType,3> imageAcces(image, image->GetVolumeData());
      for(unsigned int k = 0; k < nbSlices; k++)
      {
        curIdx3D[2] = k;
        allDistances.push_back(imageAcces.GetPixelByIndex(curIdx3D));
      }

      //sort distances and compute median
      std::sort(allDistances.begin(),allDistances.end());
      unsigned int median_idx = nbSlices/2;
      if(nbSlices%2 == 1) //i.e., there is an odd number of slices
      {
        itkImage2D->SetPixel(curIdx2D,allDistances[median_idx]);
      } else
      {
        ToFScalarType upper = allDistances[median_idx];
        ToFScalarType lower = allDistances[median_idx+1];
        itkImage2D->SetPixel(curIdx2D,(upper+lower)/2.0);
      }
    }
  }
  return true;

}

bool CreateRandomDistanceImage(unsigned int dimX, unsigned int dimY, ItkImageType_2D::Pointer& itkImage, mitk::Image::Pointer& mitkImage) //TODO warum ITK image?
{

  //initialize ITK output image
  ItkImageType_2D::IndexType start;
  start[0] = 0;
  start[1] = 0;
  ItkImageType_2D::SizeType size;
  size[0] = dimX;
  size[1] = dimY;
  ItkImageType_2D::RegionType region;
  region.SetSize(size);
  region.SetIndex( start);
  ItkImageType_2D::SpacingType spacing;
  spacing[0] = 1.0;
  spacing[1] = 1.0;

  itkImage->SetRegions( region );
  itkImage->SetSpacing ( spacing );
  itkImage->Allocate();


  ItkImageRegionIteratorType2D imageIterator(itkImage,itkImage->GetLargestPossibleRegion());
  imageIterator.GoToBegin();
  itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer randomGenerator = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();


  while (!imageIterator.IsAtEnd())
  {
    ToFScalarType pixelValue = randomGenerator->GetUniformVariate(0.0,1000.0);
    imageIterator.Set(pixelValue);
    ++imageIterator;
  }

  mitk::CastToMitkImage(itkImage,mitkImage);

  return true;
}


bool CreateRandomDistanceImageStack(unsigned int dimX, unsigned int dimY, unsigned int nbSlices, ItkImageType_3D::Pointer& itkImage, mitk::Image::Pointer& mitkImage)
{
  //initialize ITK output image
  ItkImageType_3D::IndexType start;
  start[0] = 0;
  start[1] = 0;
  start[1] = 0;
  ItkImageType_3D::SizeType size;
  size[0] = dimX;
  size[1] = dimY;
  size[2] = nbSlices;
  ItkImageType_3D::RegionType region;
  region.SetSize(size);
  region.SetIndex( start);
  ItkImageType_3D::SpacingType spacing;
  spacing[0] = 1.0;
  spacing[1] = 1.0;
  spacing[2] = 1.0;

  itkImage->SetRegions( region );
  itkImage->SetSpacing ( spacing );
  itkImage->Allocate();


  //assign random pixel values
  ItkImageRegionIteratorType3D imageIterator(itkImage,itkImage->GetLargestPossibleRegion());
  imageIterator.GoToBegin();
  itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer randomGenerator = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();


  while (!imageIterator.IsAtEnd())
  {
    ToFScalarType pixelValue = randomGenerator->GetUniformVariate(0.0,1000.0);
    imageIterator.Set(pixelValue);
    ++imageIterator;
  }

  //cast to MITK image
  mitk::CastToMitkImage(itkImage,mitkImage);

  return true;
}


int mitkToFCompositeFilterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCompositeFilter");

  //initialize composite filter
  mitk::ToFCompositeFilter::Pointer compositeFilter = mitk::ToFCompositeFilter::New();

  //Initialize threshold filter
  ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();
  int threshold_min = 5;
  int threshold_max = 100;
  thresholdFilter->SetOutsideValue(0.0);
  thresholdFilter->SetLower(threshold_min);
  thresholdFilter->SetUpper(threshold_max);
  compositeFilter->SetThresholdFilterParameter(threshold_min, threshold_max);

  //Initialize spatial median filter
  MedianFilterType::Pointer medianFilter = MedianFilterType::New();

  //Initialize bilateral filter
  BilateralImageFilterType::Pointer bilateralFilter = BilateralImageFilterType::New();
  float domainSigma = 4;
  float rangeSigma = 50;
  float kernelRadius = 3;
  bilateralFilter->SetDomainSigma(domainSigma);
  bilateralFilter->SetRangeSigma(rangeSigma);
  bilateralFilter->SetRadius(kernelRadius);
  compositeFilter->SetBilateralFilterParameter(domainSigma,rangeSigma,kernelRadius);

  //Initialize pipeline
  ItkImageType_2D::Pointer itkInputImage = ItkImageType_2D::New();
  mitk::Image::Pointer mitkInputImage = mitk::Image::New();
  CreateRandomDistanceImage(100,100,itkInputImage,mitkInputImage);
  ItkImageType_2D::Pointer itkOutputImage;
  compositeFilter->SetInput(mitkInputImage);
  mitk::Image::Pointer mitkOutputImage = compositeFilter->GetOutput();


  //-------------------------------------------------------------------------------------------------------

  //Apply first filter only (threshold)

  //standard variant
  thresholdFilter->SetInput(itkInputImage);
  itkOutputImage = thresholdFilter->GetOutput();
  itkOutputImage->Update();

  //variant with composite filter
  compositeFilter->SetApplyThresholdFilter(true);
  compositeFilter->SetApplyMedianFilter(false);
  compositeFilter->SetApplyTemporalMedianFilter(false);
  compositeFilter->SetApplyBilateralFilter(false);
  mitkOutputImage->Update();

  //compare output
  mitk::Image::Pointer itkOutputImageConverted;
  mitk::CastToMitkImage(itkOutputImage,itkOutputImageConverted);

  MITK_TEST_CONDITION_REQUIRED( mitk::Equal(itkOutputImageConverted,mitkOutputImage,mitk::eps,true),
                                "Test threshold filter in pipeline");

  //-------------------------------------------------------------------------------------------------------

  //Apply first and second filter

  //standard variant
  medianFilter->SetInput(thresholdFilter->GetOutput());
  itkOutputImage = medianFilter->GetOutput();
  itkOutputImage->Update();

  //variant with composite filter
  compositeFilter->SetApplyMedianFilter(true);
  mitkOutputImage->Update();

  //compare output
  mitk::CastToMitkImage(itkOutputImage,itkOutputImageConverted);

  MITK_TEST_CONDITION_REQUIRED( mitk::Equal(itkOutputImageConverted,mitkOutputImage,mitk::eps,true),
                               "Test threshold and median filter in pipeline");


  //-------------------------------------------------------------------------------------------------------

  //Apply first three filters

  //standard variant
  bilateralFilter->SetInput(medianFilter->GetOutput());
  itkOutputImage = bilateralFilter->GetOutput();
  itkOutputImage->Update();

  //variant with composite filter
  compositeFilter->SetApplyBilateralFilter(true);
  mitkOutputImage->Update();

  //compare output
  mitk::CastToMitkImage(itkOutputImage,itkOutputImageConverted);

  MITK_TEST_CONDITION_REQUIRED( mitk::Equal(itkOutputImageConverted,mitkOutputImage,mitk::eps,true),
                               "Test threshold filter, bilateral filter and temporal median filter in pipeline");


  //-------------------------------------------------------------------------------------------------------
  // TODO: Rewrite this. This don't make sense. the itk reference applies a median filter
  // and threshold filter afterwards. The composite filter does it in the other directtion.
  // also the input of random image stacks is never set into the composite filter

  //Apply all filters

  //generate image stack
//  ItkImageType_3D::Pointer itkInputImage3D = ItkImageType_3D::New();
//  mitk::Image::Pointer mitkImage3D = mitk::Image::New();
//  CreateRandomDistanceImageStack(100,100,12,itkInputImage3D,mitkImage3D);
//
//  //standard variant
//  ItkImageType_2D::Pointer medianFilteredImage = ItkImageType_2D::New();
//  ApplyTemporalMedianFilter(mitkImage3D,medianFilteredImage);
//  thresholdFilter->SetInput(medianFilteredImage);
//  itkOutputImage->Update();
//
//  //variant with composite filter
//  compositeFilter->SetApplyTemporalMedianFilter(true);
//  mitkOutputImage->Update();
//
//  //compare output
//  mitk::CastToMitkImage(itkOutputImage,itkOutputImageConverted);
//  pipelineSuccess = CompareImages(itkOutputImageConverted,mitkOutputImage);
//  MITK_TEST_CONDITION_REQUIRED(pipelineSuccess,"Test all filters in pipeline");


//-------------------------------------------------------------------------------------------------------

  //Check set/get functions
  mitk::Image::Pointer newImage = mitk::Image::New();
  mitk::Image::Pointer returnedImage;
  compositeFilter->SetInput(newImage);
  returnedImage = compositeFilter->GetInput();
  MITK_TEST_CONDITION_REQUIRED(newImage == returnedImage,"Get/Set empty image");

  compositeFilter->SetApplyTemporalMedianFilter(false);
  MITK_TEST_CONDITION_REQUIRED(compositeFilter->GetApplyTemporalMedianFilter()==false,"Get/Set ApplyTemporalMedianFilter");

  compositeFilter->SetApplyMedianFilter(false);
  MITK_TEST_CONDITION_REQUIRED(compositeFilter->GetApplyMedianFilter()==false,"Get/Set ApplyMedianFilter");

  compositeFilter->SetApplyThresholdFilter(false);
  MITK_TEST_CONDITION_REQUIRED(compositeFilter->GetApplyThresholdFilter()==false,"Get/Set ApplyThresholdFilter");

  compositeFilter->SetApplyBilateralFilter(false);
  MITK_TEST_CONDITION_REQUIRED(compositeFilter->GetApplyBilateralFilter()==false,"Get/Set ApplyBilateralFilter");

//-------------------------------------------------------------------------------------------------------

  MITK_TEST_END();

}
