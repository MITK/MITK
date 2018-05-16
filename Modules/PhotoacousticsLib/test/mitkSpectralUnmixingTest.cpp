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
  mitk::Image::Pointer inputImage;
  std::vector<float> m_inputWavelengths;
  std::vector<float> m_CorrectResult;

public:

  void setUp() override
  {
    MITK_INFO << "setUp ... ";
    //Set empty input image:
    inputImage = mitk::Image::New();
    mitk::PixelType pixelType = mitk::MakeScalarPixelType<float>();
    const int NUMBER_OF_SPATIAL_DIMENSIONS = 3;
    auto* dimensions = new unsigned int[NUMBER_OF_SPATIAL_DIMENSIONS];

    dimensions[0] = 1;
    dimensions[0] = 1;
    dimensions[2] = 3;

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
      m_Value.push_back(
        fracHb * (m_PropertyCalculator->GetAbsorptionForWavelength(
          mitk::pa::PropertyCalculator::MapType::OXYGENATED, m_inputWavelengths[j]))
        + fracHbO2 * (m_PropertyCalculator->GetAbsorptionForWavelength(
          mitk::pa::PropertyCalculator::MapType::DEOXYGENATED, m_inputWavelengths[j])));
    }

    float* data = new(float[3]);
    data[0] = m_Value[0];
    data[1] = m_Value[1];
    data[2] = m_Value[2];

    inputImage->SetImportVolume(data);
    delete[] data;

    //Set input into filter
    auto m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    m_SpectralUnmixingFilter->SetInput(inputImage);

    // Set Algortihm to filter
    int SetAlgorithmIndex = mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::householderQr;
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

    MITK_INFO << "[DONE]";
  }

  void testSUAlgorithm()
  {
    MITK_INFO << "START FILTER TEST ... ";

    //compare filter result (output) with theoretical result
    float threshold = 1e-5;
    for (int i = 0; i < 2; ++i)
    {
      mitk::Image::Pointer output = m_SpectralUnmixingFilter->GetOutput(i);

      mitk::ImageReadAccessor readAccess(output);

      const float* inputDataArray = ((const float*)readAccess.GetData());

      auto pixel = inputDataArray[0];
      CPPUNIT_ASSERT((pixel - m_CorrectResult[i])<threshold);
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
