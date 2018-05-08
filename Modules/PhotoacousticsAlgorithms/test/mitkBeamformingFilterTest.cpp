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

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkImage.h>
#include <mitkImageReadAccessor.h>
#include <mitkBeamformingFilter.h>

class mitkBeamformingFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkBeamformingFilterTestSuite);
  MITK_TEST(testBeamformingCPU);
  MITK_TEST(testBeamformingGPU);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::BeamformingFilter::Pointer m_BeamformingFilter;
  const unsigned int NUM_ITERATIONS = 15;
  const unsigned int DATA_DIM = 10;

public:

  void setUp() override
  {
    m_BeamformingFilter = mitk::BeamformingFilter::New(nullptr);
  }

  void test(bool GPU)
  {
    float* data = new float[DATA_DIM*DATA_DIM*DATA_DIM];

    for (unsigned int i = 0; i < DATA_DIM*DATA_DIM*DATA_DIM; ++i)
    {
      data[i] = 0;
    }

    mitk::Image::Pointer inputImage = mitk::Image::New();
    unsigned int dimension[3]{ DATA_DIM, DATA_DIM, DATA_DIM };
    inputImage->Initialize(mitk::MakeScalarPixelType<float>(), 3, dimension);
    inputImage->SetImportVolume(data);

    for (unsigned int iteration = 0; iteration < NUM_ITERATIONS; ++iteration)
    {
      m_BeamformingFilter->SetInput(inputImage);
      m_BeamformingFilter->Update();
      mitk::Image::Pointer outputImage = m_BeamformingFilter->GetOutput();
      mitk::ImageReadAccessor readAccess(outputImage);
      const float* outputData = (const float*)readAccess.GetData();
      for (unsigned int i = 0; i < DATA_DIM*DATA_DIM*DATA_DIM; ++i)
      {
        CPPUNIT_ASSERT_MESSAGE(std::string("expected " + std::to_string(data[i]) + " but was " + std::to_string(outputData[i])), abs(outputData[i] - data[i]) < mitk::eps);
      }
    }
  }

  void testBeamformingCPU()
  {
    test(false);
  }

  void testBeamformingGPU()
  {
    test(true);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkBeamformingFilter)
