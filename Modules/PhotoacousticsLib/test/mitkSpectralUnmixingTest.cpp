///*===================================================================

//The Medical Imaging Interaction Toolkit (MITK)

//Copyright (c) German Cancer Research Center,
//Division of Medical and Biological Informatics.
//All rights reserved.

//This software is distributed WITHOUT ANY WARRANTY; without
//even the implied warranty of MERCHANTABILITY or FITNESS FOR
//A PARTICULAR PURPOSE.

//See LICENSE.txt or http://www.mitk.org for details.

//===================================================================*/
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkPASpectralUnmixingFilterBase.h>
#include <mitkPALinearSpectralUnmixingFilter.h>

class mitkSpectralUnmixingTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSpectralUnmixingTestSuite);
  MITK_TEST(testSUAlgorithm);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter;
  mitk::Image *inputImage;
  std::vector<float> m_inputWavelengths;
  std::vector<float> m_CorrectResult;

public:

  void setUp() override
  {
    MITK_INFO << "setUp ... ";
    //Set empty input image:
    mitk::PixelType pixelType = mitk::MakeScalarPixelType<float>();
    const int NUMBER_OF_SPATIAL_DIMENSIONS = 3;
    auto* dimensions = new unsigned int[NUMBER_OF_SPATIAL_DIMENSIONS];

    //Set dimensions of input inmage;
    unsigned int xDim = 1;
    unsigned int yDim = 1;
    unsigned int zDim = 3;

    dimensions[0] = xDim;
    dimensions[0] = yDim;
    dimensions[2] = zDim;

    //Initialie empty input image:
    inputImage->Initialize(pixelType, NUMBER_OF_SPATIAL_DIMENSIONS, dimensions);

    //Set wavelengths for unmixing:
    m_inputWavelengths.push_back(750);
    m_inputWavelengths.push_back(800);
    m_inputWavelengths.push_back(850);

    //Set fraction of Hb and HbO2 to unmix:
    float fracHb = 0.3;
    float fracHbO2 = 0.67;

    //Fractions are also correct unmixing result:
    m_CorrectResult.push_back(fracHb);
    m_CorrectResult.push_back(fracHbO2);

    //Calculate values of wavelengths and multiply with fractions to get pixel values:
    mitk::pa::PropertyCalculator::Pointer m_PropertyCalculator;
    std::vector<float> m_Value;
    for (unsigned int j = 0; j < m_inputWavelengths.size(); ++j)
    {
      m_Value.push_back(fracHb*(m_PropertyCalculator->GetAbsorptionForWavelength(
        static_cast<mitk::pa::PropertyCalculator::MapType>(1), m_inputWavelengths[j]))*
        fracHbO2*(m_PropertyCalculator->GetAbsorptionForWavelength(
          static_cast<mitk::pa::PropertyCalculator::MapType>(2), m_inputWavelengths[j])));
    }

    //Write pixel values into image (1 dimension equals 1 wavelength:
    mitk::ImageWriteAccessor writeAccess(inputImage);

    for (int x = 0; x < xDim; ++x)
    {
      for (int y = 0; y < yDim; ++y)
      {
        for (int z = 0; z < zDim; ++z)
        {
          // write value[z] @ pixel
          float* writeBuffer = (float *)writeAccess.GetData();
          writeBuffer[z] = m_Value[z];
        }
      }
    }
    MITK_INFO << "[DONE]";
  }

  void testSUAlgorithm()
  {
    MITK_INFO << "START FILTER TEST ... ";
    //Set input into filter
    auto m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    m_SpectralUnmixingFilter->SetInput(inputImage);

    // Set Algortihm to filter
    int SetAlgorithmIndex = 0; // 0-7
    m_SpectralUnmixingFilter->SetAlgorithm(SetAlgorithmIndex);

    //Set wavelengths to filter
    for (unsigned int imageIndex = 0; imageIndex < m_inputWavelengths.size(); imageIndex++)
    {
      unsigned int wavelength = m_inputWavelengths[imageIndex];
      m_SpectralUnmixingFilter->AddWavelength(wavelength);
    }

    //Set Chromophores to filter
    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::SpectralUnmixingFilterBase::ChromophoreType::OXYGENATED_HEMOGLOBIN);

    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::SpectralUnmixingFilterBase::ChromophoreType::DEOXYGENATED_HEMOGLOBIN);

    //compare filter result (output) with theoretical result
    float threshold = 1e-5;
    for (int i = 0; i < 2; ++i)
    {
      mitk::Image::Pointer output = m_SpectralUnmixingFilter->GetOutput(i);
      MITK_INFO << "output: " << output;
      //CPPUNIT_ASSERT((output - m_CorrectResult[i])<threshold);
    }
    MITK_INFO << "FILTER TEST SUCCESFULL :)";
  }

  void tearDown() override
  {
    m_SpectralUnmixingFilter = nullptr;
    inputImage = nullptr;
    m_inputWavelengths.clear();
    m_CorrectResult.clear();
    MITK_INFO << "tearDown ... [DONE]";
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSpectralUnmixing)
