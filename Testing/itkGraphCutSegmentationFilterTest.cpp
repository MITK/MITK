/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <iostream>
#include <itkImage.h>
#include <itkIndex.h>
#include <itkImageRegionIterator.h>

#include <itkGraphCutSegmentationGridGraphFilter.h>
#include <itkGraphCutSegmentationBoykovFilter.h>


int itkGraphCutSegmentationFilterTest(int, char* [] )
//int main(int, char* [] )
{
  //typedef short PixelType;
  typedef itk::Index<2> IndexType2D;
  typedef itk::Index<3> IndexType;

  int testValue2D = 0;
  int referenceValue2D = 400;
  int testValue = 0;
  int referenceValue = 8000;

  typedef itk::Image< short, 2 > ImageType2D;
  typedef itk::Image< short, 3 > ImageType;
  typedef itk::ImageRegionIterator<ImageType2D>IteratorType2D;
  typedef itk::ImageRegionIterator<ImageType>IteratorType;

  typedef itk::GraphCutSegmentationGridGraphFilter<ImageType2D, ImageType2D> GraphCutSegmentationGridGraphFilterType2D;
  typedef itk::GraphCutSegmentationBoykovFilter<ImageType2D, ImageType2D> GraphCutSegmentationBoykovFilterType2D;
  typedef itk::GraphCutSegmentationFilter<ImageType2D, ImageType2D> GraphCutSegmentationFilterType2D;
  GraphCutSegmentationFilterType2D::Pointer graphcutFilter2D;
 
  typedef itk::GraphCutSegmentationGridGraphFilter<ImageType, ImageType> GraphCutSegmentationGridGraphFilterType;
  typedef itk::GraphCutSegmentationBoykovFilter<ImageType, ImageType> GraphCutSegmentationBoykovFilterType;
  typedef itk::GraphCutSegmentationFilter<ImageType, ImageType> GraphCutSegmentationFilterType;
  GraphCutSegmentationFilterType::Pointer graphcutFilter;

  //{ start creating a 2D test image
  ImageType2D::Pointer testImage2D = ImageType2D::New();
  ImageType2D::IndexType imageStart2D;
  imageStart2D[0] = 0;
  imageStart2D[1] = 0;

  ImageType2D::SizeType imageSize2D;
  imageSize2D[0] = 60;
  imageSize2D[1] = 60;

  ImageType2D::RegionType imageRegion2D;
  imageRegion2D.SetSize( imageSize2D );
  imageRegion2D.SetIndex( imageStart2D );
  testImage2D->SetRegions(imageRegion2D);
  testImage2D->Allocate();

  IteratorType2D blackIterator2D(testImage2D, imageRegion2D);
  for (blackIterator2D.GoToBegin(); !blackIterator2D.IsAtEnd(); ++blackIterator2D){
    blackIterator2D.Set(0);
  }

  ImageType2D::IndexType objectStart2D;
  objectStart2D[0] = 20;
  objectStart2D[1] = 20;

  ImageType2D::SizeType objectSize2D;
  objectSize2D[0] = 20;
  objectSize2D[1] = 20;

  ImageType2D::RegionType objectRegion2D;
  objectRegion2D.SetSize( objectSize2D );
  objectRegion2D.SetIndex( objectStart2D );

  IteratorType2D objectIterator2D(testImage2D, objectRegion2D);
  for (objectIterator2D.GoToBegin(); !objectIterator2D.IsAtEnd(); ++objectIterator2D){
    objectIterator2D.Set(200);
  }
  //} end creating a 2D test image

  //{ start creating a 2D seeds image
  ImageType2D::Pointer seedsImage2D = ImageType2D::New();
  imageRegion2D.SetSize( imageSize2D );
  imageRegion2D.SetIndex( imageStart2D );
  seedsImage2D->SetRegions(imageRegion2D);
  seedsImage2D->Allocate();

  IteratorType2D blackIterator2_2D(seedsImage2D, imageRegion2D);
  for (blackIterator2_2D.GoToBegin(); !blackIterator2_2D.IsAtEnd(); ++blackIterator2_2D){
    blackIterator2_2D.Set(0);
  }

  ImageType2D::IndexType objectSeedsStart2D;
  objectSeedsStart2D[0] = 25;
  objectSeedsStart2D[1] = 25;

  ImageType2D::SizeType objectSeedsSize2D;
  objectSeedsSize2D[0] = 10;
  objectSeedsSize2D[1] = 10;

  ImageType2D::RegionType objectSeedsRegion2D;
  objectSeedsRegion2D.SetSize( objectSeedsSize2D );
  objectSeedsRegion2D.SetIndex( objectSeedsStart2D );

  IteratorType2D objectSeedsIterator2D(seedsImage2D, objectSeedsRegion2D);
  for (objectSeedsIterator2D.GoToBegin(); !objectSeedsIterator2D.IsAtEnd(); ++objectSeedsIterator2D){
    objectSeedsIterator2D.Set(255);
  }

  ImageType2D::IndexType backgroundSeedsStart2D;
  backgroundSeedsStart2D[0] = 5;
  backgroundSeedsStart2D[1] = 5;

  ImageType2D::SizeType backgroundSeedsSize2D;
  backgroundSeedsSize2D[0] = 10;
  backgroundSeedsSize2D[1] = 10;

  ImageType2D::RegionType backgroundSeedsRegion2D;
  backgroundSeedsRegion2D.SetSize( backgroundSeedsSize2D );
  backgroundSeedsRegion2D.SetIndex( backgroundSeedsStart2D );

  IteratorType2D backgroundSeedsIterator2D(seedsImage2D, backgroundSeedsRegion2D);
  for (backgroundSeedsIterator2D.GoToBegin(); !backgroundSeedsIterator2D.IsAtEnd(); ++backgroundSeedsIterator2D){
    backgroundSeedsIterator2D.Set(254);
  }
  //} end creating a 2D seeds image

  //{ start creating a 3D test image
  ImageType::Pointer testImage = ImageType::New();
  ImageType::IndexType imageStart;
  imageStart[0] = 0;
  imageStart[1] = 0;
  imageStart[2] = 0;

  ImageType::SizeType imageSize;
  imageSize[0] = 60;
  imageSize[1] = 60;
  imageSize[2] = 60;

  ImageType::RegionType imageRegion;
  imageRegion.SetSize( imageSize );
  imageRegion.SetIndex( imageStart );
  testImage->SetRegions(imageRegion);
  testImage->Allocate();

  IteratorType blackIterator(testImage, imageRegion);
  for (blackIterator.GoToBegin(); !blackIterator.IsAtEnd(); ++blackIterator){
    blackIterator.Set(0);
  }

  ImageType::IndexType objectStart;
  objectStart[0] = 20;
  objectStart[1] = 20;
  objectStart[2] = 20;

  ImageType::SizeType objectSize;
  objectSize[0] = 20;
  objectSize[1] = 20;
  objectSize[2] = 20;

  ImageType::RegionType objectRegion;
  objectRegion.SetSize( objectSize );
  objectRegion.SetIndex( objectStart );

  IteratorType objectIterator(testImage, objectRegion);
  for (objectIterator.GoToBegin(); !objectIterator.IsAtEnd(); ++objectIterator){
    objectIterator.Set(200);
  }
  //} end creating a 3D test image

  //{ start creating a 3D seeds image
  ImageType::Pointer seedsImage = ImageType::New();
  imageRegion.SetSize( imageSize );
  imageRegion.SetIndex( imageStart );
  seedsImage->SetRegions(imageRegion);
  seedsImage->Allocate();

  IteratorType blackIterator2(seedsImage, imageRegion);
  for (blackIterator2.GoToBegin(); !blackIterator2.IsAtEnd(); ++blackIterator2){
    blackIterator2.Set(0);
  }

  ImageType::IndexType objectSeedsStart;
  objectSeedsStart[0] = 25;
  objectSeedsStart[1] = 25;
  objectSeedsStart[2] = 25;

  ImageType::SizeType objectSeedsSize;
  objectSeedsSize[0] = 10;
  objectSeedsSize[1] = 10;
  objectSeedsSize[2] = 10;

  ImageType::RegionType objectSeedsRegion;
  objectSeedsRegion.SetSize( objectSeedsSize );
  objectSeedsRegion.SetIndex( objectSeedsStart );

  IteratorType objectSeedsIterator(seedsImage, objectSeedsRegion);
  for (objectSeedsIterator.GoToBegin(); !objectSeedsIterator.IsAtEnd(); ++objectSeedsIterator){
    objectSeedsIterator.Set(255);
  }

  ImageType::IndexType backgroundSeedsStart;
  backgroundSeedsStart[0] = 5;
  backgroundSeedsStart[1] = 5;
  backgroundSeedsStart[2] = 5;

  ImageType::SizeType backgroundSeedsSize;
  backgroundSeedsSize[0] = 10;
  backgroundSeedsSize[1] = 10;
  backgroundSeedsSize[2] = 10;

  ImageType::RegionType backgroundSeedsRegion;
  backgroundSeedsRegion.SetSize( backgroundSeedsSize );
  backgroundSeedsRegion.SetIndex( backgroundSeedsStart );

  IteratorType backgroundSeedsIterator(seedsImage, backgroundSeedsRegion);
  for (backgroundSeedsIterator.GoToBegin(); !backgroundSeedsIterator.IsAtEnd(); ++backgroundSeedsIterator){
    backgroundSeedsIterator.Set(254);
  }
  //} end creating a 3D seeds image

  //{ start testing itkGraphCutSegmentationGridGraphFilter with 2D image
  std::cout << "Testing itkGraphCutSegmentationGridGraphFilter with 2D image: " <<std::endl;

  graphcutFilter2D = GraphCutSegmentationGridGraphFilterType2D::New();
  graphcutFilter2D->SetInput(testImage2D);
  graphcutFilter2D->SetParameter_sigma_sqr(32);
  graphcutFilter2D->SetParameter_h(0);
  graphcutFilter2D->SetIntensityDifference(100);
  graphcutFilter2D->SetGradientMagnitude(90);
  graphcutFilter2D->SetParameter_MaxMagni(15);
  graphcutFilter2D->SetParameter_Exp(8);
  graphcutFilter2D->SetLaplacian(70);
  graphcutFilter2D->SetSeedsImage(seedsImage2D);
  graphcutFilter2D->Update();

  ImageType2D::Pointer resultImage2D;
  resultImage2D = graphcutFilter2D->GetOutput();

  IteratorType2D testIterator2D(resultImage2D, resultImage2D->GetRequestedRegion());
  for (testIterator2D.GoToBegin(); !testIterator2D.IsAtEnd(); ++testIterator2D){
    if(testIterator2D.Get() == 1)
      testValue2D = testValue2D + 1;
  }

  if(referenceValue2D != testValue2D){
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  //} end testing itkGraphCutSegmentationGridGraphFilter with 2D image


  //{ start testing itkGraphCutSegmentationGridGraphFilter with 3D image
  std::cout << "Testing itkGraphCutSegmentationGridGraphFilter with 3D image: " <<std::endl;

  graphcutFilter = GraphCutSegmentationGridGraphFilterType::New();
  graphcutFilter->SetInput(testImage);
  graphcutFilter->SetParameter_sigma_sqr(32);
  graphcutFilter->SetParameter_h(0);
  graphcutFilter->SetIntensityDifference(100);
  graphcutFilter->SetGradientMagnitude(90);
  graphcutFilter->SetParameter_MaxMagni(15);
  graphcutFilter->SetParameter_Exp(8);
  graphcutFilter->SetLaplacian(70);
  graphcutFilter->SetSeedsImage(seedsImage);
  graphcutFilter->Update();

  ImageType::Pointer resultImage;
  resultImage = graphcutFilter->GetOutput();

  IteratorType testIterator(resultImage, resultImage->GetRequestedRegion());
  for (testIterator.GoToBegin(); !testIterator.IsAtEnd(); ++testIterator){
    if(testIterator.Get() == 1)
      testValue = testValue + 1;
  }

  if(referenceValue != testValue){
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  //} end testing itkGraphCutSegmentationGridGraphFilter with 3D image


  //{ start testing itkGraphCutSegmentationBoykovFilter with 2D image
  std::cout << "Testing itkGraphCutSegmentationBoykovFilter with 2D image: " <<std::endl;

  testValue2D = 0;
  graphcutFilter2D = GraphCutSegmentationBoykovFilterType2D::New();
  graphcutFilter2D->SetInput(testImage2D);
  graphcutFilter2D->SetParameter_sigma_sqr(32);
  graphcutFilter2D->SetParameter_h(0);
  graphcutFilter2D->SetIntensityDifference(100);
  graphcutFilter2D->SetGradientMagnitude(90);
  graphcutFilter2D->SetParameter_MaxMagni(15);
  graphcutFilter2D->SetParameter_Exp(8);
  graphcutFilter2D->SetLaplacian(70);
  graphcutFilter2D->SetSeedsImage(seedsImage2D);
  graphcutFilter2D->Update();

  //ImageType2D::Pointer resultImage2D;
  resultImage2D = graphcutFilter2D->GetOutput();

  IteratorType2D testIteratorBoykov2D(resultImage2D, resultImage2D->GetRequestedRegion());
  for (testIteratorBoykov2D.GoToBegin(); !testIteratorBoykov2D.IsAtEnd(); ++testIteratorBoykov2D){
    if(testIteratorBoykov2D.Get() == 1)
      testValue2D = testValue2D + 1;
  }

  if(referenceValue2D != testValue2D){
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  //} end testing itkGraphCutSegmentationBoykovFilter with 2D image


  //{ start testing itkGraphCutSegmentationBoykovFilter with 3D image
  std::cout << "Testing itkGraphCutSegmentationBoykovFilter with 3D image: " <<std::endl;

  testValue = 0;
  graphcutFilter = GraphCutSegmentationBoykovFilterType::New();
  graphcutFilter->SetInput(testImage);
  graphcutFilter->SetParameter_sigma_sqr(32);
  graphcutFilter->SetParameter_h(0);
  graphcutFilter->SetIntensityDifference(100);
  graphcutFilter->SetGradientMagnitude(90);
  graphcutFilter->SetParameter_MaxMagni(15);
  graphcutFilter->SetParameter_Exp(8);
  graphcutFilter->SetLaplacian(70);
  graphcutFilter->SetSeedsImage(seedsImage);
  graphcutFilter->Update();

  resultImage = graphcutFilter->GetOutput();

  IteratorType testIteratorBoykov(resultImage, resultImage->GetRequestedRegion());
  for (testIteratorBoykov.GoToBegin(); !testIteratorBoykov.IsAtEnd(); ++testIteratorBoykov){
    if(testIteratorBoykov.Get() == 1)
      testValue = testValue + 1;
  }

  if(referenceValue != testValue){
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  //} end testing itkGraphCutSegmentationBoykovFilter with 3D image

std::cout<<"[TEST DONE]"<<std::endl;
return EXIT_SUCCESS;
}
