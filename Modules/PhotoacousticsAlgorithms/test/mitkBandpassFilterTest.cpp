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
#include <mitkBandpassFilter.h>
#include <cmath>
#include <random>

class mitkBandpassFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkBandpassFilterTestSuite);
  MITK_TEST(testHighPass);
  MITK_TEST(testLowPass);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::BandpassFilter::Pointer m_BandpassFilter;
  const unsigned int NUM_ITERATIONS = 15;
  const unsigned int DATA_DIM = 500;
  const unsigned float TIME_SPACING = 0.0001; // [us]
  const unsigned float HIGHPASS_FREQENCY = 1.f / TIMESPACING * DATA_DIM * 0.7f; // [Hz]
  const unsigned float LOWPASS_FREQENCY = 1.f / TIMESPACING * DATA_DIM * 0.3f; // [Hz]
  const unsigned float ALPHA = 0; // 0 = box, 1 = von Hann; changing this may make the test invalid

public:

  void setUp() override
  {
    m_BandpassFilter = mitk::BandpassFilter::New();
  }

  void test(float signalFreq, float HighPass, float LowPass, float HighPassAlpha, float LowPassAlpha)
  {
    std::random_device r;
    std::default_random_engine randGen(r());
    std::uniform_real_distribution<float> randDistrHighPass(0, HighPass * 0.9f);
    std::uniform_real_distribution<float> randDistrLowPass(LowPass * 1.1f, LowPass * 10);

    float* data = new float[DATA_DIM*DATA_DIM*DATA_DIM];

    mitk::Image::Pointer inputImage = mitk::Image::New();
    unsigned int dimension[3]{ DATA_DIM, DATA_DIM, DATA_DIM };
    inputImage->Initialize(mitk::MakeScalarPixelType<float>(), 3, dimension);
    mitk::Vector3D spacing { 1, TIME_SPACING, 1 };
    inputImage->SetSpacing(spacing);

    for (unsigned int iteration = 0; iteration < NUM_ITERATIONS; ++iteration)
    {
      for (unsigned int i = 0; i < DATA_DIM*DATA_DIM*DATA_DIM; ++i)
      {
        data[i] = 0;
      }

      if(HighPass != -1)
        addFrequency(randDistrHighPass(randGen), TIME_SPACING, data, dimension);
      if(LowPass != -1)
        addFrequency(randDistrLowPass(randGen), TIME_SPACING, data, dimension);

      inputImage->SetImportVolume(data, 0, 0, mitk::Image::ImportMemoryManagementType::CopyMemory);

      m_BandpassFilter->SetInput(inputImage);

      m_BandpassFilter->SetHighPass(HighPass);
      m_BandpassFilter->SetLowPass(LowPass);
      m_BandpassFilter->SetHighPassAlpha(HighPassAlpha);
      m_BandpassFilter->SetLowPassAlpha(LowPassAlpha);

      m_BandpassFilter->Update();
      mitk::Image::Pointer outputImage = m_BandpassFilter->GetOutput();

      mitk::ImageReadAccessor readAccess(outputImage);
      const float* outputData = (const float*)readAccess.GetData();

      for (unsigned int z = 0; z < DATA_DIM; ++z)
      {
        for (unsigned int y = 0; y < DATA_DIM; ++y)
        {
          for (unsigned int x = 0; x < DATA_DIM; ++x)
          {
            unsigned int outPos = x + y * DATA_DIM + z * DATA_DIM * DATA_DIM;
            CPPUNIT_ASSERT_MESSAGE(std::string("expected" + "" + "sth but no :(, got " + std::to_string(outputData[outPos])),
            abs(1) < mitk::eps);
          }
        }
      }
    }

    delete[] data;
  }

  void addFrequency(float freq, float timeSpacing, float* data, unsigned int* dim)
  {
    for(unsigned int z = 0; z < dim[2]; ++z)
    {
      for(unsigned int y = 0; y < dim[2]; ++y)
      {
        for(unsigned int x = 0; x < dim[2]; ++x)
        {
          data[x + y*dim[0] + z*dim[0]*dim[1]] += std::sin(freq * timeSpacing * y);
        }
      }
    }
  }

  void testHighPass()
  {
    test(HIGHPASS_FREQENCY, -1, ALPHA, 0);
  }

  void testLowPass()
  {
    test(-1, LOWPASS_FREQENCY, 0, ALPHA);
  }

  void tearDown() override
  {
    m_BandpassFilter = nullptr;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkBandpassFilterTestSuite)
