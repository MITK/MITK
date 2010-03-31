/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-09-26 18:50:26 +0200 (Mi, 26 Sep 2007) $
Version:   $Revision: 9585 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "itkTotalVariationDenoisingImageFilter.h"
#include "itkTotalVariationSingleIterationImageFilter.h"
#include "itkLocalVariationImageFilter.h"
#include "itkImageRegionIterator.h"

// image typedefs
typedef itk::Image<float, 3> 
ImageType;
typedef itk::ImageRegionIterator<ImageType> 
IteratorType;

// vector image typedefs
typedef itk::Vector<float,2> 
VectorPixelType;
typedef itk::Image<VectorPixelType, 3>
VectorImageType;
typedef itk::ImageRegionIterator<VectorImageType> 
VectorIteratorType;

/**
* 3x3x3 test image
*/
ImageType::Pointer GenerateTestImage()
{
  // init
  ImageType::Pointer image = ImageType::New();;

  // spacing
  ImageType::SpacingType spacing;
  spacing[0] = 1;
  spacing[1] = 1;
  spacing[2] = 1;
  image->SetSpacing(spacing);

  // extent
  ImageType::RegionType largestPossibleRegion;
  ImageType::SizeType size = {{3,3,1}};
  largestPossibleRegion.SetSize( size );
  ImageType::IndexType index = {{0,0,0}};
  largestPossibleRegion.SetIndex( index );
  image->SetLargestPossibleRegion( largestPossibleRegion );
  image->SetBufferedRegion( largestPossibleRegion );

  // allocate memory
  image->Allocate();

  int i=0;
  IteratorType it(image, largestPossibleRegion);
  it.GoToBegin();
  while(!it.IsAtEnd())
  {
    it.Set((float)i++);
    ++it;
  }

  return image;
}

VectorImageType::Pointer GenerateVectorTestImage()
{
  // init
  VectorImageType::Pointer image = VectorImageType::New();;

  // spacing
  VectorImageType::SpacingType spacing;
  spacing[0] = 1;
  spacing[1] = 1;
  spacing[2] = 1;
  image->SetSpacing(spacing);

  // extent
  VectorImageType::RegionType largestPossibleRegion;
  VectorImageType::SizeType size = {{3,3,1}};
  largestPossibleRegion.SetSize( size );
  VectorImageType::IndexType index = {{0,0,0}};
  largestPossibleRegion.SetIndex( index );
  image->SetLargestPossibleRegion( largestPossibleRegion );
  image->SetBufferedRegion( largestPossibleRegion );

  // allocate memory
  image->Allocate();

  int i=0;
  VectorIteratorType it(image, largestPossibleRegion);
  it.GoToBegin();
  while(!it.IsAtEnd())
  {
    VectorPixelType vec;
    vec[0] = (float)i;
    vec[1] = (float)i++;
    it.Set(vec);
    ++it;
  }

  return image;
}

void PrintImage(ImageType::Pointer image)
{
  IteratorType it(image, image->GetLargestPossibleRegion());
  for(it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    std::cout << it.Get() << " ";
  }
  std::cout << std::endl;
}

void PrintVectorImage(VectorImageType::Pointer image)
{
  VectorIteratorType it(image, image->GetLargestPossibleRegion());
  for(it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    std::cout << it.Get() << " ";
  }
  std::cout << std::endl;
}

/**
* todo
*/
int itkTotalVariationDenoisingImageFilterTest(int /*argc*/, char* /*argv*/[])
{

  ImageType::Pointer image = GenerateTestImage();
  PrintImage(image);

  double precision = 0.01;
  ImageType::IndexType index = {{1,1,0}};
  VectorImageType::IndexType vecIndex = {{1,1,0}};

  try
  {
    typedef itk::LocalVariationImageFilter<ImageType,ImageType> 
      LocalFilterType;
    LocalFilterType::Pointer filter = LocalFilterType::New();
    filter->SetInput(image);
    filter->SetNumberOfThreads(1);
    filter->Update();
    ImageType::Pointer outImage = filter->GetOutput();

    PrintImage(outImage);
    if(fabs(outImage->GetPixel(index) - 4.472) > precision)
    {
      return EXIT_FAILURE;
    }
    
  }
  catch (...)
  {
    return EXIT_FAILURE;
  }

  try
  {
    typedef itk::TotalVariationSingleIterationImageFilter<ImageType,ImageType> 
      SingleFilterType;
    SingleFilterType::Pointer sFilter = SingleFilterType::New();
    sFilter->SetInput( image );
    sFilter->SetOriginalImage(GenerateTestImage());
    sFilter->SetLambda(0.5);
    sFilter->SetNumberOfThreads(1);
    sFilter->Update();  
    ImageType::Pointer outImageS = sFilter->GetOutput();
    
    PrintImage(outImageS);
    if(fabs(outImageS->GetPixel(index) - 4.0) > precision)
    {
      return EXIT_FAILURE;
    }

  }
  catch (...)
  {
    return EXIT_FAILURE;
  }

  try
  {
    typedef itk::TotalVariationDenoisingImageFilter<ImageType,ImageType> 
      TVFilterType;
    TVFilterType::Pointer tvFilter = TVFilterType::New();
    tvFilter->SetInput(image);
    tvFilter->SetNumberIterations(30);
    tvFilter->SetNumberOfThreads(1);
    tvFilter->SetLambda(0.1);
    tvFilter->Update();
    ImageType::Pointer outImageTV = tvFilter->GetOutput();
    
    PrintImage(outImageTV);
    if(fabs(outImageTV->GetPixel(index) - 4.0) > precision)
    {
      return EXIT_FAILURE;
    }

  }
  catch (...)
  {
    return EXIT_FAILURE;
  }

  VectorImageType::Pointer vecImage = GenerateVectorTestImage();
  PrintVectorImage(vecImage);

  try
  {
    typedef itk::LocalVariationImageFilter<VectorImageType,ImageType> 
      LocalVecFilterType;
    LocalVecFilterType::Pointer vecFilter = LocalVecFilterType::New();
    vecFilter->SetInput(vecImage);
    vecFilter->SetNumberOfThreads(1);
    vecFilter->Update();
    ImageType::Pointer outVecImage = vecFilter->GetOutput();
    
    PrintImage(outVecImage);
    if(fabs(outVecImage->GetPixel(index) - 6.324) > precision)
    {
      return EXIT_FAILURE;
    }

  }
  catch (...)
  {
    return EXIT_FAILURE;
  }

  try
  {
    typedef itk::TotalVariationSingleIterationImageFilter
      <VectorImageType,VectorImageType> 
      SingleVecFilterType;
    SingleVecFilterType::Pointer sVecFilter = SingleVecFilterType::New();
    sVecFilter->SetInput( vecImage );
    sVecFilter->SetOriginalImage(vecImage);
    sVecFilter->SetLambda(0.5);
    sVecFilter->SetNumberOfThreads(1);
    sVecFilter->UpdateLargestPossibleRegion();  
    VectorImageType::Pointer outVecImageS = sVecFilter->GetOutput();
    
    PrintVectorImage(outVecImageS);
    if(fabs(outVecImageS->GetPixel(vecIndex)[1] - 4.0) > precision)
    {
      return EXIT_FAILURE;
    }
  }
  catch (...)
  {
    return EXIT_FAILURE;
  }

  try
  {
    typedef itk::TotalVariationDenoisingImageFilter
      <VectorImageType,VectorImageType> TVVectorFilterType;
    TVVectorFilterType::Pointer tvVecFilter = TVVectorFilterType::New();
    tvVecFilter->SetInput(vecImage);
    tvVecFilter->SetNumberIterations(30);
    tvVecFilter->SetNumberOfThreads(1);
    tvVecFilter->SetLambda(0.1);
    tvVecFilter->Update();
    VectorImageType::Pointer outVecImageTV = tvVecFilter->GetOutput();
    
    PrintVectorImage(outVecImageTV);
    if(fabs(outVecImageTV->GetPixel(vecIndex)[1] - 4.0) > precision)
    {
      return EXIT_FAILURE;
    }
  }
  catch (...)
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

