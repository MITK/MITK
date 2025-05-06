/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCommon.h>
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include <mitkStandardFileLocations.h>
#include <mitkIOUtil.h>
#include <mitkPythonContext.h>


class mitkPythonTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPythonTestSuite);
  MITK_TEST(TestEvaluateOperationWithResult);
  MITK_TEST(TestCopyImageToAndBackPython);
  MITK_TEST(TestPythonContextExclusivity);
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp() { MITK_INFO << " setip--------0"; }

  void TestEvaluateOperationWithResult()
  {
    MITK_INFO << "TestEvaluateOperationWithResult";
    auto pythonContext = mitk::PythonContext::New();
    MITK_INFO << "pythonContext ready";

    pythonContext->Activate();
    MITK_INFO << "pythonContext activated";

    std::string pythonCommand;
    pythonCommand.append("_mitk_stdout = io.StringIO()\n");
    pythonCommand.append("sys.stdout = sys.stderr = _mitk_stdout\n");
    pythonContext->ExecuteString(pythonCommand);
    std::string result = pythonContext->ExecuteString("print(5+5)\n");
    CPPUNIT_ASSERT_MESSAGE("Result should be 10", result == "10\n");
  }

  void TestCopyImageToAndBackPython()
  {
    auto pythonContext = mitk::PythonContext::New();
    if (pythonContext.IsNotNull())
    {
      pythonContext->Activate();
      mitk::Image::Pointer image_in = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Pic3D.nrrd"));
      mitk::Image::Pointer image_out;
      try
      {
        pythonContext->TransferBaseDataToPython(image_in.GetPointer());
      }
      catch (const mitk::Exception &e)
      {
        MITK_ERROR << e.GetDescription();
        CPPUNIT_FAIL("Error in copying mitk image to Python");
      }
      try
      {
        image_out = pythonContext->LoadImageFromPython("_mitk_image");
      }
      catch (const mitk::Exception &e)
      {
        MITK_ERROR << e.GetDescription();
        CPPUNIT_FAIL("Error in copying mitk image from Python");
      }
      CPPUNIT_ASSERT_MESSAGE("copy an image to python and back should result in equal image",
                             mitk::Equal(*image_in, *image_out, mitk::eps, true));
    }
    else
    {
      CPPUNIT_FAIL("Error occured while TestCopyImageToAndBackPython");
    }
  }

  void TestPythonContextExclusivity()
  {
    auto pythonContext_1 = mitk::PythonContext::New();
    auto pythonContext_2 = mitk::PythonContext::New();
    if (pythonContext_1.IsNotNull() && pythonContext_2.IsNotNull())
    {
      try
      {
        pythonContext_1->ExecuteString("test_var_context_1 = 10\n");
        pythonContext_2->ExecuteString("test_var_context_2 = 20\n");
      }
      catch (const mitk::Exception &e)
      {
        MITK_ERROR << e.GetDescription();
        CPPUNIT_FAIL("Error in executing commands in python");
      }
      bool isVarExists = pythonContext_1->HasVariable("test_var_context_1");
      CPPUNIT_ASSERT_MESSAGE("test_var_context_1 should be found in context 1", isVarExists);

      isVarExists = pythonContext_1->HasVariable("test_var_context_2");
      CPPUNIT_ASSERT_MESSAGE("test_var_context_2 should not be found in context 1", !isVarExists);

      isVarExists = pythonContext_2->HasVariable("test_var_context_2");
      CPPUNIT_ASSERT_MESSAGE("test_var_context_2 should be found in context 2", isVarExists);

      isVarExists = pythonContext_2->HasVariable("test_var_context_1");
      CPPUNIT_ASSERT_MESSAGE("test_var_context_1 should not be found in context 2", !isVarExists);
    }
    else
    {
      CPPUNIT_FAIL("Error occured while TestCopyImageToAndBackPython");
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPython)
