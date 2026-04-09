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
#include <mitkIOUtil.h>
#include <mitkPythonContext.h>

class mitkPythonTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPythonTestSuite);
  MITK_TEST(TestExecuteAndGetVariable);
  MITK_TEST(TestBindImageToPython);
  MITK_TEST(TestImageAsNumpyAccessors);
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

  void TestImageAsNumpyAccessors()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    auto image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Pic3D.nrrd"));
    pythonContext.BindImage(image, "test_image");

    // Default: as_numpy() returns a writeable view (numpy convention).
    // The direct-access path used by default holds no MITK accessor lock,
    // so there is no cost to defaulting to writeable. For a read-only
    // view, callers should use the .array property or np.asarray(img),
    // or pass writeable=False explicitly (see below).
    pythonContext.Execute(
      "import gc\n"
      "arr_default = test_image.as_numpy()\n"
      "default_writeable = arr_default.flags.writeable\n"
      "del arr_default\n"
    );

    auto defaultWriteable = pythonContext.GetVariableAsBool("default_writeable");
    CPPUNIT_ASSERT_MESSAGE("default_writeable should exist", defaultWriteable.has_value());
    CPPUNIT_ASSERT_MESSAGE("as_numpy() default should be writeable", defaultWriteable.value());

    // Explicit read-only: as_numpy(writeable=False) returns a non-writeable view.
    pythonContext.Execute(
      "arr_ro = test_image.as_numpy(writeable=False)\n"
      "ro_shape = arr_ro.shape\n"
      "ro_writeable = arr_ro.flags.writeable\n"
      "del arr_ro\n"
    );

    auto roWriteable = pythonContext.GetVariableAsBool("ro_writeable");
    CPPUNIT_ASSERT_MESSAGE("ro_writeable should exist", roWriteable.has_value());
    CPPUNIT_ASSERT_MESSAGE("as_numpy(writeable=False) must not be writeable", !roWriteable.value());

    // Explicit writeable: round-trip a write through as_numpy(writeable=True).
    pythonContext.Execute(
      "arr_rw = test_image.as_numpy(writeable=True)\n"
      "rw_writeable = arr_rw.flags.writeable\n"
      "original_value = int(arr_rw[0, 0, 0])\n"
      "arr_rw[0, 0, 0] = 42\n"
      "modified_value = int(arr_rw[0, 0, 0])\n"
      "del arr_rw\n"
    );

    auto rwWriteable = pythonContext.GetVariableAsBool("rw_writeable");
    CPPUNIT_ASSERT_MESSAGE("rw_writeable should exist", rwWriteable.has_value());
    CPPUNIT_ASSERT_MESSAGE("as_numpy(writeable=True) must be writeable", rwWriteable.value());

    auto modifiedValue = pythonContext.GetVariableAsInt("modified_value");
    CPPUNIT_ASSERT_MESSAGE("modified_value should exist", modifiedValue.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Modified value should be 42", 42, modifiedValue.value());
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

MITK_TEST_SUITE_REGISTRATION(mitkPython)
