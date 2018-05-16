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

#include <mitkPAVesselTree.h>
#include <mitkPAIOUtil.h>
#include <mitkIOUtil.h>
#include <mitkCoreServices.h>
#include <mitkPropertyPersistence.h>
#include <mitkPropertyPersistenceInfo.h>
#include <itkFileTools.h>

class mitkPhotoacousticIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPhotoacousticIOTestSuite);
  MITK_TEST(testLoadInSilicoTissueNrrdFile);
  MITK_TEST(testLoad3DVolumeNrrdFile);
  MITK_TEST(testLoad3DVolumeNrrdFileWithBlur);
  MITK_TEST(testGetNumberOfNrrdFilesInTestDir);
  MITK_TEST(testGetChildFoldersFromFolder);
  MITK_TEST(testLoadFCMs);
  CPPUNIT_TEST_SUITE_END();

private:

  const std::string TEST_FOLDER_PATH = "testFiles/";
  const std::string TEST_IN_SILICO_VOLUME_PATH = "testInSilicoVolume";
  const std::string TEST_3D_Volume_PATH = "test3DVolume";
  const std::string TEST_FILE_ENDING = ".nrrd";
  const std::string TEST_QUALIFIED_FOLDER_PATH = TEST_FOLDER_PATH + TEST_IN_SILICO_VOLUME_PATH + "/";
  const std::string FOLDER_FOLDER = "folder/";
  const std::string FCM_PATH = TEST_FOLDER_PATH + "fcms/";
  const int NUMBER_OF_NRRD_FILES_IN_TEST_DIR = 2;
  mitk::pa::TissueGeneratorParameters::Pointer m_VolumeProperties;
  mitk::pa::InSilicoTissueVolume::Pointer m_TestInSilicoVolume;
  mitk::pa::Volume::Pointer m_Test3DVolume;

public:

  void setUp() override
  {
    m_VolumeProperties = createTestVolumeParameters();
    m_TestInSilicoVolume = mitk::pa::InSilicoTissueVolume::New(m_VolumeProperties);
    m_Test3DVolume = createTest3DVolume(5);
    itk::FileTools::CreateDirectory(TEST_FOLDER_PATH);
    itk::FileTools::CreateDirectory(TEST_QUALIFIED_FOLDER_PATH);
    itk::FileTools::CreateDirectory(TEST_FOLDER_PATH + FOLDER_FOLDER + FOLDER_FOLDER);
    itk::FileTools::CreateDirectory(FCM_PATH);
    CPPUNIT_ASSERT(itksys::SystemTools::FileIsDirectory(TEST_FOLDER_PATH));
    CPPUNIT_ASSERT(itksys::SystemTools::FileIsDirectory(TEST_QUALIFIED_FOLDER_PATH));
    CPPUNIT_ASSERT(itksys::SystemTools::FileIsDirectory(TEST_FOLDER_PATH + FOLDER_FOLDER + FOLDER_FOLDER));
    CPPUNIT_ASSERT(itksys::SystemTools::FileIsDirectory(FCM_PATH));

    mitk::IOUtil::Save(m_TestInSilicoVolume->ConvertToMitkImage(),
      TEST_FOLDER_PATH + TEST_IN_SILICO_VOLUME_PATH + TEST_FILE_ENDING);
    mitk::IOUtil::Save(m_Test3DVolume->AsMitkImage(),
      TEST_FOLDER_PATH + TEST_3D_Volume_PATH + TEST_FILE_ENDING);
    auto yo0 = createTest3DVolume(1)->AsMitkImage();
    auto yo1 = createTest3DVolume(2)->AsMitkImage();

    yo0->GetPropertyList()->SetStringProperty("y-offset", "0");
    yo1->GetPropertyList()->SetStringProperty("y-offset", "1");
    mitk::CoreServices::GetPropertyPersistence()->AddInfo(mitk::PropertyPersistenceInfo::New("y-offset"));

    mitk::IOUtil::Save(yo0, TEST_QUALIFIED_FOLDER_PATH + TEST_IN_SILICO_VOLUME_PATH + "_yo0" + TEST_FILE_ENDING);
    mitk::IOUtil::Save(yo1, TEST_QUALIFIED_FOLDER_PATH + TEST_IN_SILICO_VOLUME_PATH + "_yo1" + TEST_FILE_ENDING);
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

  mitk::pa::TissueGeneratorParameters::Pointer createTestVolumeParameters()
  {
    auto returnParameters = mitk::pa::TissueGeneratorParameters::New();
    returnParameters->SetXDim(10);
    returnParameters->SetYDim(10);
    returnParameters->SetZDim(10);
    returnParameters->SetBackgroundAbsorption(0);
    returnParameters->SetBackgroundScattering(0);
    returnParameters->SetBackgroundAnisotropy(0);
    return returnParameters;
  }

  void assertEqual(mitk::pa::Volume::Pointer first, mitk::pa::Volume::Pointer second)
  {
    CPPUNIT_ASSERT(first->GetXDim() == second->GetXDim());
    CPPUNIT_ASSERT(first->GetYDim() == second->GetYDim());
    CPPUNIT_ASSERT(first->GetZDim() == second->GetZDim());
    for (unsigned int x = 0; x < first->GetXDim(); ++x)
      for (unsigned int y = 0; y < first->GetYDim(); ++y)
        for (unsigned int z = 0; z < first->GetZDim(); ++z)
        {
          std::string message = "Expected " + std::to_string(first->GetData(x, y, z)) + " but was " + std::to_string(second->GetData(x, y, z));
          CPPUNIT_ASSERT_MESSAGE(message, std::abs(first->GetData(x, y, z) - second->GetData(x, y, z)) < 1e-6);
        }
  }

  void testLoadInSilicoTissueNrrdFile()
  {
    auto loadedVolume = mitk::pa::IOUtil::LoadInSilicoTissueVolumeFromNrrdFile(TEST_FOLDER_PATH + TEST_IN_SILICO_VOLUME_PATH + TEST_FILE_ENDING);
    CPPUNIT_ASSERT(loadedVolume->GetTDim() == m_TestInSilicoVolume->GetTDim());
    assertEqual(m_TestInSilicoVolume->GetAbsorptionVolume(), loadedVolume->GetAbsorptionVolume());
    assertEqual(m_TestInSilicoVolume->GetScatteringVolume(), loadedVolume->GetScatteringVolume());
    assertEqual(m_TestInSilicoVolume->GetAnisotropyVolume(), loadedVolume->GetAnisotropyVolume());
  }

  void testLoad3DVolumeNrrdFile()
  {
    auto loadedVolume = mitk::pa::IOUtil::LoadNrrd(TEST_FOLDER_PATH + TEST_3D_Volume_PATH + TEST_FILE_ENDING);
    assertEqual(loadedVolume, m_Test3DVolume);
  }

  void testLoad3DVolumeNrrdFileWithBlur()
  {
    auto loadedVolume = mitk::pa::IOUtil::LoadNrrd(TEST_FOLDER_PATH + TEST_3D_Volume_PATH + TEST_FILE_ENDING, 1);
    assertEqual(loadedVolume, m_Test3DVolume);
  }

  void testGetNumberOfNrrdFilesInTestDir()
  {
    int numberOfFiles = mitk::pa::IOUtil::GetNumberOfNrrdFilesInDirectory(TEST_FOLDER_PATH);
    CPPUNIT_ASSERT(numberOfFiles == NUMBER_OF_NRRD_FILES_IN_TEST_DIR);
  }

  void testGetChildFoldersFromFolder()
  {
    std::vector<std::string> childFolders = mitk::pa::IOUtil::GetAllChildfoldersFromFolder(TEST_FOLDER_PATH);
    CPPUNIT_ASSERT(childFolders.size() == 1);
    CPPUNIT_ASSERT(childFolders[0] == TEST_FOLDER_PATH);
    childFolders = mitk::pa::IOUtil::GetAllChildfoldersFromFolder(TEST_FOLDER_PATH + FOLDER_FOLDER);
    MITK_INFO << "ChildFolders: " << childFolders.size();
    CPPUNIT_ASSERT(childFolders.size() == 1);
    CPPUNIT_ASSERT(childFolders[0] == TEST_FOLDER_PATH + FOLDER_FOLDER + "/folder");
  }

  void testLoadFCMs()
  {
    auto fcm1 = createTest3DVolume(1);
    auto fcm2 = createTest3DVolume(2);
    auto fcm3 = createTest3DVolume(3);
    auto fcm4 = createTest3DVolume(4);
    mitk::IOUtil::Save(fcm1->AsMitkImage(), FCM_PATH + "fcm1_p0,0,0FluenceContributionMap.nrrd");
    mitk::IOUtil::Save(fcm2->AsMitkImage(), FCM_PATH + "fcm1_p0,0,1FluenceContributionMap.nrrd");
    mitk::IOUtil::Save(fcm3->AsMitkImage(), FCM_PATH + "fcm1_p1,0,0FluenceContributionMap.nrrd");
    mitk::IOUtil::Save(fcm4->AsMitkImage(), FCM_PATH + "fcm1_p1,0,1FluenceContributionMap.nrrd");

    int prog = 0;
    auto map = mitk::pa::IOUtil::LoadFluenceContributionMaps(FCM_PATH, 0, &prog);

    assertEqual(fcm1, map[mitk::pa::IOUtil::Position{ 0,0 }]);
    assertEqual(fcm2, map[mitk::pa::IOUtil::Position{ 0,1 }]);
    assertEqual(fcm3, map[mitk::pa::IOUtil::Position{ 1,0 }]);
    assertEqual(fcm4, map[mitk::pa::IOUtil::Position{ 1,1 }]);
  }

  void tearDown() override
  {
    //CPPUNIT_ASSERT_MESSAGE("Resource leak of test files onto hard drive..", itksys::SystemTools::RemoveADirectory(TEST_FOLDER_PATH) == true);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPhotoacousticIO)
