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

class itkStitchImageFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(itkStitchImageFilterTestSuite);
  // Test the append method
  MITK_TEST(StitchWithNoTransformAndNoInterp);
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
    m_Filter = FilterType::New();
    m_Input1 = mitk::GenerateTestImage(1);

    m_Input2 = mitk::GenerateTestImage(2);
    origin[1] = 3.;
    m_Input2->SetOrigin(origin);

    m_Input3 = mitk::GenerateTestImage(3);
    origin[1] = 6.;
    m_Input3->SetOrigin(origin);

    FilterType::SizeType size = { 3, 9 };
    m_Filter->SetSize(size);
  }

  void tearDown() override
  {
  }

  void StitchWithNoTransformAndNoInterp()
  {
    m_Filter->SetInput(0, m_Input1);
    m_Filter->SetInput(1, m_Input2);
    m_Filter->SetInput(2, m_Input3);

    m_Filter->Update();
    auto output = m_Filter->GetOutput();
  }


};

MITK_TEST_SUITE_REGISTRATION(itkStitchImageFilter)
