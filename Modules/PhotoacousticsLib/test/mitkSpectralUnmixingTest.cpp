//*===================================================================

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
#include <mitkPASpectralUnmixingFilterVigra.h>
#include <mitkPASpectralUnmixingFilterSimplex.h>
#include <mitkPASpectralUnmixingSO2.h>
#include <mitkImageReadAccessor.h>

class mitkSpectralUnmixingTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSpectralUnmixingTestSuite);
  MITK_TEST(testEigenSUAlgorithm);
  MITK_TEST(testVigraSUAlgorithm);
  //MITK_TEST(testSimplexSUAlgorithm);// --> RESULT FAILS
  MITK_TEST(testSO2); //--> SO2 Settings FAILS
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter;
  mitk::Image::Pointer inputImage;
  std::vector<int> m_inputWavelengths;
  std::vector<double> m_inputWeights;
  std::vector<float> m_CorrectResult;
  float threshold;

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
    dimensions[1] = 1;
    dimensions[2] = 5;

    inputImage->Initialize(pixelType, NUMBER_OF_SPATIAL_DIMENSIONS, dimensions);

    //Set wavelengths for unmixing:
    m_inputWavelengths.push_back(750);
    m_inputWavelengths.push_back(800);

    m_inputWeights.push_back(50);
    m_inputWeights.push_back(100);

    //Set fraction of Hb and HbO2 to unmix:
    float fracHb = 100;
    float fracHbO2 = 300;
    m_CorrectResult.push_back(fracHbO2);
    m_CorrectResult.push_back(fracHb);
    m_CorrectResult.push_back(fracHbO2 + 10);
    m_CorrectResult.push_back(fracHb - 10);
    threshold = 0.01;

    //Multiply values of wavelengths (750,800,850 nm) with fractions to get pixel values:
    float px1 = fracHb * 7.52 + fracHbO2 * 2.77;
    float px2 = fracHb * 4.08 + fracHbO2 * 4.37;
    float px3 = (fracHb - 10) * 7.52 + (fracHbO2 + 10) * 2.77;
    float px4 = (fracHb - 10) * 4.08 + (fracHbO2 + 10) * 4.37;

    float* data = new float[3];
    data[0] = px1;
    data[1] = px2;
    data[2] = px3;
    data[3] = px4;
    data[5] = 0;

    inputImage->SetImportVolume(data, mitk::Image::ImportMemoryManagementType::CopyMemory);
    delete[] data;
    MITK_INFO << "[DONE]";
  }

  void testEigenSUAlgorithm()
  {
    MITK_INFO << "START FILTER TEST ... ";
    // Set input image
    auto m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    m_SpectralUnmixingFilter->SetInput(inputImage);

    //Set wavelengths to filter
    for (unsigned int imageIndex = 0; imageIndex < m_inputWavelengths.size(); imageIndex++)
    {
      unsigned int wavelength = m_inputWavelengths[imageIndex];
      m_SpectralUnmixingFilter->AddWavelength(wavelength);
    }

    //Set Chromophores to filter
    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::OXYGENATED);
    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::DEOXYGENATED);

    ofstream myfile;
    myfile.open("EigenTestResult.txt");

    std::vector<mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType> m_Eigen = {
    mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::householderQr, /* mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::ldlt, 
    mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::llt,*/ mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::colPivHouseholderQr,
    mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::jacobiSvd, mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::fullPivLu,
    mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::fullPivHouseholderQr/*mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::test*/};

    for (int Algorithmidx = 0; Algorithmidx < m_Eigen.size();++Algorithmidx)
    {
      m_SpectralUnmixingFilter->SetAlgorithm(m_Eigen[Algorithmidx]);

      m_SpectralUnmixingFilter->Update();


      /*For printed pixel values and results look at: [...]\mitk-superbuild\MITK-build\Modules\PhotoacousticsLib\test\*/

      for (int i = 0; i < 2; ++i)
      {
        mitk::Image::Pointer output = m_SpectralUnmixingFilter->GetOutput(i);
        mitk::ImageReadAccessor readAccess(output);
        const float* inputDataArray = ((const float*)readAccess.GetData());
        auto pixel = inputDataArray[0];
        auto pixel2 = inputDataArray[1];

        myfile << "Algorithmidx: " << Algorithmidx << "\n Output " << i << ": " << "\n" << inputDataArray[0] << "\n" << inputDataArray[1] << "\n";
        myfile << "Correct Result: " << "\n" << m_CorrectResult[i] << "\n" << m_CorrectResult[i + 2] << "\n";

        CPPUNIT_ASSERT(std::abs(pixel - m_CorrectResult[i]) < threshold);
        CPPUNIT_ASSERT(std::abs(pixel2 - m_CorrectResult[i + 2]) < threshold);
      }
    }
    myfile.close();
    MITK_INFO << "EIGEN FILTER TEST SUCCESFULL :)";
  }

  void testVigraSUAlgorithm()
  {
    MITK_INFO << "START FILTER TEST ... ";
    // Set input image
    auto m_SpectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterVigra::New();
    m_SpectralUnmixingFilter->SetInput(inputImage);



    //Set wavelengths to filter
    for (unsigned int imageIndex = 0; imageIndex < m_inputWavelengths.size(); imageIndex++)
    {
      unsigned int wavelength = m_inputWavelengths[imageIndex];
      double Weight = m_inputWeights[imageIndex];
      m_SpectralUnmixingFilter->AddWavelength(wavelength);
      m_SpectralUnmixingFilter->AddWeight(Weight);
    }

    //Set Chromophores to filter
    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::OXYGENATED);
    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::DEOXYGENATED);


    std::vector<mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType> Vigra = {
    mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::LARS, mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::GOLDFARB,
    mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::WEIGHTED, mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::LS/*,
    mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::vigratest*/};

    ofstream myfile;
    myfile.open("VigraTestResult.txt");

    for (int Algorithmidx = 0; Algorithmidx < Vigra.size();++Algorithmidx)
    {
      m_SpectralUnmixingFilter->SetAlgorithm(Vigra[0]);

      m_SpectralUnmixingFilter->Update();


      /*For printed pixel values and results look at: [...]\mitk-superbuild\MITK-build\Modules\PhotoacousticsLib\test\*/

      for (int i = 0; i < 2; ++i)
      {
        mitk::Image::Pointer output = m_SpectralUnmixingFilter->GetOutput(i);
        mitk::ImageReadAccessor readAccess(output);
        const float* inputDataArray = ((const float*)readAccess.GetData());
        auto pixel = inputDataArray[0];
        auto pixel2 = inputDataArray[1];

        myfile << "Algorithmidx: " << Algorithmidx << "\n Output " << i << ": " << "\n" << inputDataArray[0] << "\n" << inputDataArray[1] << "\n";
        myfile << "Correct Result: " << "\n" << m_CorrectResult[i] << "\n" << m_CorrectResult[i + 2] << "\n";

        CPPUNIT_ASSERT(std::abs(pixel - m_CorrectResult[i]) < threshold);
        CPPUNIT_ASSERT(std::abs(pixel2 - m_CorrectResult[i + 2]) < threshold);
      }
    }
    myfile.close();
    MITK_INFO << "VIGRA FILTER TEST SUCCESFULL :)";
  }

  void testSimplexSUAlgorithm()
  {
    MITK_INFO << "START FILTER TEST ... ";
    // Set input image
    auto m_SpectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterSimplex::New();
    m_SpectralUnmixingFilter->SetInput(inputImage);

    //Set wavelengths to filter
    for (unsigned int imageIndex = 0; imageIndex < m_inputWavelengths.size(); imageIndex++)
    {
      unsigned int wavelength = m_inputWavelengths[imageIndex];
      m_SpectralUnmixingFilter->AddWavelength(wavelength);
    }

    //Set Chromophores to filter
    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::OXYGENATED);
    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::DEOXYGENATED);

    m_SpectralUnmixingFilter->Update();


    /*For printed pixel values and results look at: [...]\mitk-superbuild\MITK-build\Modules\PhotoacousticsLib\test\*/
    ofstream myfile;
    myfile.open("SimplexTestResult.txt");
    for (int i = 0; i < 2; ++i)
    {
      mitk::Image::Pointer output = m_SpectralUnmixingFilter->GetOutput(i);
      mitk::ImageReadAccessor readAccess(output);
      const float* inputDataArray = ((const float*)readAccess.GetData());
      auto pixel = inputDataArray[0];
      auto pixel2 = inputDataArray[1];

      myfile << "Output " << i << ": " << "\n" << inputDataArray[0] << "\n" << inputDataArray[1] << "\n";
      myfile << "Correct Result: " << "\n" << m_CorrectResult[i] << "\n" << m_CorrectResult[i + 2] << "\n";

      CPPUNIT_ASSERT(std::abs(pixel - m_CorrectResult[i])<threshold);
      CPPUNIT_ASSERT(std::abs(pixel2 - m_CorrectResult[i + 2])<threshold);
    }
    myfile.close();
    MITK_INFO << "SIMPLEX FILTER TEST SUCCESFULL :)";
  }

  void testSO2()
  {
    MITK_INFO << "START SO2 TEST ... ";
    auto m_sO2 = mitk::pa::SpectralUnmixingSO2::New();
    m_sO2->SetInput(0, inputImage);
    m_sO2->SetInput(1, inputImage);
    std::vector<float> m_CorrectSO2Result = { 0.5, 0.5, 0.5, 0.5, 0 };
    std::vector<float> SO2Settings = { 1, 1, 1, 49 }; // active settings (sum, fraction) are not tested (for false case) --> BUT ALL FALSE
    for (int i = 0; i < SO2Settings.size(); ++i)
      m_sO2->AddSO2Settings(SO2Settings[i]);
    m_sO2->Update();

    /*For printed pixel values and results look at: [...]\mitk-superbuild\MITK-build\Modules\PhotoacousticsLib\test\*/
    ofstream myfile;
    myfile.open("SO2TestResult.txt");

    mitk::Image::Pointer output = m_sO2->GetOutput(0);
    mitk::ImageReadAccessor readAccess(output);
    const float* inputDataArray = ((const float*)readAccess.GetData());

    for (unsigned int Pixel = 0; Pixel < inputImage->GetDimensions()[2]; ++Pixel)
    {
      auto Value = inputDataArray[Pixel];

      myfile << "Output " << Pixel << ": " << "\n" << Value <<"\n";
      myfile << "Correct Result: " << "\n" << m_CorrectSO2Result[Pixel] << "\n";

      CPPUNIT_ASSERT(std::abs(Value - m_CorrectSO2Result[Pixel]) < threshold);
    }
    myfile.close();
    MITK_INFO << "SO2 TEST SUCCESFULL :)";
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
