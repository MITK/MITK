/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include <mitkTestFixture.h>
#include <mitkTestingConfig.h>

#include <mitkIOUtil.h>
#include <mitkUtf8Util.h>
#include <mitkImageGenerator.h>
#include <mitkIOMetaInformationPropertyConstants.h>
#include <mitkVersion.h>

#include <itkMetaDataObject.h>
#include <itkNrrdImageIO.h>
#include <itksys/SystemTools.hxx>

class mitkIOUtilTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkIOUtilTestSuite);
  MITK_TEST(TestTempMethods);
  MITK_TEST(TestSaveEmptyData);
  MITK_TEST(TestLoadAndSaveImage);
  MITK_TEST(TestNullLoad);
  MITK_TEST(TestNullSave);
  MITK_TEST(TestLoadAndSavePointSet);
  MITK_TEST(TestLoadAndSaveSurface);
  MITK_TEST(TestTempMethodsForUniqueFilenames);
  MITK_TEST(TestTempMethodsForUniqueFilenames);
  MITK_TEST(TestIOMetaInformation);
  MITK_TEST(TestUtf8);
  CPPUNIT_TEST_SUITE_END();

private:
  std::string m_ImagePath;
  std::string m_SurfacePath;
  std::string m_PointSetPath;

public:
  void setUp() override
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
    std::string tmpFilePath3 =
      mitk::IOUtil::CreateTemporaryFile(tmpFile3, std::ios_base::binary, "my-XXXXXX.TXT", programPath);
    CPPUNIT_ASSERT(tmpFile3 && tmpFile3.is_open());
    CPPUNIT_ASSERT(tmpFilePath3.size() > programPath.size());
    CPPUNIT_ASSERT(tmpFilePath3.substr(0, programPath.size()) == programPath);
    CPPUNIT_ASSERT(tmpFilePath3.substr(tmpFilePath3.size() - 13, 3) == "my-");
    CPPUNIT_ASSERT(tmpFilePath3.substr(tmpFilePath3.size() - 4) == ".TXT");
    tmpFile3.close();
    // CPPUNIT_ASSERT(std::remove(tmpFilePath3.c_str()) == 0)

    std::string tmpFilePath4 = mitk::IOUtil::CreateTemporaryFile();
    std::ofstream file;
    file.open(tmpFilePath4.c_str());
    CPPUNIT_ASSERT_MESSAGE("Testing if file exists after CreateTemporaryFile()", file.is_open());

    CPPUNIT_ASSERT_THROW(mitk::IOUtil::CreateTemporaryFile(tmpFile2, "XX"), mitk::Exception);

    std::string tmpDir = mitk::IOUtil::CreateTemporaryDirectory();
    CPPUNIT_ASSERT(tmpDir.size() > tmpPath.size());
    CPPUNIT_ASSERT(tmpDir.substr(0, tmpPath.size()) == tmpPath);
    CPPUNIT_ASSERT(itksys::SystemTools::RemoveADirectory(mitk::Utf8Util::Local8BitToUtf8(tmpDir).c_str()));

    std::string tmpDir2 = mitk::IOUtil::CreateTemporaryDirectory("my-XXXXXX", programPath);
    CPPUNIT_ASSERT(tmpDir2.size() > programPath.size());
    CPPUNIT_ASSERT(tmpDir2.substr(0, programPath.size()) == programPath);
    CPPUNIT_ASSERT(itksys::SystemTools::RemoveADirectory(mitk::Utf8Util::Local8BitToUtf8(tmpDir2).c_str()));
  }

  void TestTempMethodsForUniqueFilenames()
  {
    int numberOfFiles = 100;

    // create 100 empty files
    std::vector<std::string> v100filenames;
    for (int i = 0; i < numberOfFiles; i++)
    {
      v100filenames.push_back(mitk::IOUtil::CreateTemporaryFile());
    }

    // check if all of them are unique
    for (int i = 0; i < numberOfFiles; i++)
      for (int j = 0; j < numberOfFiles; j++)
      {
        if (i != j)
        {
          std::stringstream message;
          message << "Checking if file " << i << " and file " << j
                  << " are different, which should be the case because each of them should be unique.";
          CPPUNIT_ASSERT_MESSAGE(message.str(), (v100filenames.at(i) != v100filenames.at(j)));
        }
      }

    // delete all the files / clean up
    for (int i = 0; i < numberOfFiles; i++)
    {
      std::remove(v100filenames.at(i).c_str());
    }
  }

  void TestLoadAndSaveImage()
  {
    mitk::Image::Pointer img1 = mitk::IOUtil::Load<mitk::Image>(m_ImagePath);
    CPPUNIT_ASSERT(img1.IsNotNull());

    std::ofstream tmpStream;
    std::string imagePath = mitk::IOUtil::CreateTemporaryFile(tmpStream, "diffpic3d-XXXXXX.nrrd");
    tmpStream.close();
    std::string imagePath2 = mitk::IOUtil::CreateTemporaryFile(tmpStream, "diffpic3d-XXXXXX.nii.gz");
    tmpStream.close();

    // the cases where no exception should be thrown
    CPPUNIT_ASSERT_NO_THROW(mitk::IOUtil::Save(img1, imagePath));
    CPPUNIT_ASSERT_NO_THROW(mitk::IOUtil::Save(img1.GetPointer(), imagePath2));

    // load data which does not exist
    CPPUNIT_ASSERT_THROW(mitk::IOUtil::Load("fileWhichDoesNotExist.nrrd"), mitk::Exception);

    // delete the files after the test is done
    std::remove(imagePath.c_str());
    std::remove(imagePath2.c_str());

    mitk::Image::Pointer relativImage = mitk::ImageGenerator::GenerateGradientImage<float>(4, 4, 4, 1);
    std::string imagePath3 = mitk::IOUtil::CreateTemporaryFile(tmpStream, "XXXXXX.nrrd");
    tmpStream.close();
    mitk::IOUtil::Save(relativImage, imagePath3);
    CPPUNIT_ASSERT_NO_THROW(mitk::IOUtil::Load(imagePath3));
    std::remove(imagePath3.c_str());
  }

  /**
  * \brief This method calls all available load methods with a nullpointer and an empty pathand expects an exception
  **/
  void TestNullLoad()
  {
    CPPUNIT_ASSERT_THROW(mitk::IOUtil::Load(""), mitk::Exception);
  }

  /**
  * \brief This method calls the save method (to which all other convenience save methods reference) with null
  *parameters
  **/
  void TestNullSave()
  {
    CPPUNIT_ASSERT_THROW(mitk::IOUtil::Save(nullptr, mitk::IOUtil::CreateTemporaryFile()), mitk::Exception);
    CPPUNIT_ASSERT_THROW(mitk::IOUtil::Save(mitk::Image::New().GetPointer(), ""), mitk::Exception);
  }

  void TestLoadAndSavePointSet()
  {
    mitk::PointSet::Pointer pointset = mitk::IOUtil::Load<mitk::PointSet>(m_PointSetPath);
    CPPUNIT_ASSERT(pointset.IsNotNull());

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

    // delete the files after the test is done
    std::remove(pointSetPath.c_str());
    std::remove(pointSetPathWithDefaultExtension.c_str());
    std::remove(pointSetPathWithoutDefaultExtension.c_str());
  }

  void TestLoadAndSaveSurface()
  {
    mitk::Surface::Pointer surface = mitk::IOUtil::Load<mitk::Surface>(m_SurfacePath);
    CPPUNIT_ASSERT(surface.IsNotNull());

    std::ofstream tmpStream;
    std::string surfacePath = mitk::IOUtil::CreateTemporaryFile(tmpStream, "diffsurface-XXXXXX.stl");

    // the cases where no exception should be thrown
    CPPUNIT_ASSERT_NO_THROW(mitk::IOUtil::Save(surface, surfacePath));

    // test if exception is thrown as expected on unknown extsension
    CPPUNIT_ASSERT_THROW(mitk::IOUtil::Save(surface, "testSurface.xXx"), mitk::Exception);

    // delete the files after the test is done
    std::remove(surfacePath.c_str());
  }

  std::string GenerateMetaDictKey(const mitk::PropertyKeyPath& propKey)
  {
    auto result = mitk::PropertyKeyPathToPropertyName(propKey);
    std::replace(result.begin(), result.end(), '.', '_');
    return result;
  }

  std::string GetValueFromMetaDict(const itk::MetaDataDictionary& dict, const mitk::PropertyKeyPath& propKey)
  {
    auto metaValueBase = dict.Get(GenerateMetaDictKey(propKey));
    auto metaValue = dynamic_cast<const itk::MetaDataObject<std::string>*>(metaValueBase);
    return metaValue->GetMetaDataObjectValue();
  }

  void TestIOMetaInformation()
  {
    mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(m_ImagePath);
    CPPUNIT_ASSERT(img.IsNotNull());
    
    auto value = img->GetProperty(mitk::PropertyKeyPathToPropertyName(mitk::IOMetaInformationPropertyConstants::READER_DESCRIPTION()).c_str())->GetValueAsString();
    CPPUNIT_ASSERT_EQUAL(std::string("ITK NrrdImageIO"), value);
    value = img->GetProperty(mitk::PropertyKeyPathToPropertyName(mitk::IOMetaInformationPropertyConstants::READER_INPUTLOCATION()).c_str())->GetValueAsString();
    CPPUNIT_ASSERT_EQUAL(m_ImagePath, value);
    value = img->GetProperty(mitk::PropertyKeyPathToPropertyName(mitk::IOMetaInformationPropertyConstants::READER_MIME_CATEGORY()).c_str())->GetValueAsString();
    CPPUNIT_ASSERT_EQUAL(std::string("Images"), value);
    value = img->GetProperty(mitk::PropertyKeyPathToPropertyName(mitk::IOMetaInformationPropertyConstants::READER_MIME_NAME()).c_str())->GetValueAsString();
    CPPUNIT_ASSERT_EQUAL(std::string("application/vnd.mitk.image.nrrd"), value);
    value = img->GetProperty(mitk::PropertyKeyPathToPropertyName(mitk::IOMetaInformationPropertyConstants::READER_VERSION()).c_str())->GetValueAsString();
    CPPUNIT_ASSERT_EQUAL(std::string(MITK_VERSION_STRING), value);

    //check if the information is persistet correctly on save.
    std::ofstream tmpStream;
    std::string imagePath = mitk::IOUtil::CreateTemporaryFile(tmpStream, "ioMeta_XXXXXX.nrrd");
    tmpStream.close();
    mitk::IOUtil::Save(img, imagePath);

    auto io = itk::NrrdImageIO::New();
    io->SetFileName(imagePath);
    io->ReadImageInformation();
    auto metaDict = io->GetMetaDataDictionary();

    auto metaValue = GetValueFromMetaDict(metaDict, mitk::IOMetaInformationPropertyConstants::READER_DESCRIPTION());
    CPPUNIT_ASSERT_EQUAL(std::string("ITK NrrdImageIO"), metaValue);
    metaValue = GetValueFromMetaDict(metaDict, mitk::IOMetaInformationPropertyConstants::READER_INPUTLOCATION());
    CPPUNIT_ASSERT_EQUAL(m_ImagePath, metaValue);
    metaValue = GetValueFromMetaDict(metaDict, mitk::IOMetaInformationPropertyConstants::READER_MIME_CATEGORY());
    CPPUNIT_ASSERT_EQUAL(std::string("Images"), metaValue);
    metaValue = GetValueFromMetaDict(metaDict, mitk::IOMetaInformationPropertyConstants::READER_MIME_NAME());
    CPPUNIT_ASSERT_EQUAL(std::string("application/vnd.mitk.image.nrrd"), metaValue);
    metaValue = GetValueFromMetaDict(metaDict, mitk::IOMetaInformationPropertyConstants::READER_VERSION());
    CPPUNIT_ASSERT_EQUAL(std::string(MITK_VERSION_STRING), metaValue);

    // delete the files after the test is done
    std::remove(imagePath.c_str());
  }

  void TestUtf8()
  {
    const std::string utf8Path = u8"UTF-8/\u00c4.nrrd"; // LATIN CAPITAL LETTER A WITH DIAERESIS (U+00C4)
    const std::string local8BitPath = mitk::Utf8Util::Utf8ToLocal8Bit(utf8Path);

    CPPUNIT_ASSERT(utf8Path == mitk::Utf8Util::Local8BitToUtf8(local8BitPath));

    const std::string path = GetTestDataFilePath(local8BitPath);

    std::fstream file;
    file.open(path);
    CPPUNIT_ASSERT(file.is_open());

    auto image = mitk::IOUtil::Load<mitk::Image>(path);
    CPPUNIT_ASSERT(image.IsNotNull());
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkIOUtil)
