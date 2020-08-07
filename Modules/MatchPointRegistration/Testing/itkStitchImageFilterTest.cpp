/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"
#include "mitkTestDynamicImageGenerator.h"

#include "itkStitchImageFilter.h"
#include "itkTranslationTransform.h"
#include "itkNearestNeighborInterpolateImageFunction.h"

class itkStitchImageFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(itkStitchImageFilterTestSuite);
  // Test the append method
  MITK_TEST(StitchWithNoTransformAndNoInterp);
  MITK_TEST(StitchWithNoInterp);
  MITK_TEST(Stitch);
  CPPUNIT_TEST_SUITE_END();

  using InputImageType = mitk::TestImageType;
  using OutputImageType = itk::Image<double, 2>;
private:
  using FilterType = itk::StitchImageFilter<InputImageType, OutputImageType>;
  FilterType::Pointer m_Filter;

  InputImageType::Pointer m_Input1;
  InputImageType::Pointer m_Input2;
  InputImageType::Pointer m_Input3;

public:
  void setUp() override
  {
    InputImageType::PointType origin;
    origin.Fill(0.);
    InputImageType::SpacingType spacing;
    spacing.Fill(1.);
    spacing[1] = 5.;

    m_Filter = FilterType::New();
    m_Input1 = mitk::GenerateTestImage(1);
    m_Input1->SetSpacing(spacing);

    m_Input2 = mitk::GenerateTestImage(10);
    origin[1] = 10.;
    m_Input2->SetOrigin(origin);
    m_Input2->SetSpacing(spacing);

    m_Input3 = mitk::GenerateTestImage(100);
    origin[1] = 20.;
    m_Input3->SetOrigin(origin);
    m_Input3->SetSpacing(spacing);

    FilterType::SizeType size = { 3, 9 };
    m_Filter->SetDefaultPixelValue(1000);
    m_Filter->SetSize(size);
    m_Filter->SetOutputSpacing(spacing);
  }

  void tearDown() override
  {
  }

  bool CheckPixels(const OutputImageType* image, const std::vector<double>& pixels)
  {
    bool result = true;

    itk::ImageRegionConstIteratorWithIndex<OutputImageType> iter(image, image->GetLargestPossibleRegion());
    auto refIter = pixels.begin();
    iter.GoToBegin();
    while (!iter.IsAtEnd())
    {
      if (refIter == pixels.end())
      {
        std::cerr << "Error image to check has a different pixel count then the reference pixel value vector."<<std::endl;
        return false;
      }

      if (*refIter != iter.Get())
      {
        std::cerr << "Checked image differs from reference. Index: " << iter.GetIndex() << "; value: " << iter.Get() << "; ref: " << *refIter <<std::endl;
        result = false;
      }
      ++iter;
      ++refIter;
    }

    return result;
  }

  void StitchWithNoTransformAndNoInterp()
  {
    m_Filter->SetInput(0, m_Input1);
    m_Filter->SetInput(1, m_Input2);
    m_Filter->SetInput(2, m_Input3);

    m_Filter->Update();
    auto output = m_Filter->GetOutput();

    CPPUNIT_ASSERT(CheckPixels(output, {1, 2, 3, 4, 5, 6, 8.5, 14, 19.5, 40, 50, 60, 85, 140, 195, 400, 500, 600, 700, 800, 900, 1000, 1000, 1000, 1000, 1000, 1000}));
  }

  void StitchWithNoInterp()
  {
    m_Filter->SetInput(0, m_Input1);

    using TranslationType = itk::TranslationTransform<double, 2>;
    TranslationType::OutputVectorType offset;
    offset[0] = 0.;
    offset[1] = -5.;
    auto translation1 = TranslationType::New();
    translation1->SetOffset(offset);
    m_Filter->SetInput(1, m_Input2, translation1);
    
    offset[1] = -10.;
    auto translation2 = TranslationType::New();
    translation2->SetOffset(offset);
    m_Filter->SetInput(2, m_Input3, translation2);

    m_Filter->Update();
    auto output = m_Filter->GetOutput();

    CPPUNIT_ASSERT(CheckPixels(output, { 1,2,3,4,5,6,7,8,9,10,20,30,40,50,60,70,80,90,100,200,300,400,500,600,700,800,900}));
  }

  void Stitch()
  {
    using TranslationType = itk::TranslationTransform<double, 2>;
    TranslationType::OutputVectorType offset;
    offset[0] = 0;
    offset[1] = -7.5;
    auto translation1 = TranslationType::New();
    translation1->SetOffset(offset);

    offset[1] = -12.5;
    auto translation2 = TranslationType::New();
    translation2->SetOffset(offset);

    m_Filter->SetInput(0, m_Input1);
    m_Filter->SetInput(1, m_Input2, translation1, itk::NearestNeighborInterpolateImageFunction<InputImageType>::New());
    m_Filter->SetInput(2, m_Input3, translation2);

    m_Filter->Update();
    auto output = m_Filter->GetOutput();

    CPPUNIT_ASSERT(CheckPixels(output, { 1,2,3,4,5,6,7,8,9,10,20,30,40,50,60,70,80,90,100,200,300,250,350,450,550,650,750 }));
  }

};

MITK_TEST_SUITE_REGISTRATION(itkStitchImageFilter)
