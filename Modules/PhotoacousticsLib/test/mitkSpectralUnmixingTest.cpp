/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
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
  //MITK_TEST(testSimplexSUAlgorithm);
  MITK_TEST(testSO2);
  MITK_TEST(testExceptionSO2);
  MITK_TEST(testWavelengthExceptions);
  MITK_TEST(testNoChromophoresSelected);
  MITK_TEST(testInputImage);
  MITK_TEST(testAddOutput);
  MITK_TEST(testWeightsError);
  MITK_TEST(testOutputs);
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

    float* data = new float[6];
    data[0] = px1;
    data[1] = px2;
    data[2] = px3;
    data[3] = px4;
    data[5] = 0;

    inputImage->SetImportVolume(data, mitk::Image::ImportMemoryManagementType::CopyMemory);
    delete[] data;
  }

  // Tests implemented EIGEN algortihms with correct inputs
  void testEigenSUAlgorithm()
  {
    MITK_INFO << "START FILTER TEST ... ";
    // Set input image
    auto m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    m_SpectralUnmixingFilter->Verbose(false);
    m_SpectralUnmixingFilter->RelativeError(false);
    m_SpectralUnmixingFilter->SetInput(inputImage);
    m_SpectralUnmixingFilter->AddOutputs(2);

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
    mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::HOUSEHOLDERQR, /* mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::LDLT,
    mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::LLT,*/ mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::COLPIVHOUSEHOLDERQR,
    mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::JACOBISVD, mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::FULLPIVLU,
    mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::FULLPIVHOUSEHOLDERQR};

    for (unsigned int Algorithmidx = 0; Algorithmidx < m_Eigen.size();++Algorithmidx)
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

  // Tests implemented VIGRA algortihms with correct inputs
  void testVigraSUAlgorithm()
  {
    MITK_INFO << "START FILTER TEST ... ";
    // Set input image
    auto m_SpectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterVigra::New();
    m_SpectralUnmixingFilter->SetInput(inputImage);
    m_SpectralUnmixingFilter->AddOutputs(2);
    m_SpectralUnmixingFilter->Verbose(false);
    m_SpectralUnmixingFilter->RelativeError(false);


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

    for (unsigned int Algorithmidx = 0; Algorithmidx < Vigra.size();++Algorithmidx)
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
    m_SpectralUnmixingFilter->AddOutputs(2);
    m_SpectralUnmixingFilter->Verbose(true);
    m_SpectralUnmixingFilter->RelativeError(false);

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

  // Tests SO2 Filter with unequal inputs
  void testExceptionSO2()
  {
    MITK_INFO << "START EXCEPTION SO2 TEST ... ";

    auto m_sO2 = mitk::pa::SpectralUnmixingSO2::New();
    m_sO2->SetInput(0, inputImage);

    inputImage = nullptr;
    inputImage = mitk::Image::New();
    mitk::PixelType pixelType = mitk::MakeScalarPixelType<float>();
    const int NUMBER_OF_SPATIAL_DIMENSIONS = 3;
    auto* dimensions = new unsigned int[NUMBER_OF_SPATIAL_DIMENSIONS];

    dimensions[0] = 1;
    dimensions[1] = 1;
    dimensions[2] = 4;

    inputImage->Initialize(pixelType, NUMBER_OF_SPATIAL_DIMENSIONS, dimensions);

    float* data = new float[4];
    data[0] = 1;
    data[1] = 2;
    data[2] = 3;
    data[3] = 4;

    inputImage->SetImportVolume(data, mitk::Image::ImportMemoryManagementType::CopyMemory);
    delete[] data;

    m_sO2->SetInput(1, inputImage);

    MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)
      m_sO2->Update();
    MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)
  }

  // Tests SO2 Filter with correct inputs
  void testSO2()
  {
    MITK_INFO << "START SO2 TEST ... ";
    std::vector<float> CorrectSO2Result1 = { 0, 0, 0, 0, 0 };
    std::vector<float> Test1 = { 0,0,0,51 };
    std::vector<float> CorrectSO2Result2 = { 0, 0.5, 0, 0.5, 0 };
    std::vector<float> Test2 = { 1584, 0, 0, 0 };
    std::vector<float> CorrectSO2Result3 = { 0.5, 0.5, 0, 0.5, 0 };
    std::vector<float> Test3 = { 0, 1536, 0, 0 };
    std::vector<float> CorrectSO2Result4 = { 0.5, 0.5, 0, 0.5, 0 };
    std::vector<float> Test4 = { 0, 0, 3072, 49 };
    std::vector<float> CorrectSO2Result5 = { 0.5, 0.5, 0.5, 0.5, 0 };
    std::vector<float> Test5 = { 1, 1, 1, 49 };

    std::vector<std::vector<float>> TestList;
    std::vector<std::vector<float>> ResultList;

    TestList.push_back(Test1);
    TestList.push_back(Test2);
    TestList.push_back(Test3);
    TestList.push_back(Test4);
    TestList.push_back(Test5);

    ResultList.push_back(CorrectSO2Result1);
    ResultList.push_back(CorrectSO2Result2);
    ResultList.push_back(CorrectSO2Result3);
    ResultList.push_back(CorrectSO2Result4);
    ResultList.push_back(CorrectSO2Result5);

    /*For printed pixel values and results look at: [...]\mitk-superbuild\MITK-build\Modules\PhotoacousticsLib\test\*/
    ofstream myfile;
    myfile.open("SO2TestResult.txt");

    for (int k = 0; k < 5; ++k)
    {
      std::vector<float> SO2Settings = TestList[k];
      std::vector<float> m_CorrectSO2Result = ResultList[k];
      auto m_sO2 = mitk::pa::SpectralUnmixingSO2::New();
      m_sO2->SetInput(0, inputImage);
      m_sO2->SetInput(1, inputImage);
      for (unsigned int i = 0; i < SO2Settings.size(); ++i)
        m_sO2->AddSO2Settings(SO2Settings[i]);
      m_sO2->Update();

      mitk::Image::Pointer output = m_sO2->GetOutput(0);
      mitk::ImageReadAccessor readAccess(output);
      const float* inputDataArray = ((const float*)readAccess.GetData());

      for (unsigned int Pixel = 0; Pixel < inputImage->GetDimensions()[2]; ++Pixel)
      {
        auto Value = inputDataArray[Pixel];

        myfile << "Output(Test " << k << ") " << Pixel << ": " << "\n" << Value << "\n";
        myfile << "Correct Result: " << "\n" << m_CorrectSO2Result[Pixel] << "\n";

        CPPUNIT_ASSERT(std::abs(Value - m_CorrectSO2Result[Pixel]) < threshold);
      }
    }
    myfile.close();
    MITK_INFO << "SO2 TEST SUCCESFULL :)";
  }

  // Test exceptions for wrong wavelength inputs
  void testWavelengthExceptions()
  {
    MITK_INFO << "START WavelengthExceptions TEST ... ";
    // Set input image
    auto m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    m_SpectralUnmixingFilter->Verbose(false);
    m_SpectralUnmixingFilter->RelativeError(false);
    m_SpectralUnmixingFilter->SetInput(inputImage);
    m_SpectralUnmixingFilter->AddOutputs(2);

    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::OXYGENATED);
    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::DEOXYGENATED);

    m_SpectralUnmixingFilter->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::HOUSEHOLDERQR);

    MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)
      m_SpectralUnmixingFilter->Update();
    MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)

    m_SpectralUnmixingFilter->AddWavelength(300);

    MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)
      m_SpectralUnmixingFilter->Update();
    MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)

    m_SpectralUnmixingFilter->AddWavelength(299);

    MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)
      m_SpectralUnmixingFilter->Update();
    MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)
    MITK_INFO << "DONE";
  }

  // Test exceptions for wrong chromophore inputs
  void testNoChromophoresSelected()
  {
    MITK_INFO << "testNoChromophoresSelected";
    // Set input image
    auto m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    m_SpectralUnmixingFilter->Verbose(false);
    m_SpectralUnmixingFilter->RelativeError(false);
    m_SpectralUnmixingFilter->SetInput(inputImage);
    m_SpectralUnmixingFilter->AddOutputs(2);

    //Set wavelengths to filter
    for (unsigned int imageIndex = 0; imageIndex < m_inputWavelengths.size(); imageIndex++)
    {
      unsigned int wavelength = m_inputWavelengths[imageIndex];
      m_SpectralUnmixingFilter->AddWavelength(wavelength);
    }

    m_SpectralUnmixingFilter->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::HOUSEHOLDERQR);

    MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)
      m_SpectralUnmixingFilter->Update();
    MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)
  }

  // Test exceptions for wrong input image
  void testInputImage()
  {
    MITK_INFO << "INPUT IMAGE TEST";
    inputImage = nullptr;
    // Set input image
    auto m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    m_SpectralUnmixingFilter->Verbose(false);
    m_SpectralUnmixingFilter->RelativeError(false);
    //m_SpectralUnmixingFilter->SetInput(inputImage);
    m_SpectralUnmixingFilter->AddOutputs(2);

    //Set wavelengths to filter
    for (unsigned int imageIndex = 0; imageIndex < m_inputWavelengths.size(); imageIndex++)
    {
      unsigned int wavelength = m_inputWavelengths[imageIndex];
      m_SpectralUnmixingFilter->AddWavelength(wavelength);
    }

    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::OXYGENATED);
    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::DEOXYGENATED);

    m_SpectralUnmixingFilter->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::HOUSEHOLDERQR);

    MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)
      m_SpectralUnmixingFilter->Update();
    MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)

    inputImage = mitk::Image::New();
    mitk::PixelType pixelType = mitk::MakeScalarPixelType<double>();
    const int NUMBER_OF_SPATIAL_DIMENSIONS = 3;
    auto* dimensions = new unsigned int[NUMBER_OF_SPATIAL_DIMENSIONS];

    dimensions[0] = 1;
    dimensions[1] = 1;
    dimensions[2] = 5;

    inputImage->Initialize(pixelType, NUMBER_OF_SPATIAL_DIMENSIONS, dimensions);

    double* data = new double[6];
    data[0] = 1;
    data[1] = 2;
    data[2] = 3;
    data[3] = 4;
    data[5] = 0;

    inputImage->SetImportVolume(data, mitk::Image::ImportMemoryManagementType::CopyMemory);
    delete[] data;

    m_SpectralUnmixingFilter->SetInput(inputImage);

    MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)
      m_SpectralUnmixingFilter->Update();
    MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)
  }

  // Test exceptions for addOutputs method
  void testAddOutput()
  {
    MITK_INFO << "addOutputs TEST";

    // Set input image
    auto m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    m_SpectralUnmixingFilter->Verbose(false);
    m_SpectralUnmixingFilter->RelativeError(false);
    m_SpectralUnmixingFilter->SetInput(inputImage);

    //Set wavelengths to filter
    for (unsigned int imageIndex = 0; imageIndex < m_inputWavelengths.size(); imageIndex++)
    {
      unsigned int wavelength = m_inputWavelengths[imageIndex];
      m_SpectralUnmixingFilter->AddWavelength(wavelength);
    }

    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::OXYGENATED);
    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::DEOXYGENATED);

    m_SpectralUnmixingFilter->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::HOUSEHOLDERQR);

    for (int i = 0; i < 4; ++i)
    {
      MITK_INFO << "i: " << i;
      if (i != 2)
      {
        MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)
          m_SpectralUnmixingFilter->AddOutputs(i);
          m_SpectralUnmixingFilter->Update();
        MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)
      }
      else
      {
        m_SpectralUnmixingFilter->AddOutputs(2);
        m_SpectralUnmixingFilter->Update();
        for (int i = 0; i < 2; ++i)
        {
          mitk::Image::Pointer output = m_SpectralUnmixingFilter->GetOutput(i);
          mitk::ImageReadAccessor readAccess(output);
          const float* inputDataArray = ((const float*)readAccess.GetData());
          auto pixel = inputDataArray[0];
          auto pixel2 = inputDataArray[1];

          CPPUNIT_ASSERT(std::abs(pixel - m_CorrectResult[i]) < threshold);
          CPPUNIT_ASSERT(std::abs(pixel2 - m_CorrectResult[i + 2]) < threshold);
        }
      }
    }
  }

  // Test exceptions for weights error
  void testWeightsError()
  {
    MITK_INFO << "testWeightsError";

    // Set input image
    auto m_SpectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterVigra::New();
    m_SpectralUnmixingFilter->Verbose(false);
    m_SpectralUnmixingFilter->RelativeError(false);
    m_SpectralUnmixingFilter->SetInput(inputImage);
    m_SpectralUnmixingFilter->AddOutputs(2);

    //Set wavelengths to filter
    for (unsigned int imageIndex = 0; imageIndex < m_inputWavelengths.size(); imageIndex++)
    {
      unsigned int wavelength = m_inputWavelengths[imageIndex];
      m_SpectralUnmixingFilter->AddWavelength(wavelength);
    }

    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::OXYGENATED);
    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::DEOXYGENATED);

    m_SpectralUnmixingFilter->SetAlgorithm(mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::WEIGHTED);

    MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)
    m_SpectralUnmixingFilter->Update();
    MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)

    m_SpectralUnmixingFilter->AddWeight(50);
    MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)
    m_SpectralUnmixingFilter->Update();
    MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)

    m_SpectralUnmixingFilter->AddWeight(50);
    m_SpectralUnmixingFilter->Update();

    for (int i = 0; i < 2; ++i)
    {
      mitk::Image::Pointer output = m_SpectralUnmixingFilter->GetOutput(i);
      mitk::ImageReadAccessor readAccess(output);
      const float* inputDataArray = ((const float*)readAccess.GetData());
      auto pixel = inputDataArray[0];
      auto pixel2 = inputDataArray[1];

      CPPUNIT_ASSERT(std::abs(pixel - m_CorrectResult[i]) < threshold);
      CPPUNIT_ASSERT(std::abs(pixel2 - m_CorrectResult[i + 2]) < threshold);
    }
  }

  // Test correct outputs
  void testOutputs()
  {
    MITK_INFO << "TEST";

    // Set input image
    auto m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    m_SpectralUnmixingFilter->Verbose(false);
    m_SpectralUnmixingFilter->RelativeError(false);
    m_SpectralUnmixingFilter->SetInput(inputImage);
    m_SpectralUnmixingFilter->AddOutputs(2);

    //Set wavelengths to filter
    for (unsigned int imageIndex = 0; imageIndex < m_inputWavelengths.size(); imageIndex++)
    {
      unsigned int wavelength = m_inputWavelengths[imageIndex];
      m_SpectralUnmixingFilter->AddWavelength(wavelength);
    }

    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::OXYGENATED);
    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::DEOXYGENATED);

    m_SpectralUnmixingFilter->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::HOUSEHOLDERQR);

    m_SpectralUnmixingFilter->Update();

    for (int i = 0; i < 2; ++i)
    {
      mitk::Image::Pointer output = m_SpectralUnmixingFilter->GetOutput(i);
      mitk::ImageReadAccessor readAccess(output);
      const float* inputDataArray = ((const float*)readAccess.GetData());
      auto pixel = inputDataArray[0];
      auto pixel2 = inputDataArray[1];

      CPPUNIT_ASSERT(std::abs(pixel - m_CorrectResult[i]) < threshold);
      CPPUNIT_ASSERT(std::abs(pixel2 - m_CorrectResult[i + 2]) < threshold);

      // test correct output dimensions and pixel type
      CPPUNIT_ASSERT(inputImage->GetDimensions()[0] == output->GetDimensions()[0]);
      CPPUNIT_ASSERT(inputImage->GetDimensions()[0] == output->GetDimensions()[1]);
      CPPUNIT_ASSERT(2 == output->GetDimensions()[2]);
      CPPUNIT_ASSERT(output->GetPixelType() == mitk::MakeScalarPixelType<float>());
    }
  }

  // TEST TEMPLATE:
  /*
  // Test exceptions for
  void test()
  {
    MITK_INFO << "TEST";

    // Set input image
    auto m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    m_SpectralUnmixingFilter->Verbose(false);
    m_SpectralUnmixingFilter->RelativeError(false);
    m_SpectralUnmixingFilter->SetInput(inputImage);
    m_SpectralUnmixingFilter->AddOutputs(2);

    //Set wavelengths to filter
    for (unsigned int imageIndex = 0; imageIndex < m_inputWavelengths.size(); imageIndex++)
    {
      unsigned int wavelength = m_inputWavelengths[imageIndex];
      m_SpectralUnmixingFilter->AddWavelength(wavelength);
    }

    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::OXYGENATED);
    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::PropertyCalculator::ChromophoreType::DEOXYGENATED);

    m_SpectralUnmixingFilter->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::HOUSEHOLDERQR);

    //MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ExceptionObject)
      m_SpectralUnmixingFilter->Update();
    //MITK_TEST_FOR_EXCEPTION_END(itk::ExceptionObject)

  }*/

  void tearDown() override
  {
    m_SpectralUnmixingFilter = nullptr;
    inputImage = nullptr;
    m_inputWavelengths.clear();
    m_CorrectResult.clear();
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSpectralUnmixing)
