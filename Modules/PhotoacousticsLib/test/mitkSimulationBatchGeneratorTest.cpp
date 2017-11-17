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

#include <mitkPASimulationBatchGenerator.h>
#include <mitkPAVolume.h>
#include <itkFileTools.h>

class mitkSimulationBatchGeneratorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSimulationBatchGeneratorTestSuite);
  MITK_TEST(testGenerateBatchFileString);
  MITK_TEST(testGenerateBatchFileAndSaveFile);
  CPPUNIT_TEST_SUITE_END();

private:

  const std::string TEST_FOLDER_PATH = "testFiles/";
  mitk::pa::SimulationBatchGeneratorParameters::Pointer m_Parameters;
  mitk::pa::Volume::Pointer m_Test3DVolume;

public:

  void setUp() override
  {
    m_Parameters = mitk::pa::SimulationBatchGeneratorParameters::New();
    m_Parameters->SetBinaryPath("binary");
    m_Parameters->SetNrrdFilePath(TEST_FOLDER_PATH);
    m_Parameters->SetNumberOfPhotons(100);
    m_Parameters->SetTissueName("tissueName");
    m_Parameters->SetVolumeIndex(0);
    m_Parameters->SetYOffsetLowerThresholdInCentimeters(-1);
    m_Parameters->SetYOffsetUpperThresholdInCentimeters(1);
    m_Parameters->SetYOffsetStepInCentimeters(0.5);

    m_Test3DVolume = createTest3DVolume(5);
    itk::FileTools::CreateDirectory(TEST_FOLDER_PATH);
    CPPUNIT_ASSERT(itksys::SystemTools::FileIsDirectory(TEST_FOLDER_PATH));
  }

  mitk::pa::Volume::Pointer createTest3DVolume(double value)
  {
    unsigned int xDim = 10;
    unsigned int yDim = 10;
    unsigned int zDim = 10;
    unsigned int length = xDim * yDim * zDim;
    auto* data = new double[length];
    for (unsigned int i = 0; i < length; i++)
      data[i] = value;

    return mitk::pa::Volume::New(data, xDim, yDim, zDim);
  }

  void testGenerateBatchFileString()
  {
    std::string batchGenerationString = mitk::pa::SimulationBatchGenerator::CreateBatchSimulationString(m_Parameters);
    CPPUNIT_ASSERT(!batchGenerationString.empty());
  }

  void testGenerateBatchFileAndSaveFile()
  {
    mitk::pa::SimulationBatchGenerator::WriteBatchFileAndSaveTissueVolume(m_Parameters, m_Test3DVolume->AsMitkImage());
    CPPUNIT_ASSERT(itksys::SystemTools::FileExists(TEST_FOLDER_PATH + m_Parameters->GetTissueName() + "000.nrrd"));
    CPPUNIT_ASSERT(itksys::SystemTools::FileExists(TEST_FOLDER_PATH + "simulate_all.sh") || itksys::SystemTools::FileExists(TEST_FOLDER_PATH + "simulate_all.bat"));
    CPPUNIT_ASSERT(itksys::SystemTools::FileExists(TEST_FOLDER_PATH + m_Parameters->GetTissueName() + "000")
      && itksys::SystemTools::FileIsDirectory(TEST_FOLDER_PATH + m_Parameters->GetTissueName() + "000"));
  }

  void tearDown() override
  {
    m_Parameters = nullptr;
    CPPUNIT_ASSERT_MESSAGE("Resource leak of test files onto hard drive..", itksys::SystemTools::RemoveADirectory(TEST_FOLDER_PATH) == true);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSimulationBatchGenerator)
