/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkImage.h>
#include <mitkImageReadAccessor.h>
#include <mitkCastToFloatImageFilter.h>

class mitkCastToFloatImageFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkCastToFloatImageFilterTestSuite);
  MITK_TEST(testShortConversion);
  MITK_TEST(testIntConversion);
  MITK_TEST(testLongConversion);
  MITK_TEST(testFloatConversion);
  MITK_TEST(testDoubleConversion);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::CastToFloatImageFilter::Pointer m_CastToFloatImageFilter;
  const unsigned int NUM_ITERATIONS = 15;
  const unsigned int DATA_DIM = 10;

public:

  void setUp() override
  {
    m_CastToFloatImageFilter = mitk::CastToFloatImageFilter::New();
  }

  template<class TPixelType>
  void test()
  {
    TPixelType* data = new TPixelType[DATA_DIM*DATA_DIM*DATA_DIM];

    for (unsigned int i = 0; i < DATA_DIM*DATA_DIM*DATA_DIM; ++i)
    {
      data[i] = (TPixelType)i;
    }

    mitk::Image::Pointer inputImage = mitk::Image::New();
    unsigned int dimension[3]{ DATA_DIM, DATA_DIM, DATA_DIM };
    inputImage->Initialize(mitk::MakeScalarPixelType<TPixelType>(), 3, dimension);
    inputImage->SetImportVolume(data);

    for (unsigned int iteration = 0; iteration < NUM_ITERATIONS; ++iteration)
    {
      m_CastToFloatImageFilter->SetInput(inputImage);
      m_CastToFloatImageFilter->Update();
      mitk::Image::Pointer outputImage = m_CastToFloatImageFilter->GetOutput();
      mitk::ImageReadAccessor readAccess(outputImage);
      const float* outputData = (const float*)readAccess.GetData();
      for (unsigned int i = 0; i < DATA_DIM*DATA_DIM*DATA_DIM; ++i)
      {
        CPPUNIT_ASSERT_MESSAGE(std::string("expected " + std::to_string(data[i]) + " but was " + std::to_string(outputData[i])), std::abs(outputData[i] - data[i]) < mitk::eps);
      }
    }
  }

  void testFloatConversion()
  {
    test<float>();
  }

  void testShortConversion()
  {
    test<short>();
    test<unsigned short>();
  }

  void testIntConversion()
  {
    test<int>();
    test<unsigned int>();
  }

  void testDoubleConversion()
  {
    test<double>();
  }

  void testLongConversion()
  {
    test<long>();
  }

  void tearDown() override
  {
    m_CastToFloatImageFilter = nullptr;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkCastToFloatImageFilter)
