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

#include "mitkTestingMacros.h"
#include "itkImage.h"
#include "mitkExtendedLabelStatisticsImageFilter.h"
#include "mitkExtendedStatisticsImageFilter.h"

/**
 \section Testing of Skewness and Kurtosis

  * This test class is for testing the added coefficients Skewness and Kurtosis
  * for the mitkExtendedLabelStatisticsImageFilter (Masked Images) and for the
  * mitkExtendedStatisticsImageFilter (Unmasked Images). Both filter will be tested
  * against two pictures.
*/

class mitkImageStatisticsTextureAnalysisTestClass
{
   /**
  * \brief Explanation of the mitkImageStatisticsTextureAnalysisTestClass test class
  *
  * this test class produce test images and masking images with the method CreatingTestImageForDifferentLabelSize.
  * TestInstanceFortheMaskedStatisticsFilter and TestInstanceFortheUnmaskedStatisticsFilter are the two Instances
  * for the filters of masking and unmasking images.
  * TestofSkewnessAndKurtosisForMaskedImagesand TestofSkewnessAndKurtosisForUnmaskedImages are the correlated test
  * for the checking the values.
  */

public:

  typedef itk::Image<unsigned short,3 >ImageType;
  typedef ImageType::Pointer PointerOfImage;

  typedef itk::ExtendedLabelStatisticsImageFilter< ImageType, ImageType > LabelStatisticsFilterType;
  typedef LabelStatisticsFilterType::Pointer labelStatisticsFilterPointer;

  typedef itk::ExtendedStatisticsImageFilter< ImageType > StatisticsFilterType;
  typedef StatisticsFilterType::Pointer StatisticsFilterPointer;


  ImageType::Pointer CreatingTestImageForDifferentLabelSize( int factorOfDividingThePicture, int bufferValue, int labelValue)
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
    image->FillBuffer(bufferValue);

    for(unsigned int r = 0; r < 50; r++)
    {
      for(unsigned int c = 0; c < factorOfDividingThePicture; c++)
      {
        for(unsigned int l = 0; l < 100; l++)
        {
          ImageType::IndexType pixelIndex;
          pixelIndex[0] = r;
          pixelIndex[1] = c;
          pixelIndex[2] = l;

          image->SetPixel(pixelIndex, labelValue);
        }
      }
    }

    return image;
  }


  LabelStatisticsFilterType::Pointer  TestInstanceFortheMaskedStatisticsFilter(ImageType::Pointer image, ImageType::Pointer maskImage)
  {
    LabelStatisticsFilterType::Pointer labelStatisticsFilter;
    labelStatisticsFilter = LabelStatisticsFilterType::New();
    labelStatisticsFilter->SetInput( image );
    labelStatisticsFilter->SetLabelInput( maskImage );
    labelStatisticsFilter->Update();


    return labelStatisticsFilter;
  }

  StatisticsFilterType::Pointer  TestInstanceFortheUnmaskedStatisticsFilter(ImageType::Pointer image )
  {
    StatisticsFilterType::Pointer StatisticsFilter;
    StatisticsFilter = StatisticsFilterType::New();
    StatisticsFilter->SetInput( image );
    StatisticsFilter->Update();


    return StatisticsFilter;
  }


  void TestofSkewnessAndKurtosisForMaskedImages(LabelStatisticsFilterType::Pointer labelStatisticsFilter, double expectedSkewness, double expectedKurtosis)
  {
    // let's create an object of our class
    bool isSkewsnessLowerlimitCorrect = labelStatisticsFilter->GetSkewness( 1 )- expectedKurtosis+ std::pow(10,-3) <= expectedSkewness;
    bool isSkewsnessUpperlimitCorrect = labelStatisticsFilter->GetSkewness( 1 )+ expectedKurtosis+ std::pow(10,-3) >= expectedSkewness;

    MITK_TEST_CONDITION( isSkewsnessLowerlimitCorrect && isSkewsnessUpperlimitCorrect,"expectedSkewness: " << expectedSkewness << " actual Value: " << labelStatisticsFilter->GetSkewness( 1 ) );


    bool isKurtosisUpperlimitCorrect = labelStatisticsFilter->GetKurtosis( 1 ) <= expectedKurtosis+ std::pow(10,-3);
    bool isKurtosisLowerlimitCorrect = expectedKurtosis- std::pow(10,-3) <= labelStatisticsFilter->GetKurtosis( 1 );

    MITK_TEST_CONDITION( isKurtosisUpperlimitCorrect && isKurtosisLowerlimitCorrect,"expectedKurtosis: " << expectedKurtosis << " actual Value: " << labelStatisticsFilter->GetKurtosis( 1 ) );
  }



  void TestofSkewnessAndKurtosisForUnmaskedImages(StatisticsFilterType::Pointer StatisticsFilter, double expectedSkewness, double expectedKurtosis)
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

    mitkImageStatisticsTextureAnalysisTestClass testclassInstance;

  mitkImageStatisticsTextureAnalysisTestClass::PointerOfImage labelImage = testclassInstance.CreatingTestImageForDifferentLabelSize(100, 1, 1);
  mitkImageStatisticsTextureAnalysisTestClass::PointerOfImage image  = testclassInstance.CreatingTestImageForDifferentLabelSize(100, 3, 2);
  mitkImageStatisticsTextureAnalysisTestClass::PointerOfImage image2 = testclassInstance.CreatingTestImageForDifferentLabelSize(50, 3, 2);


  mitkImageStatisticsTextureAnalysisTestClass::labelStatisticsFilterPointer mitkLabelFilter= testclassInstance.TestInstanceFortheMaskedStatisticsFilter( image,labelImage);
  testclassInstance.TestofSkewnessAndKurtosisForMaskedImages(mitkLabelFilter, 0, 0.999998);

  mitkImageStatisticsTextureAnalysisTestClass::labelStatisticsFilterPointer mitkLabelFilter2= testclassInstance.TestInstanceFortheMaskedStatisticsFilter( image2,labelImage);
  testclassInstance.TestofSkewnessAndKurtosisForMaskedImages(mitkLabelFilter2, -1.1547, 2.33333);

  mitkImageStatisticsTextureAnalysisTestClass::StatisticsFilterPointer mitkFilter= testclassInstance.TestInstanceFortheUnmaskedStatisticsFilter( image);
  testclassInstance.TestofSkewnessAndKurtosisForUnmaskedImages(mitkFilter, 0, 0.999998);

  mitkImageStatisticsTextureAnalysisTestClass::StatisticsFilterPointer mitkFilter2= testclassInstance.TestInstanceFortheUnmaskedStatisticsFilter( image2);
  testclassInstance.TestofSkewnessAndKurtosisForUnmaskedImages(mitkFilter2, -1.1547, 2.33333);

  MITK_TEST_END()
}
