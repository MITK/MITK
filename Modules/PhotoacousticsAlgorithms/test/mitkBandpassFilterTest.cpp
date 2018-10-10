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
#define _USE_MATH_DEFINES
#include <cmath>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkImage.h>
#include <mitkImageReadAccessor.h>
#include <mitkPhotoacousticFilterService.h>
#include <random>
#include <mitkIOUtil.h>

#include "../ITKFilter/ITKUltrasound/itkFFT1DRealToComplexConjugateImageFilter.h"
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"
#include "itkComplexToModulusImageFilter.h"

class mitkBandpassFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkBandpassFilterTestSuite);
  MITK_TEST(testHighPass);
  MITK_TEST(testLowPass);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::PhotoacousticFilterService::Pointer m_FilterService;
  const unsigned int NUM_ITERATIONS = 10;
  const unsigned int DATA_XY_DIM = 512;
  const unsigned int DATA_Z_DIM = 8;
  const float TIME_SPACING = 0.00625; // [us]
  float FREQUENCY_RESOLUTION = 1 / (TIME_SPACING / 1e6 * DATA_XY_DIM); // [Hz]
  const float MAX_FREQUENCY = FREQUENCY_RESOLUTION * DATA_XY_DIM / 2.f; // [Hz]
  const float HIGHPASS_FREQENCY = MAX_FREQUENCY * 0.8f; // [Hz]
  const float LOWPASS_FREQENCY = MAX_FREQUENCY * 0.1f; // [Hz]
  const float ALPHA = 0.01; // 0 = box, 1 = von Hann; changing this may make the test invalid
  const float EPSILON_FFT = 0.0001f;

public:

  void setUp() override
  {
    m_FilterService = mitk::PhotoacousticFilterService::New();
  }

  void test(float HighPass, float LowPass, float HighPassAlpha, float LowPassAlpha, bool useLow, bool useHigh)
  {
    std::random_device r;
    std::default_random_engine randGen(r());
    std::uniform_real_distribution<float> randDistrHighPass(HighPass * 0.01f, HighPass * 0.2f);
    std::uniform_real_distribution<float> randDistrLowPass(LowPass * 1.5f, LowPass * 2.f);
    float* data = new float[DATA_XY_DIM*DATA_XY_DIM*DATA_Z_DIM];

    mitk::Image::Pointer inputImage = mitk::Image::New();
    unsigned int dimension[3]{ DATA_XY_DIM, DATA_XY_DIM, DATA_Z_DIM };
    inputImage->Initialize(mitk::MakeScalarPixelType<float>(), 3, dimension);
    mitk::Vector3D spacing;
    spacing[0] = 1;
    spacing[1] = TIME_SPACING;
    spacing[2] = 1;
    inputImage->SetSpacing(spacing);

    for (unsigned int iteration = 0; iteration < NUM_ITERATIONS; ++iteration)
    {
      // fill image with zeros
      for (unsigned int i = 0; i < DATA_XY_DIM*DATA_XY_DIM*DATA_Z_DIM; ++i)
      {
        data[i] = 0;
      }

      // write specific frequencies to the image
      if (useHigh)
        addFrequency(randDistrHighPass(randGen), TIME_SPACING, data, dimension);
      if (useLow)
        addFrequency(randDistrLowPass(randGen), TIME_SPACING, data, dimension);

      inputImage->SetImportVolume(data, 0, 0, mitk::Image::ImportMemoryManagementType::CopyMemory);

      if (!useHigh)
        HighPass = 0;

      if (!useLow)
        LowPass = MAX_FREQUENCY;

      mitk::Image::Pointer outputImage = m_FilterService->ApplyBandpassFilter(inputImage, HighPass, LowPass, HighPassAlpha, LowPassAlpha, TIME_SPACING / 1e6, 0, false);

      // do a fourier transform, and check whether the part of the image that has been filtered is zero
      typedef itk::Image< float, 3 > RealImageType;
      RealImageType::Pointer image;

      mitk::CastToItkImage(outputImage, image);

      typedef itk::FFT1DRealToComplexConjugateImageFilter<RealImageType> ForwardFFTFilterType;
      // typedef ForwardFFTFilterType::OutputImageType ComplexImageType;
      ForwardFFTFilterType::Pointer forwardFFTFilter = ForwardFFTFilterType::New();
      forwardFFTFilter->SetInput(image);
      forwardFFTFilter->SetDirection(1);
      forwardFFTFilter->UpdateOutputInformation();
      forwardFFTFilter->Update();

      auto fftResult = forwardFFTFilter->GetOutput();

      /*
      std::string img = "D:/img" + std::to_string(iteration) + ".nrrd";
      mitk::IOUtil::Save(inputImage, img);
      std::string res = "D:/res" + std::to_string(iteration) + ".nrrd";
      mitk::IOUtil::Save(outputImage, res);*/

      // the resulting image should consist only of zeros, as we filtered the frequencies out
      for (unsigned int z = 0; z < DATA_Z_DIM; ++z)
      {
        for (unsigned int y = 0; y < DATA_XY_DIM / 2; ++y)
        {
          if (y < (unsigned int)std::floor(0.95 * HighPass / FREQUENCY_RESOLUTION) || y >(unsigned int)std::ceil(1.05 * LowPass / FREQUENCY_RESOLUTION))
          {
            for (unsigned int x = 0; x < DATA_XY_DIM; ++x)
            {
              // unsigned int outPos = x + y * DATA_XY_DIM + z * DATA_XY_DIM * DATA_XY_DIM;
              std::complex<float> value = fftResult->GetPixel({ x,y,z });
              CPPUNIT_ASSERT_MESSAGE(std::string("Expected 0, got (" + std::to_string(value.real()) + " + " + std::to_string(value.imag()) + "i) at " + std::to_string(x) + "-" + std::to_string(y) + "-" + std::to_string(z)),
                (value.real() < EPSILON_FFT) && (value.imag() < EPSILON_FFT));
            }
          }
        }

        for (unsigned int y = DATA_XY_DIM / 2; y < DATA_XY_DIM; ++y)
        {
          if (y > DATA_XY_DIM - (unsigned int)std::floor(HighPass / FREQUENCY_RESOLUTION) || y < DATA_XY_DIM - (unsigned int)std::ceil(LowPass / FREQUENCY_RESOLUTION))
          {
            for (unsigned int x = 0; x < DATA_XY_DIM; ++x)
            {
              // unsigned int outPos = x + y * DATA_XY_DIM + z * DATA_XY_DIM * DATA_XY_DIM;
              std::complex<float> value = fftResult->GetPixel({ x,y,z });
              CPPUNIT_ASSERT_MESSAGE(std::string("Expected 0, got (" + std::to_string(value.real()) + " + " + std::to_string(value.imag()) + "i) at " + std::to_string(x) + "-" + std::to_string(y) + "-" + std::to_string(z)),
                (value.real() < EPSILON_FFT) && (value.imag() < EPSILON_FFT));
            }
          }
        }
      }
    }

    delete[] data;
  }

  // write a fixed-frequency signal to the image
  void addFrequency(float freq, float timeSpacing, float* data, unsigned int* dim)
  {
    for (unsigned int z = 0; z < dim[2]; ++z)
    {
      for (unsigned int y = 0; y < dim[1]; ++y)
      {
        for (unsigned int x = 0; x < dim[0]; ++x)
        {
          data[x + y * dim[0] + z * dim[0] * dim[1]] += std::sin(freq * timeSpacing * y);
        }
      }
    }
  }

  void testHighPass()
  {
    MITK_INFO << "Performing HighPass test";
    test(HIGHPASS_FREQENCY, 0, ALPHA, ALPHA, false, true);
  }

  void testLowPass()
  {
    MITK_INFO << "Performing LowPass test";
    test(0, LOWPASS_FREQENCY, ALPHA, ALPHA, true, false);
  }

  void tearDown() override
  {
    m_FilterService = nullptr;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkBandpassFilter)
