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

#include "mitkStandardFileLocations.h"
#include "mitkTestingMacros.h"
#include "mitkPointSetStatisticsCalculator.h"
#include "itkImage.h"
#include "mitkExtendedLabelStatisticsImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"

#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIteratorWithIndex.h>
//#include <QtCore>

/**
 * \brief Test class for mitkPointSetStatisticsCalculator
 */
class mitkImageStatisticsTextureAnalysisTestClass
{
public:

  typedef itk::Image<unsigned short,3 >ImageType;
  typedef ImageType::Pointer pointerOfImage;

  typedef itk::ExtendedLabelStatisticsImageFilter< ImageType, ImageType > LabelStatisticsFilterType;
  typedef LabelStatisticsFilterType::Pointer labelStatisticsFilterPointer;
  //typedef itk::ImageRegionConstIteratorWithIndex<MaskImageType> MaskImageIteratorType;
  //typedef itk::ImageRegionConstIteratorWithIndex <ImageType> InputImageIndexIteratorType;

  static ImageType::Pointer CreatingTestLabelImage()
  {
    //InputImageIndexIteratorType labelIterator2;
    ImageType::Pointer image = ImageType :: New();
    ImageType::IndexType start;
    ImageType::SizeType size;

    start[0] = 0;
    start[1] = 0;
    start[2] = 0;

    size[0] = 2;
    size[1] = 2;
    size[2] = 2;

    ImageType:: RegionType region;
    region.SetSize( size );
    region.SetIndex( start );

    image->SetRegions(region);
    image->Allocate();
    image->FillBuffer(1.0);

    return image;
  }

  static ImageType::Pointer CreatingTestImage()
  {

    //InputImageIndexIteratorType labelIterator2;
    ImageType::Pointer image = ImageType :: New();
    ImageType::IndexType start;
    ImageType::SizeType size;

    start[0] = 0;
    start[1] = 0;
    start[2] = 0;

    size[0] = 2;
    size[1] = 2;
    size[2] = 2;

    ImageType:: RegionType region;
    region.SetSize( size );
    region.SetIndex( start );

    image->SetRegions(region);
    image->Allocate();
    image->FillBuffer(3.0);

    //for( labelIterator2.GoToBegin(); !labelIterator2.IsAtEnd(); ++labelIterator2)
    // {
    // }

    for(unsigned int r = 0; r < 1; r++)
    {
      for(unsigned int c = 0; c < 2; c++)
      {
        for(unsigned int l = 0; l < 2; l++)
        {
          ImageType::IndexType pixelIndex;
          pixelIndex[0] = r;
          pixelIndex[1] = c;
          pixelIndex[2] = l;

          image->SetPixel(pixelIndex, 2.0);
        }
      }
    }

    return image;
  }




  static LabelStatisticsFilterType::Pointer  TestInstantiation(ImageType::Pointer image, ImageType::Pointer maskImage)
  {
   // typedef itk::ExtendedLabelStatisticsImageFilter< ImageType, ImageType > LabelStatisticsFilterType;
    LabelStatisticsFilterType::Pointer labelStatisticsFilter;
    labelStatisticsFilter = LabelStatisticsFilterType::New();
    labelStatisticsFilter->SetInput( image );
    labelStatisticsFilter->SetLabelInput( maskImage );
    labelStatisticsFilter->SetLabelInput( maskImage );
    labelStatisticsFilter->Update();
    //typename StatisticsFilterType::Pointer statisticsFilter = StatisticsFilterType::New();
   // mitk::PointSetStatisticsCalculator::Pointer myPointSetStatisticsCalculator = mitk::PointSetStatisticsCalculator::New();
   // MITK_TEST_CONDITION_REQUIRED(myPointSetStatisticsCalculator.IsNotNull(),"Testing instantiation with constructor 1.");

    return labelStatisticsFilter;
  }

  static void TestTrueFalse(LabelStatisticsFilterType::Pointer labelStatisticsFilter, double expectedSkewness, double expectedKurtosis)
  {
    // let's create an object of our class
    MITK_TEST_CONDITION(labelStatisticsFilter->GetSkewness( 1 ) == expectedSkewness,"expectedSkewness: " << expectedSkewness << " actual Value: " << labelStatisticsFilter->GetSkewness( 1 ) );
    MITK_TEST_CONDITION(labelStatisticsFilter->GetKurtosis( 1 ) == expectedKurtosis,"expectedKurtosis: " << expectedKurtosis << " actual Value: " << labelStatisticsFilter->GetKurtosis( 1 ) );
  }
};

int mitkImageStatisticsTextureAnalysisTest(int, char* [])
{
  // always start with this!
  MITK_TEST_BEGIN("mitkImageStatisticsTextureAnalysisTest")

  mitkImageStatisticsTextureAnalysisTestClass::pointerOfImage label = mitkImageStatisticsTextureAnalysisTestClass:: CreatingTestLabelImage();
  mitkImageStatisticsTextureAnalysisTestClass::pointerOfImage image = mitkImageStatisticsTextureAnalysisTestClass:: CreatingTestImage();


   itk::ImageFileWriter<mitkImageStatisticsTextureAnalysisTestClass::ImageType>:: Pointer writer1 = itk::ImageFileWriter<mitkImageStatisticsTextureAnalysisTestClass::ImageType>::New();
 // writer->SetImageIO(nrrdImageIO);
  writer1->SetFileName("C:\\Users\\tmueller\\Documents\\TestPics\\SeedsFG_TEST1.nrrd");
  writer1->SetInput(label);
  writer1->Update();

   itk::ImageFileWriter<mitkImageStatisticsTextureAnalysisTestClass::ImageType>:: Pointer writer = itk::ImageFileWriter<mitkImageStatisticsTextureAnalysisTestClass::ImageType>::New();
 // writer->SetImageIO(nrrdImageIO);
  writer->SetFileName("C:\\Users\\tmueller\\Documents\\TestPics\\SeedsFG_TEST2.nrrd");
  writer->SetInput(image);
  writer->Update();


  mitkImageStatisticsTextureAnalysisTestClass::labelStatisticsFilterPointer mitkLabelFilter= mitkImageStatisticsTextureAnalysisTestClass::TestInstantiation( image,label);


  mitkImageStatisticsTextureAnalysisTestClass::TestTrueFalse(mitkLabelFilter, 0, 1);
 // superImage = CreateSuperImage();
 // superImage1 = CreateSuperImage();
 // superImage2 = CreateSuperImage();
 // superImage3 = CreateSuperImage();


  //mitkImageStatisticsTextureAnalysisTestClass::TestTrueFalse(superImage, 2334.33, 2332.3);
  //mitkImageStatisticsTextureAnalysisTestClass::TestTrueFalse(superImage1, 2312334.33, 43434.0);
  //mitkImageStatisticsTextureAnalysisTestClass::TestTrueFalse(superImage2, 123223.8, 2332.3);
  //mitkImageStatisticsTextureAnalysisTestClass::TestTrueFalse(superImage3, 2312334.33, 2332.3);

  MITK_TEST_END()
}
