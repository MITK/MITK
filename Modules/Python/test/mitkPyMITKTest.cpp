/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkFileSystem.h>
#include <mitkIOUtil.h>
#include <mitkNodePredicateGeometry.h>
#include <mitkPythonContext.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkPyMITKTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPyMITKTestSuite);
    MITK_TEST(TestImportPyMITK);
    MITK_TEST(TestLoadImage);
    MITK_TEST(TestTransferImage);
    MITK_TEST(TestSaveImage);
    MITK_TEST(TestConvertToNumpy);
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp() override
  {
    m_ImagePath = GetTestDataFilePath("Pic3D.nrrd");
  }

  void tearDown() override
  {
  }

  void TestImportPyMITK()
  {
    auto pythonContext = mitk::PythonContext::New();
    pythonContext->Activate();

    pythonContext->ExecuteString("print(pyMITK.SayHi())");
  }

  void TestLoadImage()
  {
    auto pythonContext = mitk::PythonContext::New();
    pythonContext->Activate();

    std::ostringstream pyCommands; pyCommands
      << "image_list = pyMITK.IOUtil.imread('" << m_ImagePath << "')\n"
      << "image = image_list[0]\n";
    pythonContext->ExecuteString(pyCommands.str());

    auto image = pythonContext->LoadImageFromPython("image");

    CPPUNIT_ASSERT(image != nullptr);

    auto refImage = mitk::IOUtil::Load<mitk::Image>(m_ImagePath);

    CPPUNIT_ASSERT(mitk::Equal(
      *refImage,
      *image,
      mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION,
      false));
  }

  void TestTransferImage()
  {
    auto image = mitk::IOUtil::Load<mitk::Image>(m_ImagePath);

    auto pythonContext = mitk::PythonContext::New();
    pythonContext->Activate();

    pythonContext->TransferBaseDataToPython(image, "image");

    pythonContext->ExecuteString("image_dim = image.GetDimension()");

    CPPUNIT_ASSERT_EQUAL(3, pythonContext->GetVariableAs<int>("image_dim").value_or(0));
  }

  void TestSaveImage()
  {
    std::ofstream tmpStream;
    std::string imagePath = mitk::IOUtil::CreateTemporaryFile(tmpStream, "pyMITK-XXXXXX.nrrd");
    tmpStream.close();
    std::replace(imagePath.begin(), imagePath.end(), '\\', '/');

    auto pythonContext = mitk::PythonContext::New();
    pythonContext->Activate();

    std::ostringstream pyCommands; pyCommands
      << "image_list = pyMITK.IOUtil.imread('" << m_ImagePath << "')\n"
      << "image = image_list[0]\n"
      << "pyMITK.IOUtil.imsave(image, '" << imagePath << "')\n";
    pythonContext->ExecuteString(pyCommands.str());

    CPPUNIT_ASSERT(fs::file_size(imagePath) > 0);

    auto refImage = mitk::IOUtil::Load<mitk::Image>(m_ImagePath);
    auto savedImage = mitk::IOUtil::Load<mitk::Image>(imagePath);

    CPPUNIT_ASSERT(mitk::Equal(
      *refImage,
      *savedImage,
      mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION,
      false));

    std::error_code error;
    fs::remove(imagePath, error);
  }

  void TestConvertToNumpy()
  {
    auto image = mitk::IOUtil::Load<mitk::Image>(m_ImagePath);

    auto pythonContext = mitk::PythonContext::New();
    pythonContext->Activate();

    std::ostringstream pyCommands; pyCommands
      << "image_list = pyMITK.IOUtil.imread('" << m_ImagePath << "')\n"
      << "image = image_list[0].GetAsNumpy()\n"
      << "assert image.shape == (49, 256, 256)\n";
    pythonContext->ExecuteString(pyCommands.str());
  }

private:
  std::string m_ImagePath;
};

MITK_TEST_SUITE_REGISTRATION(mitkPyMITK)
