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
#include <mitkCropImageFilter.h>
#include <random>

class mitkCropImageFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkCropImageFilterTestSuite);
  MITK_TEST(testCropImage);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::CropImageFilter::Pointer m_CropImageFilter;
  const unsigned int NUM_ITERATIONS = 5;
  const unsigned int DATA_DIM = 25;

public:

  void setUp() override
  {
    m_CropImageFilter = mitk::CropImageFilter::New();
  }

  void test()
  {
    std::random_device r;
    std::default_random_engine randGen(r());
    std::uniform_int_distribution<int> randDistr(0, (DATA_DIM / 2) - 1);

    float* data = new float[DATA_DIM*DATA_DIM*DATA_DIM];

    for (unsigned int i = 0; i < DATA_DIM*DATA_DIM*DATA_DIM; ++i)
    {
      data[i] = (float)i;
    }

    mitk::Image::Pointer inputImage = mitk::Image::New();
    unsigned int dimension[3]{ DATA_DIM, DATA_DIM, DATA_DIM };
    inputImage->Initialize(mitk::MakeScalarPixelType<float>(), 3, dimension);
    inputImage->SetImportVolume(data, 0, 0, mitk::Image::ImportMemoryManagementType::CopyMemory);

    for (unsigned int iteration = 0; iteration < NUM_ITERATIONS; ++iteration)
    {
      unsigned int XPixelsCropStart = randDistr(randGen);
      unsigned int YPixelsCropStart = randDistr(randGen);
      unsigned int ZPixelsCropStart = randDistr(randGen);
      unsigned int XPixelsCropEnd = randDistr(randGen);
      unsigned int YPixelsCropEnd = randDistr(randGen);
      unsigned int ZPixelsCropEnd = randDistr(randGen);

      unsigned int newXDim = DATA_DIM - XPixelsCropStart - XPixelsCropEnd;
      unsigned int newYDim = DATA_DIM - YPixelsCropStart - YPixelsCropEnd;
      unsigned int newZDim = DATA_DIM - ZPixelsCropStart - ZPixelsCropEnd;

      m_CropImageFilter->SetInput(inputImage);

      m_CropImageFilter->SetXPixelsCropStart(XPixelsCropStart);
      m_CropImageFilter->SetYPixelsCropStart(YPixelsCropStart);
      m_CropImageFilter->SetZPixelsCropStart(ZPixelsCropStart);
      m_CropImageFilter->SetXPixelsCropEnd(XPixelsCropEnd);
      m_CropImageFilter->SetYPixelsCropEnd(YPixelsCropEnd);
      m_CropImageFilter->SetZPixelsCropEnd(ZPixelsCropEnd);

      m_CropImageFilter->Update();
      mitk::Image::Pointer outputImage = m_CropImageFilter->GetOutput();

      mitk::ImageReadAccessor readAccess(outputImage);
      const float* outputData = (const float*)readAccess.GetData();

      CPPUNIT_ASSERT_MESSAGE(std::string("expected x size to be " + std::to_string(newXDim) + " but was " + std::to_string(outputImage->GetDimension(0))), newXDim == outputImage->GetDimension(0));
      CPPUNIT_ASSERT_MESSAGE(std::string("expected y size to be " + std::to_string(newYDim) + " but was " + std::to_string(outputImage->GetDimension(1))), newYDim == outputImage->GetDimension(1));
      CPPUNIT_ASSERT_MESSAGE(std::string("expected z size to be " + std::to_string(newZDim) + " but was " + std::to_string(outputImage->GetDimension(2))), newZDim == outputImage->GetDimension(2));

      for (unsigned int z = 0; z < newZDim; ++z)
      {
        for (unsigned int y = 0; y < newYDim; ++y)
        {
          for (unsigned int x = 0; x < newXDim; ++x)
          {
            unsigned int origPos = (x + XPixelsCropStart) + (y + YPixelsCropStart) * DATA_DIM + (z + ZPixelsCropStart) * DATA_DIM * DATA_DIM;
            unsigned int outPos = x + y * newXDim + z * newXDim * newYDim;
            CPPUNIT_ASSERT_MESSAGE(std::string("expected " + std::to_string(data[origPos])
              + " but was " + std::to_string(outputData[outPos])),
              std::abs((float)outputData[outPos] - (float)data[origPos]) < mitk::eps);
          }
        }
      }
    }

    delete[] data;
  }

  void testCropImage()
  {
    for (int repetition = 0; repetition < 20; ++repetition)
    {
      MITK_INFO << "[" << (repetition + 1) << "/20]";
      test();
    }
  }

  void tearDown() override
  {
    m_CropImageFilter = nullptr;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkCropImageFilter)
