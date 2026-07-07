/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCommon.h>
#include <mitkFileSystem.h>
#include <mitkIOUtil.h>
#include <mitkPythonContext.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <fstream>

class mitkPythonContextTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPythonContextTestSuite);
  MITK_TEST(TestExecuteAndGetVariable);
  MITK_TEST(TestExecuteFile);
  MITK_TEST(TestBindImageToPython);
  MITK_TEST(TestPythonContextExclusivity);
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp()
  {
  }

  void TestExecuteAndGetVariable()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    pythonContext.Execute("result = 5 + 5");
    auto result = pythonContext.GetVariableAsInt("result");

    CPPUNIT_ASSERT_MESSAGE("Variable 'result' should exist", result.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Result should be 10", 10, result.value());
  }

  void TestExecuteFile()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    const fs::path scriptPath = fs::temp_directory_path() / "mitk_python_context_execute_file_test.py";
    {
      std::ofstream stream(scriptPath.string(), std::ios::binary);
      stream << "from pathlib import Path\n"
             << "file_name = Path(__file__).name\n"
             << "file_exists = Path(__file__).is_file()\n"
             << "file_value = 21 * 2\n";
    }

    try
    {
      pythonContext.ExecuteFile(scriptPath);
    }
    catch (...)
    {
      std::error_code error;
      fs::remove(scriptPath, error);
      throw;
    }

    std::error_code error;
    fs::remove(scriptPath, error);

    auto fileName = pythonContext.GetVariableAsString("file_name");
    CPPUNIT_ASSERT_MESSAGE("file_name should exist", fileName.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Executed file should see its own path",
                                 std::string("mitk_python_context_execute_file_test.py"),
                                 fileName.value());

    auto fileExists = pythonContext.GetVariableAsBool("file_exists");
    CPPUNIT_ASSERT_MESSAGE("file_exists should exist", fileExists.has_value());
    CPPUNIT_ASSERT_MESSAGE("__file__ should point to an existing file during execution", fileExists.value());

    auto fileValue = pythonContext.GetVariableAsInt("file_value");
    CPPUNIT_ASSERT_MESSAGE("file_value should exist", fileValue.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("ExecuteFile should run the file contents", 42, fileValue.value());
  }

  void TestBindImageToPython()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    auto image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Pic3D.nrrd"));

    try
    {
      pythonContext.BindImage(image, "test_image");
    }
    catch (const mitk::Exception& e)
    {
      MITK_ERROR << e.GetDescription();
      CPPUNIT_FAIL("Error in binding MITK image to Python");
    }

    CPPUNIT_ASSERT_MESSAGE("test_image should exist in context", pythonContext.HasVariable("test_image"));

    pythonContext.Execute("dims = test_image.get_dimension()");
    auto dims = pythonContext.GetVariableAsInt("dims");

    CPPUNIT_ASSERT_MESSAGE("Variable 'dims' should exist", dims.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image should have 3 dimensions", 3, dims.value());
  }

  void TestPythonContextExclusivity()
  {
    mitk::PythonContext pythonContext_1;
    mitk::PythonContext pythonContext_2;

    try
    {
      pythonContext_1.Execute("test_var_context_1 = 10");
      pythonContext_2.Execute("test_var_context_2 = 20");
    }
    catch (const mitk::Exception& e)
    {
      MITK_ERROR << e.GetDescription();
      CPPUNIT_FAIL("Error in executing commands in Python");
    }

    CPPUNIT_ASSERT_MESSAGE("test_var_context_1 should be found in context 1",
                           pythonContext_1.HasVariable("test_var_context_1"));
    CPPUNIT_ASSERT_MESSAGE("test_var_context_2 should not be found in context 1",
                           !pythonContext_1.HasVariable("test_var_context_2"));
    CPPUNIT_ASSERT_MESSAGE("test_var_context_2 should be found in context 2",
                           pythonContext_2.HasVariable("test_var_context_2"));
    CPPUNIT_ASSERT_MESSAGE("test_var_context_1 should not be found in context 2",
                           !pythonContext_2.HasVariable("test_var_context_1"));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPythonContext)
