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
#include <mitkBeamformingFilter.h>
#include <mitkIOUtil.h>
#include <usModuleResource.h>
#include <random>

class SyntheticPAImageData
{
public:
  SyntheticPAImageData(float spacing_x, float spacing_y, unsigned int samples, unsigned int num_transducers, float speedOfSound)
  {
    m_Spacing_x = spacing_x;
    m_Spacing_y = spacing_y;
    m_TransducerElements = num_transducers;
    m_Samples = samples;
    m_SpeedOfSound = speedOfSound;
    m_Data = new float[m_Samples*m_TransducerElements];

    for (size_t i = 0; i < m_Samples * m_TransducerElements; ++i)
    {
      m_Data[i] = 0;
    }
  }

  ~SyntheticPAImageData()
  {
    delete[] m_Data;
  }

  void AddWave(float origin_depth, float origin_x, float base_value= 10000)
  {
    AddLine(origin_depth, origin_x, base_value);
    AddLine(origin_depth + 0.0001f, origin_x, -base_value);
  }

  const float* GetData()
  {
    return (const float*)m_Data;
  }

private:
  void AddLine(float origin_depth, unsigned int origin_x, float base_value = 10000)
  {
    for (unsigned int x = 0; x < m_TransducerElements; ++x)
    {
      float distance = std::abs((int)x - (int)origin_x);
      float delay_in_seconds = std::sqrt(std::pow(origin_depth, 2) + std::pow(distance*m_Spacing_x, 2)) / m_SpeedOfSound;
      int pixels = std::round(delay_in_seconds / m_Spacing_y);

      for (int index = -4; index < 9; ++index)
      {
        if ((int)pixels + index < (int)m_Samples && (int)pixels + index > 0)
        {
          m_Data[(size_t)(x + (pixels + index)*m_TransducerElements)] += base_value / std::sqrt(distance + 1);
        }
      }
    }
  }

  float* m_Data;
  float m_Spacing_x;
  float m_Spacing_y;
  unsigned int m_TransducerElements;
  unsigned int m_Samples;
  float m_SpeedOfSound;
};

class mitkBeamformingFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkBeamformingFilterTestSuite);
  MITK_TEST(testBeamformingCPU_DAS);
  MITK_TEST(testBeamformingGPU_DAS);
  MITK_TEST(testBeamformingCPU_DMAS);
  MITK_TEST(testBeamformingGPU_DMAS);
  MITK_TEST(testBeamformingCPU_sDMAS);
  MITK_TEST(testBeamformingGPU_sDMAS);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::BeamformingFilter::Pointer m_BeamformingFilter;
  const unsigned int NUM_ITERATIONS = 15;
  const unsigned int SAMPLES = 5000 * 2;
  const unsigned int RECONSTRUCTED_SAMPLES = 2048;
  const unsigned int ELEMENTS = 128;
  const unsigned int RECONSTRUCTED_LINES = 128;
  const float SPEED_OF_SOUND = 1540; // m/s
  const float SPACING_X = 0.3; // mm
  const float SPACING_Y = 0.00625 / 2; // us
  const unsigned int GPU_BATCH_SIZE = 16;

public:

  void setUp() override
  {

  }

  void test()
  {
    std::random_device r;
    std::default_random_engine randGen(r());
    float maxDepthInMeters = SAMPLES * (SPACING_Y / 1000000) * SPEED_OF_SOUND / 2;
    std::uniform_real_distribution<float> randDistrDepth(maxDepthInMeters*0.1, maxDepthInMeters*0.8);

    for (unsigned int iteration = 0; iteration < NUM_ITERATIONS; ++iteration)
    {
      // create some synthetic input data
      float depth_in_meters1 = randDistrDepth(randGen);
      float depth_in_meters2 = randDistrDepth(randGen);
      float depth_in_meters3 = randDistrDepth(randGen);

      unsigned int element1 = 29;
      unsigned int element2 = 63;
      unsigned int element3 = 98;

      SyntheticPAImageData image(SPACING_X / 1000.f, SPACING_Y / 1000000.f, SAMPLES, ELEMENTS, SPEED_OF_SOUND);
      image.AddWave(depth_in_meters1, 29);
      image.AddWave(depth_in_meters2, 63);
      image.AddWave(depth_in_meters3, 98);

      mitk::Image::Pointer inputImage = mitk::Image::New();
      unsigned int dimension[3]{ ELEMENTS, SAMPLES, 1 };
      inputImage->Initialize(mitk::MakeScalarPixelType<float>(), 3, dimension);
      mitk::Vector3D spacing;
      spacing[0] = SPACING_X;
      spacing[1] = SPACING_Y;
      spacing[2] = 1;
      inputImage->SetSpacing(spacing);
      inputImage->SetImportVolume((const void*)image.GetData(), mitk::Image::CopyMemory);

      // setup the beamforming filter
      m_BeamformingFilter->SetInput(inputImage);
      m_BeamformingFilter->Update();

      mitk::Image::Pointer outputImage = m_BeamformingFilter->GetOutput();
      mitk::ImageReadAccessor readAccess(outputImage);
      const float* outputData = (const float*)readAccess.GetData();

      unsigned int pos1[3] = { element1, (unsigned int)std::round(depth_in_meters1 * 1000.f / outputImage->GetGeometry()->GetSpacing()[1]), 0 };
      unsigned int pos2[3] = { element2, (unsigned int)std::round(depth_in_meters2 * 1000.f / outputImage->GetGeometry()->GetSpacing()[1]), 0 };
      unsigned int pos3[3] = { element3, (unsigned int)std::round(depth_in_meters3 * 1000.f / outputImage->GetGeometry()->GetSpacing()[1]), 0 };

      double average = 0;

      for (unsigned int i = 0; i < RECONSTRUCTED_LINES*RECONSTRUCTED_SAMPLES; ++i)
      {
        average += outputData[i] / (RECONSTRUCTED_LINES*RECONSTRUCTED_SAMPLES);
      }

      CPPUNIT_ASSERT_MESSAGE(std::string("Iteration " + std::to_string(iteration) + ": first point source incorrectly reconstructed; should be > average*100, is " +
        std::to_string(abs(outputData[pos1[0] + pos1[1] * RECONSTRUCTED_LINES]))) + " < " + std::to_string(average) + "*100"
        , abs(outputData[pos1[0] + pos1[1]*RECONSTRUCTED_LINES] / average) > 100);
      CPPUNIT_ASSERT_MESSAGE(std::string("Iteration " + std::to_string(iteration) + ": second point source incorrectly reconstructed; should be > average*100, is " +
        std::to_string(abs(outputData[pos2[0] + pos2[1] * RECONSTRUCTED_LINES]))) + " < " + std::to_string(average) + "*100"
        , abs(outputData[pos2[0] + pos2[1] * RECONSTRUCTED_LINES] / average) > 100);
      CPPUNIT_ASSERT_MESSAGE(std::string("Iteration " + std::to_string(iteration) + ": third point source incorrectly reconstructed; should be > average*100, is " +
        std::to_string(abs(outputData[pos3[0] + pos3[1] * RECONSTRUCTED_LINES]))) + " < " + std::to_string(average) + "*100"
        , abs(outputData[pos3[0] + pos3[1] * RECONSTRUCTED_LINES] / average) > 100);
    }
  }

  mitk::BeamformingSettings::Pointer createConfig(bool UseGPU, unsigned int* inputDim, mitk::BeamformingSettings::BeamformingAlgorithm alg)
  {
    return mitk::BeamformingSettings::New(SPACING_X / 1000,
      SPEED_OF_SOUND,
      SPACING_Y / 1000000,
      27.f,
      true,
      RECONSTRUCTED_SAMPLES,
      RECONSTRUCTED_LINES,
      inputDim,
      SPEED_OF_SOUND * (SPACING_Y / 1000000) * SAMPLES,
      UseGPU,
      GPU_BATCH_SIZE,
      mitk::BeamformingSettings::DelayCalc::Spherical,
      mitk::BeamformingSettings::Apodization::Box,
      ELEMENTS * 2,
      alg);
  }

  void testBeamformingCPU_DAS()
  {
    MITK_INFO << "Started DAS test on CPU";
    unsigned int* inputDim = new unsigned int[3];
    inputDim[0] = ELEMENTS;
    inputDim[1] = SAMPLES;
    inputDim[2] = 1;
    m_BeamformingFilter = mitk::BeamformingFilter::New(createConfig(false, inputDim, mitk::BeamformingSettings::BeamformingAlgorithm::DAS));

    test();
    delete[] inputDim;
  }

  void testBeamformingGPU_DAS()
  {
    MITK_INFO << "Started DAS test on GPU";
    unsigned int* inputDim = new unsigned int[3];
    inputDim[0] = ELEMENTS;
    inputDim[1] = SAMPLES;
    inputDim[2] = 1;
    m_BeamformingFilter = mitk::BeamformingFilter::New(createConfig(true, inputDim, mitk::BeamformingSettings::BeamformingAlgorithm::DAS));

    test();
    delete[] inputDim;
  }

  void testBeamformingCPU_sDMAS()
  {
    MITK_INFO << "Started sDMAS test on CPU";
    unsigned int* inputDim = new unsigned int[3];
    inputDim[0] = ELEMENTS;
    inputDim[1] = SAMPLES;
    inputDim[2] = 1;
    m_BeamformingFilter = mitk::BeamformingFilter::New(createConfig(false, inputDim, mitk::BeamformingSettings::BeamformingAlgorithm::sDMAS));

    test();
    delete[] inputDim;
  }

  void testBeamformingGPU_sDMAS()
  {
    MITK_INFO << "Started sDMAS test on GPU";
    unsigned int* inputDim = new unsigned int[3];
    inputDim[0] = ELEMENTS;
    inputDim[1] = SAMPLES;
    inputDim[2] = 1;
    m_BeamformingFilter = mitk::BeamformingFilter::New(createConfig(true, inputDim, mitk::BeamformingSettings::BeamformingAlgorithm::sDMAS));

    test();
    delete[] inputDim;
  }

  void testBeamformingCPU_DMAS()
  {
    MITK_INFO << "Started DMAS test on CPU";
    unsigned int* inputDim = new unsigned int[3];
    inputDim[0] = ELEMENTS;
    inputDim[1] = SAMPLES;
    inputDim[2] = 1;
    m_BeamformingFilter = mitk::BeamformingFilter::New(createConfig(false, inputDim, mitk::BeamformingSettings::BeamformingAlgorithm::DMAS));

    test();
    delete[] inputDim;
  }

  void testBeamformingGPU_DMAS()
  {
    MITK_INFO << "Started DMAS test on GPU";
    unsigned int* inputDim = new unsigned int[3];
    inputDim[0] = ELEMENTS;
    inputDim[1] = SAMPLES;
    inputDim[2] = 1;
    m_BeamformingFilter = mitk::BeamformingFilter::New(createConfig(true, inputDim, mitk::BeamformingSettings::BeamformingAlgorithm::DMAS));

    test();
    delete[] inputDim;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkBeamformingFilter)
