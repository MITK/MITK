/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <iostream>
#include "mitkTestingMacros.h"

#include "itkImage.h"
#include "itkImageRegionIterator.h"

#include "itkBinaryFunctorImageFilter.h"

#include "mitkConvertToConcentrationFunctor.h"


int ConvertToConcentrationTest(int  argc , char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("ConvertToConcentration");

  //Initialization Parameters
  double RelaxationTime = 1.2;
  double RecoveryTime = 0.125;
  double Relaxivity = 4.3;



  //Test1: Functor calculation
  mitk::ConvertToConcentrationFunctor<double,double,double> testFunctor;
  testFunctor.initialize(RelaxationTime, Relaxivity, RecoveryTime);


  double s0 = 197.0;
  double sCM = 278.0;
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(0.009,testFunctor(sCM,s0), 1e-6, true)==true,"Check Functor Calculation with Input Value and baseline: 1");
   s0 = 157.0;
   sCM = 207.0;
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(0.0069,testFunctor(sCM,s0), 1e-6, true)==true,"Check Functor Calculation with Input Value and baseline: 2");
   s0 = 195.0;
   sCM = 270.0;
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(0.0084,testFunctor(sCM,s0), 1e-6, true)==true,"Check Functor Calculation with Input Value and baseline: 3");
   s0 = 177.0;
   sCM = 308.0;
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(0.0164,testFunctor(sCM,s0), 1e-6, true)==true,"Check Functor Calculation with Input Value and baseline: 4");

  //Test2: Filter usage with Functor

  typedef itk::Image<double,3> ImageType;
  typedef mitk::ConvertToConcentrationFunctor <double, double, double> ConversionFunctorType;
  typedef itk::BinaryFunctorImageFilter<ImageType, ImageType, ImageType, ConversionFunctorType> FilterType;

  //Definition of testimages
  ImageType::Pointer image = ImageType::New();
  ImageType::Pointer BaselineImage = ImageType::New();
  ImageType::IndexType start;
  start[0] =   0;  // first index on X
  start[1] =   0;  // first index on Y
  start[2] =   0;  // first index on Z
  ImageType::SizeType  size;
  size[0]  = 2;  // size along X
  size[1]  = 2;  // size along Y
  size[2]  = 2;  // size along Z
  ImageType::RegionType region;
  region.SetSize( size );
  region.SetIndex( start );
  image->SetRegions( region );
  image->Allocate();
  BaselineImage->SetRegions( region );
  BaselineImage->Allocate();
  itk::ImageRegionIterator<ImageType> it = itk::ImageRegionIterator<ImageType>(image,image->GetLargestPossibleRegion());
  itk::ImageRegionIterator<ImageType> Bit = itk::ImageRegionIterator<ImageType>(BaselineImage,BaselineImage->GetLargestPossibleRegion());
  int count = 0;
  while (!it.IsAtEnd()&& !Bit.IsAtEnd())
  {
    Bit.Set(150);
    it.Set(count*50+150);
    ++it;
    ++Bit;
    ++count;
  }
  //Filterinitialization
  FilterType::Pointer ConversionFilter = FilterType::New();
  ConversionFilter->SetFunctor(testFunctor);
  ConversionFilter->SetInput1(image);
  ConversionFilter->SetInput2(BaselineImage);

  ConversionFilter->Update();
  ImageType::Pointer convertedImage = ImageType::New();
  convertedImage = ConversionFilter->GetOutput();

  MITK_TEST_EQUAL(image->GetImageDimension(),convertedImage->GetImageDimension(),"Check dimensions of result image");

  itk::Index<3> idx;
  idx[0]=0;
  idx[1]=0;
  idx[2]=0;
  MITK_TEST_EQUAL(0,convertedImage->GetPixel(idx),"Check pixel of converted image index <0,0,0>");
  idx[0]=1;
  idx[1]=0;
  idx[2]=0;
  MITK_TEST_EQUAL(0.0072,convertedImage->GetPixel(idx),"Check pixel of converted image index <1,0,0>");
  idx[0]=0;
  idx[1]=1;
  idx[2]=0;
  MITK_TEST_EQUAL(0.0147,convertedImage->GetPixel(idx),"Check pixel of converted image index <0,1,0>");
  idx[0]=1;
  idx[1]=1;
  idx[2]=0;
  MITK_TEST_EQUAL(0.0225,convertedImage->GetPixel(idx),"Check pixel of converted image index <1,1,0>");
  idx[0]=0;
  idx[1]=0;
  idx[2]=1;
  MITK_TEST_EQUAL(0.0307 ,convertedImage->GetPixel(idx),"Check pixel of converted image index <0,0,1>");
  idx[0]=1;
  idx[1]=0;
  idx[2]=1;
  MITK_TEST_EQUAL(0.0392,convertedImage->GetPixel(idx),"Check pixel of converted image index <1,0,1>");
  idx[0]=0;
  idx[1]=1;
  idx[2]=1;
  MITK_TEST_EQUAL(0.0480,convertedImage->GetPixel(idx),"Check pixel of converted image index <0,1,1>");
  idx[0]=1;
  idx[1]=1;
  idx[2]=1;
  MITK_TEST_EQUAL(0.0574,convertedImage->GetPixel(idx),"Check pixel of converted image index <1,1,1>");



  MITK_TEST_END()
}
