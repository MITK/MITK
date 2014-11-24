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
#include "mitkExtendedStatisticsImageFilter.h"
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

  typedef itk::ExtendedStatisticsImageFilter< ImageType > StatisticsFilterType;
  typedef StatisticsFilterType::Pointer StatisticsFilterPointer;

  static ImageType::Pointer CreatingTestLabelImage()
  {
    ImageType::Pointer image = ImageType :: New();
    ImageType::IndexType start;
    ImageType::SizeType size;

    start[0] = 0;
    start[1] = 0;
    start[2] = 0;

    size[0] = 100;
    size[1] = 100;
    size[2] = 100;

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
    ImageType::Pointer image = ImageType :: New();
    ImageType::IndexType start;
    ImageType::SizeType size;

    start[0] = 0;
    start[1] = 0;
    start[2] = 0;

    size[0] = 100;
    size[1] = 100;
    size[2] = 100;

    ImageType:: RegionType region;
    region.SetSize( size );
    region.SetIndex( start );

    image->SetRegions(region);
    image->Allocate();
    image->FillBuffer(3.0);

    for(unsigned int r = 0; r < 50; r++)
    {
      for(unsigned int c = 0; c < 100; c++)
      {
        for(unsigned int l = 0; l < 100; l++)
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

  static ImageType::Pointer CreatingTestImage2()
  {
    ImageType::Pointer image = ImageType :: New();
    ImageType::IndexType start;
    ImageType::SizeType size;

    start[0] = 0;
    start[1] = 0;
    start[2] = 0;

    size[0] = 100;
    size[1] = 100;
    size[2] = 100;

    ImageType:: RegionType region;
    region.SetSize( size );
    region.SetIndex( start );

    image->SetRegions(region);
    image->Allocate();
    image->FillBuffer(3.0);

    for(unsigned int r = 0; r < 50; r++)
    {
      for(unsigned int c = 0; c < 50; c++)
      {
        for(unsigned int l = 0; l < 100; l++)
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
    LabelStatisticsFilterType::Pointer labelStatisticsFilter;
    labelStatisticsFilter = LabelStatisticsFilterType::New();
    labelStatisticsFilter->SetInput( image );
    labelStatisticsFilter->SetLabelInput( maskImage );
    labelStatisticsFilter->Update();


    return labelStatisticsFilter;
  }

  static StatisticsFilterType::Pointer  TestInstantiation2(ImageType::Pointer image )
  {
    StatisticsFilterType::Pointer StatisticsFilter;
    StatisticsFilter = StatisticsFilterType::New();
    StatisticsFilter->SetInput( image );
    StatisticsFilter->Update();


    return StatisticsFilter;
  }


  static void TestTrueFalse(LabelStatisticsFilterType::Pointer labelStatisticsFilter, double expectedSkewness, double expectedKurtosis)
  {
    // let's create an object of our class
    bool isSkewsnessLowerlimitCorrect = labelStatisticsFilter->GetSkewness( 1 )- expectedKurtosis+ std::pow(10,-3) <= expectedSkewness;
    bool isSkewsnessUpperlimitCorrect = labelStatisticsFilter->GetSkewness( 1 )+ expectedKurtosis+ std::pow(10,-3) >= expectedSkewness;

    MITK_TEST_CONDITION( isSkewsnessLowerlimitCorrect && isSkewsnessUpperlimitCorrect,"expectedSkewness: " << expectedSkewness << " actual Value: " << labelStatisticsFilter->GetSkewness( 1 ) );


    bool isKurtosisUpperlimitCorrect = labelStatisticsFilter->GetKurtosis( 1 ) <= expectedKurtosis+ std::pow(10,-3);
    bool isKurtosisLowerlimitCorrect = expectedKurtosis- std::pow(10,-3) <= labelStatisticsFilter->GetKurtosis( 1 );

    MITK_TEST_CONDITION( isKurtosisUpperlimitCorrect && isKurtosisLowerlimitCorrect,"expectedKurtosis: " << expectedKurtosis << " actual Value: " << labelStatisticsFilter->GetKurtosis( 1 ) );
  }



  static void TestTrueFalse2(StatisticsFilterType::Pointer StatisticsFilter, double expectedSkewness, double expectedKurtosis)
  {
    // let's create an object of our class
    bool isSkewsnessLowerlimitCorrect = StatisticsFilter->GetSkewness()- expectedKurtosis+ std::pow(10,-3) <= expectedSkewness;
    bool isSkewsnessUpperlimitCorrect = StatisticsFilter->GetSkewness()+ expectedKurtosis+ std::pow(10,-3) >= expectedSkewness;

    MITK_TEST_CONDITION( isSkewsnessLowerlimitCorrect && isSkewsnessUpperlimitCorrect,"expectedSkewness: " << expectedSkewness << " actual Value: " << StatisticsFilter->GetSkewness() );


    bool isKurtosisUpperlimitCorrect = StatisticsFilter->GetKurtosis() <= expectedKurtosis+ std::pow(10,-3);
    bool isKurtosisLowerlimitCorrect = expectedKurtosis- std::pow(10,-3) <= StatisticsFilter->GetKurtosis();

    MITK_TEST_CONDITION( isKurtosisUpperlimitCorrect && isKurtosisLowerlimitCorrect,"expectedKurtosis: " << expectedKurtosis << " actual Value: " << StatisticsFilter->GetKurtosis() );
  }


};


int mitkImageStatisticsTextureAnalysisTest(int, char* [])
{
  // always start with this!
  MITK_TEST_BEGIN("mitkImageStatisticsTextureAnalysisTest")

  mitkImageStatisticsTextureAnalysisTestClass::pointerOfImage label = mitkImageStatisticsTextureAnalysisTestClass:: CreatingTestLabelImage();

  mitkImageStatisticsTextureAnalysisTestClass::pointerOfImage image = mitkImageStatisticsTextureAnalysisTestClass:: CreatingTestImage();
  mitkImageStatisticsTextureAnalysisTestClass::pointerOfImage image2 = mitkImageStatisticsTextureAnalysisTestClass:: CreatingTestImage2();


  mitkImageStatisticsTextureAnalysisTestClass::labelStatisticsFilterPointer mitkLabelFilter= mitkImageStatisticsTextureAnalysisTestClass::TestInstantiation( image,label);
  mitkImageStatisticsTextureAnalysisTestClass::TestTrueFalse(mitkLabelFilter, 0, 0.999998);

  mitkImageStatisticsTextureAnalysisTestClass::labelStatisticsFilterPointer mitkLabelFilter2= mitkImageStatisticsTextureAnalysisTestClass::TestInstantiation( image2,label);
  mitkImageStatisticsTextureAnalysisTestClass::TestTrueFalse(mitkLabelFilter2, -1.1547, 2.33333);

  mitkImageStatisticsTextureAnalysisTestClass::StatisticsFilterPointer mitkFilter= mitkImageStatisticsTextureAnalysisTestClass::TestInstantiation2( image);
  mitkImageStatisticsTextureAnalysisTestClass::TestTrueFalse(mitkLabelFilter, 0, 0.999998);

  mitkImageStatisticsTextureAnalysisTestClass::StatisticsFilterPointer mitkFilter2= mitkImageStatisticsTextureAnalysisTestClass::TestInstantiation2( image2);
  mitkImageStatisticsTextureAnalysisTestClass::TestTrueFalse(mitkLabelFilter2, -1.1547, 2.33333);

  MITK_TEST_END()
}
