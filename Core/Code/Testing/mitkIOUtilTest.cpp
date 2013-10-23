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

#include "mitkTestingMacros.h"
#include <mitkTestingConfig.h>
#include <mitkParameterizedTestFixture.h>

#include <mitkIOUtil.h>
#include <mitkImageGenerator.h>

#include <itksys/SystemTools.hxx>

class mitkIOUtilTestSuite : public mitk::ParameterizedTestFixture
{

  CPPUNIT_TEST_SUITE(mitkIOUtilTestSuite);
  CPPUNIT_TEST(TestTempMethods);
  MITK_PARAMETERIZED_TEST(TestLoadAndSaveMethods);
  CPPUNIT_TEST_SUITE_END();

private:

  std::string pathToImage;
  std::string pathToPointSet;
  std::string pathToSurface;

public:

  static CppUnit::Test* suite(int argc, char* argv[])
  {
    CppUnit::TestSuite* testSuite = new CppUnit::TestSuite("mitkIOUtilTest");
    testSuite->addTest(new mitk::ParameterizedTestCaller<mitkIOUtilTestSuite>(
                         "bla", &mitkIOUtilTestSuite::TestTempMethods));
    return testSuite;
  }

  void setUpParameter(const std::vector<std::string>& parameter)
  {
    CPPUNIT_ASSERT(parameter.size() == 3);
    pathToImage = parameter[0];
    pathToPointSet = parameter[1];
    pathToSurface = parameter[2];
  }

  void TestTempMethods()
  {
    std::string tmpPath = mitk::IOUtil::GetTempPath();
    CPPUNIT_ASSERT(!tmpPath.empty());

    std::ofstream tmpFile;
    std::string tmpFilePath = mitk::IOUtil::CreateTemporaryFile(tmpFile);
    CPPUNIT_ASSERT(tmpFile && tmpFile.is_open());
    CPPUNIT_ASSERT(tmpFilePath.size() > tmpPath.size());
    CPPUNIT_ASSERT(tmpFilePath.substr(0, tmpPath.size()) == tmpPath);

    tmpFile.close();
    CPPUNIT_ASSERT(std::remove(tmpFilePath.c_str()) == 0);

    std::string programPath = mitk::IOUtil::GetProgramPath();
    CPPUNIT_ASSERT(!programPath.empty());
    std::ofstream tmpFile2;
    std::string tmpFilePath2 = mitk::IOUtil::CreateTemporaryFile(tmpFile2, "my-XXXXXX", programPath);
    CPPUNIT_ASSERT(tmpFile2 && tmpFile2.is_open());
    CPPUNIT_ASSERT(tmpFilePath2.size() > programPath.size());
    CPPUNIT_ASSERT(tmpFilePath2.substr(0, programPath.size()) == programPath);
    tmpFile2.close();
    CPPUNIT_ASSERT(std::remove(tmpFilePath2.c_str()) == 0);

    std::ofstream tmpFile3;
    std::string tmpFilePath3 = mitk::IOUtil::CreateTemporaryFile(tmpFile3, std::ios_base::binary,
                                                                 "my-XXXXXX.TXT", programPath);
    CPPUNIT_ASSERT(tmpFile3 && tmpFile3.is_open());
    CPPUNIT_ASSERT(tmpFilePath3.size() > programPath.size());
    CPPUNIT_ASSERT(tmpFilePath3.substr(0, programPath.size()) == programPath);
    CPPUNIT_ASSERT(tmpFilePath3.substr(tmpFilePath3.size() - 13, 3) == "my-");
    CPPUNIT_ASSERT(tmpFilePath3.substr(tmpFilePath3.size() - 4) == ".TXT");
    tmpFile3.close();
    //CPPUNIT_ASSERT(std::remove(tmpFilePath3.c_str()) == 0)

    CPPUNIT_ASSERT_THROW(mitk::IOUtil::CreateTemporaryFile(tmpFile2, "XX"), mitk::Exception);

    std::string tmpDir = mitk::IOUtil::CreateTemporaryDirectory();
    CPPUNIT_ASSERT(tmpDir.size() > tmpPath.size());
    CPPUNIT_ASSERT(tmpDir.substr(0, tmpPath.size()) == tmpPath);
    CPPUNIT_ASSERT(itksys::SystemTools::RemoveADirectory(tmpDir.c_str()));

    std::string tmpDir2 = mitk::IOUtil::CreateTemporaryDirectory("my-XXXXXX", programPath);
    CPPUNIT_ASSERT(tmpDir2.size() > programPath.size());
    CPPUNIT_ASSERT(tmpDir2.substr(0, programPath.size()) == programPath);
    CPPUNIT_ASSERT(itksys::SystemTools::RemoveADirectory(tmpDir2.c_str()));
  }

  void TestLoadAndSaveMethods()
  {
    mitk::Image::Pointer img1 = mitk::IOUtil::LoadImage(pathToImage);
    CPPUNIT_ASSERT( img1.IsNotNull());
    mitk::PointSet::Pointer pointset = mitk::IOUtil::LoadPointSet(pathToPointSet);
    CPPUNIT_ASSERT( pointset.IsNotNull());
    mitk::Surface::Pointer surface = mitk::IOUtil::LoadSurface(pathToSurface);
    CPPUNIT_ASSERT( surface.IsNotNull());

    std::string outDir = MITK_TEST_OUTPUT_DIR;
    std::string imagePath = outDir+"/diffpic3d.nrrd";
    std::string imagePath2 = outDir+"/diffpic3d.nii.gz";
    std::string pointSetPath = outDir + "/diffpointset.mps";
    std::string surfacePath = outDir + "/diffsurface.stl";
    std::string pointSetPathWithDefaultExtension = outDir + "/diffpointset2.mps";
    std::string pointSetPathWithoutDefaultExtension = outDir + "/diffpointset2.xXx";

    // the cases where no exception should be thrown
    CPPUNIT_ASSERT(mitk::IOUtil::SaveImage(img1, imagePath));
    CPPUNIT_ASSERT(mitk::IOUtil::SaveBaseData(img1.GetPointer(), imagePath2));
    CPPUNIT_ASSERT(mitk::IOUtil::SavePointSet(pointset, pointSetPath));
    CPPUNIT_ASSERT(mitk::IOUtil::SaveSurface(surface, surfacePath));

    // test if defaultextension is inserted if no extension is present
    CPPUNIT_ASSERT(mitk::IOUtil::SavePointSet(pointset, pointSetPathWithoutDefaultExtension.c_str()));

    // test if exception is thrown as expected on unknown extsension
    CPPUNIT_ASSERT_THROW(mitk::IOUtil::SaveSurface(surface,"testSurface.xXx"), mitk::Exception);
    //load data which does not exist
    CPPUNIT_ASSERT_THROW(mitk::IOUtil::LoadImage("fileWhichDoesNotExist.nrrd"), mitk::Exception);

    //delete the files after the test is done
    remove(imagePath.c_str());
    remove(pointSetPath.c_str());
    remove(surfacePath.c_str());
    //remove the pointset with default extension and not the one without
    remove(pointSetPathWithDefaultExtension.c_str());

    mitk::Image::Pointer relativImage = mitk::ImageGenerator::GenerateGradientImage<float>(4,4,4,1);
    mitk::IOUtil::SaveImage(relativImage, "tempfile.nrrd");
    CPPUNIT_ASSERT_NO_THROW(mitk::IOUtil::LoadImage("tempfile.nrrd"));
    remove("tempfile.nrrd");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkIOUtil)
