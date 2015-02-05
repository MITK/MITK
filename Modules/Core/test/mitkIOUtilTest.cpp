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
#include <mitkTestFixture.h>

#include <mitkIOUtil.h>
#include <mitkImageGenerator.h>

#include <itksys/SystemTools.hxx>

class mitkIOUtilTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkIOUtilTestSuite);
  MITK_TEST(TestTempMethods);
  MITK_TEST(TestSaveEmptyData);
  MITK_TEST(TestLoadAndSaveImage);
  MITK_TEST(TestLoadAndSavePointSet);
  MITK_TEST(TestLoadAndSaveSurface);
  MITK_TEST(TestTempMethodsForUniqueFilenames);
  CPPUNIT_TEST_SUITE_END();

private:

  std::string m_ImagePath;
  std::string m_SurfacePath;
  std::string m_PointSetPath;

public:

  void setUp()
  {
    m_ImagePath = GetTestDataFilePath("Pic3D.nrrd");
    m_SurfacePath = GetTestDataFilePath("binary.stl");
    m_PointSetPath = GetTestDataFilePath("pointSet.mps");
  }

  void TestSaveEmptyData()
  {
    mitk::Surface::Pointer data = mitk::Surface::New();
    CPPUNIT_ASSERT_THROW(mitk::IOUtil::Save(data, "/tmp/dummy"), mitk::Exception);
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

    std::string tmpFilePath4 = mitk::IOUtil::CreateTemporaryFile();
    std::ofstream file;
    file.open(tmpFilePath4.c_str());
    CPPUNIT_ASSERT_MESSAGE("Testing if file exists after CreateTemporaryFile()",file.is_open());


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

  void TestTempMethodsForUniqueFilenames()
  {
    int numberOfFiles = 100;

    //create 100 empty files
    std::vector<std::string> v100filenames;
    for(int i=0; i<numberOfFiles; i++) {v100filenames.push_back(mitk::IOUtil::CreateTemporaryFile());}

    //check if all of them are unique
    for(int i=0; i<numberOfFiles; i++) for(int j=0; j<numberOfFiles; j++)
    {
      if(i!=j)
        {
        std::stringstream message;
        message << "Checking if file " << i << " and file " << j << " are different, which should be the case because each of them should be unique.";
        CPPUNIT_ASSERT_MESSAGE(message.str(),(v100filenames.at(i)!=v100filenames.at(j)));
        }
    }

    //delete all the files / clean up
    for(int i=0; i<numberOfFiles; i++) {std::remove(v100filenames.at(i).c_str());}
  }

  void TestLoadAndSaveImage()
  {
    mitk::Image::Pointer img1 = mitk::IOUtil::LoadImage(m_ImagePath);
    CPPUNIT_ASSERT( img1.IsNotNull());

    std::ofstream tmpStream;
    std::string imagePath = mitk::IOUtil::CreateTemporaryFile(tmpStream, "diffpic3d-XXXXXX.nrrd");
    tmpStream.close();
    std::string imagePath2 = mitk::IOUtil::CreateTemporaryFile(tmpStream, "diffpic3d-XXXXXX.nii.gz");
    tmpStream.close();

    // the cases where no exception should be thrown
    CPPUNIT_ASSERT_NO_THROW(mitk::IOUtil::Save(img1, imagePath));
    CPPUNIT_ASSERT_NO_THROW(mitk::IOUtil::Save(img1.GetPointer(), imagePath2));

    //load data which does not exist
    CPPUNIT_ASSERT_THROW(mitk::IOUtil::LoadImage("fileWhichDoesNotExist.nrrd"), mitk::Exception);

    //delete the files after the test is done
    std::remove(imagePath.c_str());
    std::remove(imagePath2.c_str());

    mitk::Image::Pointer relativImage = mitk::ImageGenerator::GenerateGradientImage<float>(4,4,4,1);
    std::string imagePath3 = mitk::IOUtil::CreateTemporaryFile(tmpStream, "XXXXXX.nrrd");
    tmpStream.close();
    mitk::IOUtil::Save(relativImage, imagePath3);
    CPPUNIT_ASSERT_NO_THROW(mitk::IOUtil::LoadImage(imagePath3));
    std::remove(imagePath3.c_str());
  }

  void TestLoadAndSavePointSet()
  {
    mitk::PointSet::Pointer pointset = mitk::IOUtil::LoadPointSet(m_PointSetPath);
    CPPUNIT_ASSERT( pointset.IsNotNull());

    std::ofstream tmpStream;
    std::string pointSetPath = mitk::IOUtil::CreateTemporaryFile(tmpStream, "XXXXXX.mps");
    tmpStream.close();
    std::string pointSetPathWithDefaultExtension = mitk::IOUtil::CreateTemporaryFile(tmpStream, "XXXXXX.mps");
    tmpStream.close();
    std::string pointSetPathWithoutDefaultExtension = mitk::IOUtil::CreateTemporaryFile(tmpStream);
    tmpStream.close();

    // the cases where no exception should be thrown
    CPPUNIT_ASSERT_NO_THROW(mitk::IOUtil::Save(pointset, pointSetPathWithDefaultExtension));

    // test if defaultextension is inserted if no extension is present
    CPPUNIT_ASSERT_NO_THROW(mitk::IOUtil::Save(pointset, pointSetPathWithoutDefaultExtension.c_str()));

    //delete the files after the test is done
    std::remove(pointSetPath.c_str());
    std::remove(pointSetPathWithDefaultExtension.c_str());
    std::remove(pointSetPathWithoutDefaultExtension.c_str());
  }

  void TestLoadAndSaveSurface()
  {
    mitk::Surface::Pointer surface = mitk::IOUtil::LoadSurface(m_SurfacePath);
    CPPUNIT_ASSERT( surface.IsNotNull());

    std::ofstream tmpStream;
    std::string surfacePath = mitk::IOUtil::CreateTemporaryFile(tmpStream, "diffsurface-XXXXXX.stl");

    // the cases where no exception should be thrown
    CPPUNIT_ASSERT_NO_THROW(mitk::IOUtil::Save(surface, surfacePath));

    // test if exception is thrown as expected on unknown extsension
    CPPUNIT_ASSERT_THROW(mitk::IOUtil::Save(surface,"testSurface.xXx"), mitk::Exception);

    //delete the files after the test is done
    std::remove(surfacePath.c_str());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkIOUtil)
