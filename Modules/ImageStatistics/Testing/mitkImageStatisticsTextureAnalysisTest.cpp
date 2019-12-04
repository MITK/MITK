/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include "itkImage.h"
#include "mitkExtendedLabelStatisticsImageFilter.h"
#include "mitkExtendedStatisticsImageFilter.h"
#include "mitkNumericConstants.h"

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

  typedef itk::Image< int,3 >ImageType;
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
      for(int c = 0; c < factorOfDividingThePicture; c++)
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
    labelStatisticsFilter->UseHistogramsOn();
    labelStatisticsFilter->SetHistogramParameters( 20, -10, 10);
    labelStatisticsFilter->SetLabelInput( maskImage );
    labelStatisticsFilter->Update();


    return labelStatisticsFilter;
  }

  StatisticsFilterType::Pointer  TestInstanceFortheUnmaskedStatisticsFilter(ImageType::Pointer image )
  {
    StatisticsFilterType::Pointer StatisticsFilter;
    StatisticsFilter = StatisticsFilterType::New();
    StatisticsFilter->SetInput( image );
    StatisticsFilter->SetHistogramParameters( 20, -10, 10 );
    StatisticsFilter->Update();


    return StatisticsFilter;
  }

  //test for Skewness,Kurtosis and MPP for masked Images
  void TestofSkewnessKurtosisAndMPPForMaskedImages(LabelStatisticsFilterType::Pointer labelStatisticsFilter, double expectedSkewness, double expectedKurtosis, double expectedMPP)
  {
    // let's create an object of our class
    bool isSkewsnessLowerlimitCorrect = labelStatisticsFilter->GetSkewness( 1 )- expectedKurtosis+ std::pow(10,-3) <= expectedSkewness;
    bool isSkewsnessUpperlimitCorrect = labelStatisticsFilter->GetSkewness( 1 )+ expectedKurtosis+ std::pow(10,-3) >= expectedSkewness;

    MITK_TEST_CONDITION( isSkewsnessLowerlimitCorrect && isSkewsnessUpperlimitCorrect,"expectedSkewness: " << expectedSkewness << " actual Value: " << labelStatisticsFilter->GetSkewness( 1 ) );


    bool isKurtosisUpperlimitCorrect = labelStatisticsFilter->GetKurtosis( 1 ) <= expectedKurtosis+ std::pow(10,-3);
    bool isKurtosisLowerlimitCorrect = expectedKurtosis- std::pow(10,-3) <= labelStatisticsFilter->GetKurtosis( 1 );

    MITK_TEST_CONDITION( isKurtosisUpperlimitCorrect && isKurtosisLowerlimitCorrect,"expectedKurtosis: " << expectedKurtosis << " actual Value: " << labelStatisticsFilter->GetKurtosis( 1 ) );

    MITK_TEST_CONDITION( ( expectedMPP - labelStatisticsFilter->GetMPP( 1 ) ) < 1, "expected MPP: " << expectedMPP << " actual Value: " << labelStatisticsFilter->GetMPP( 1 ) );
  }

  //test for Entropy,Uniformity and UPP for masked Images
  void TestofEntropyUniformityAndUppForMaskedImages(LabelStatisticsFilterType::Pointer labelStatisticsFilter, double expectedEntropy, double expectedUniformity, double expectedUPP)
  {
    bool calculatedEntropyLowerLimit = labelStatisticsFilter->GetEntropy( 1 ) >= expectedEntropy - std::pow(10,-3);
    bool calculatedUniformityLowerLimit = labelStatisticsFilter->GetUniformity( 1 ) >= expectedUniformity - std::pow(10,-3);
    bool calculatedUppLowerLimit = labelStatisticsFilter->GetUPP( 1 ) >= expectedUPP - std::pow(10,-3);

    bool calculatedEntropyUpperLimit = labelStatisticsFilter->GetEntropy( 1 ) <= expectedEntropy + std::pow(10,-3);
    bool calculatedUniformityUpperLimit = labelStatisticsFilter->GetUniformity( 1 ) <= expectedUniformity + std::pow(10,-3);
    bool calculatedUppUpperLimit = labelStatisticsFilter->GetUPP( 1 ) <= expectedUPP + std::pow(10,-3);


   MITK_TEST_CONDITION( calculatedEntropyLowerLimit && calculatedEntropyUpperLimit, "expected Entropy: " << expectedEntropy << " actual Value: " << labelStatisticsFilter->GetEntropy( 1 ) );
   MITK_TEST_CONDITION( calculatedUniformityLowerLimit && calculatedUniformityUpperLimit, "expected Uniformity: " << expectedUniformity << " actual Value: " << labelStatisticsFilter->GetUniformity( 1 ) );
   MITK_TEST_CONDITION( calculatedUppLowerLimit && calculatedUppUpperLimit, "expected UPP: " << expectedUPP << " actual Value: " << labelStatisticsFilter->GetUPP( 1 ) );
  }

    //test for Skewness,Kurtosis and MPP for unmasked Images
  void TestofSkewnessKurtosisAndMPPForUnmaskedImages(StatisticsFilterType::Pointer StatisticsFilter, double expectedSkewness, double expectedKurtosis, double expectedMPP)
  {
    // let's create an object of our class
    bool isSkewsnessLowerlimitCorrect = StatisticsFilter->GetSkewness()- expectedKurtosis+ std::pow(10,-3) <= expectedSkewness;
    bool isSkewsnessUpperlimitCorrect = StatisticsFilter->GetSkewness()+ expectedKurtosis+ std::pow(10,-3) >= expectedSkewness;

    MITK_TEST_CONDITION( isSkewsnessLowerlimitCorrect && isSkewsnessUpperlimitCorrect,"expectedSkewness: " << expectedSkewness << " actual Value: " << StatisticsFilter->GetSkewness() );


    bool isKurtosisUpperlimitCorrect = StatisticsFilter->GetKurtosis() <= expectedKurtosis+ std::pow(10,-3);
    bool isKurtosisLowerlimitCorrect = expectedKurtosis- std::pow(10,-3) <= StatisticsFilter->GetKurtosis();

    MITK_TEST_CONDITION( isKurtosisUpperlimitCorrect && isKurtosisLowerlimitCorrect,"expectedKurtosis: " << expectedKurtosis << " actual Value: " << StatisticsFilter->GetKurtosis() );

    MITK_TEST_CONDITION( ( expectedMPP - StatisticsFilter->GetMPP() ) < mitk::eps, "expected MPP: " << expectedMPP << " actual Value: " << StatisticsFilter->GetMPP() );
  }

  //test for Entropy,Uniformity and UPP for unmasked Images
  void TestofEntropyUniformityAndUppForUnmaskedImages(StatisticsFilterType::Pointer StatisticsFilter, double expectedEntropy, double expectedUniformity, double expectedUPP)
  {
    bool calculatedEntropyLowerLimit = StatisticsFilter->GetEntropy() >= expectedEntropy - std::pow(10,-3);
    bool calculatedUniformityLowerLimit = StatisticsFilter->GetUniformity() >= expectedUniformity - std::pow(10,-3);
    bool calculatedUppLowerLimit = StatisticsFilter->GetUPP() >= expectedUPP - std::pow(10,-3);

    bool calculatedEntropyUpperLimit = StatisticsFilter->GetEntropy() <= expectedEntropy + std::pow(10,-3);
    bool calculatedUniformityUpperLimit = StatisticsFilter->GetUniformity() <= expectedUniformity + std::pow(10,-3);
    bool calculatedUppUpperLimit = StatisticsFilter->GetUPP() <= expectedUPP + std::pow(10,-3);


    MITK_TEST_CONDITION( calculatedEntropyLowerLimit && calculatedEntropyUpperLimit, "expected Entropy: " << expectedEntropy << " actual Value: " << StatisticsFilter->GetEntropy() );
    MITK_TEST_CONDITION( calculatedUniformityLowerLimit && calculatedUniformityUpperLimit, "expected Uniformity: " << expectedUniformity << " actual Value: " << StatisticsFilter->GetUniformity() );
    MITK_TEST_CONDITION( calculatedUppLowerLimit && calculatedUppUpperLimit, "expected UPP: " << expectedUPP << " actual Value: " << StatisticsFilter->GetUPP() );
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

  //test for masked images
  mitkImageStatisticsTextureAnalysisTestClass::labelStatisticsFilterPointer mitkLabelFilter= testclassInstance.TestInstanceFortheMaskedStatisticsFilter( image,labelImage);
  testclassInstance.TestofSkewnessKurtosisAndMPPForMaskedImages(mitkLabelFilter, 0, 0.999998, 2.5);

  testclassInstance.TestofEntropyUniformityAndUppForMaskedImages(mitkLabelFilter, 1, 0.5, 0.5);

  mitkImageStatisticsTextureAnalysisTestClass::labelStatisticsFilterPointer mitkLabelFilter2= testclassInstance.TestInstanceFortheMaskedStatisticsFilter( image2,labelImage);
  testclassInstance.TestofSkewnessKurtosisAndMPPForMaskedImages(mitkLabelFilter2, -1.1547, 2.33333, 2.75);

  testclassInstance.TestofEntropyUniformityAndUppForMaskedImages(mitkLabelFilter2, 0.811278, 0.625, 0.625);



  //test for unmasked images
  mitkImageStatisticsTextureAnalysisTestClass::StatisticsFilterPointer mitkFilter= testclassInstance.TestInstanceFortheUnmaskedStatisticsFilter( image);
  testclassInstance.TestofSkewnessKurtosisAndMPPForUnmaskedImages(mitkFilter, 0, 0.999998, 2.5);

  testclassInstance.TestofEntropyUniformityAndUppForUnmaskedImages(mitkFilter, 1, 0.5, 0.5);

  mitkImageStatisticsTextureAnalysisTestClass::StatisticsFilterPointer mitkFilter2= testclassInstance.TestInstanceFortheUnmaskedStatisticsFilter( image2);
  testclassInstance.TestofSkewnessKurtosisAndMPPForUnmaskedImages(mitkFilter2, -1.1547, 2.33333, 2.75);

  testclassInstance.TestofEntropyUniformityAndUppForUnmaskedImages( mitkFilter2, 0.811278, 0.625, 0.625);

  MITK_TEST_END()
}
