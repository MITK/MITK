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
#include <mitkPythonHelper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
  // Stands in for a managed virtual environment. A context needs nothing but
  // pyvenv.cfg and the site-packages folder, so no Python has to run to create
  // one. The folder is removed again on destruction.
  class ScopedFakeVirtualEnv
  {
  public:
    ScopedFakeVirtualEnv(const std::string& name, const std::string& packageName)
      : m_Name(name)
    {
      const auto venvPath = mitk::PythonHelper::GetVirtualEnvPath(name);
      CPPUNIT_ASSERT_MESSAGE("The folder for virtual environments should be known", !venvPath.empty());

      // Left over from an aborted run.
      mitk::PythonHelper::RemoveVirtualEnv(venvPath);

#if defined(_WIN32)
      const auto sitePackages = venvPath / "Lib" / "site-packages";
#else
      const auto sitePackages = venvPath / "lib" /
        ("python" + std::to_string(mitk::PythonHelper::VERSION_MAJOR) + "." + std::to_string(mitk::PythonHelper::VERSION_MINOR)) /
        "site-packages";
#endif

      fs::create_directories(sitePackages / packageName);
      std::ofstream(venvPath / "pyvenv.cfg") << "home = unused\n";
      std::ofstream(sitePackages / packageName / "__init__.py") << "\n";
    }

    ~ScopedFakeVirtualEnv()
    {
      mitk::PythonHelper::RemoveVirtualEnv(m_Name);
    }

    ScopedFakeVirtualEnv(const ScopedFakeVirtualEnv&) = delete;
    ScopedFakeVirtualEnv& operator=(const ScopedFakeVirtualEnv&) = delete;

  private:
    std::string m_Name;
  };
}

class mitkPythonContextTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPythonContextTestSuite);
  MITK_TEST(TestExecuteAndGetVariable);
  MITK_TEST(TestExecuteFile);
  MITK_TEST(TestBindImageToPython);
  MITK_TEST(TestBindFunctionToPython);
  MITK_TEST(TestPythonContextExclusivity);
  MITK_TEST(TestSeveralVirtualEnvs);
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

  void TestBindFunctionToPython()
  {
    mitk::PythonContext pythonContext;
    pythonContext.Activate();

    std::vector<std::pair<int, int>> calls;

    pythonContext.BindFunction("progress_callback", [&calls](int done, int total)
    {
      calls.emplace_back(done, total);
      return done < total;
    });

    pythonContext.Execute("a = progress_callback(1, 4)\n"
                          "b = progress_callback(4, 4)\n");

    auto a = pythonContext.GetVariableAsBool("a");
    auto b = pythonContext.GetVariableAsBool("b");

    CPPUNIT_ASSERT_MESSAGE("Result 'a' should exist", a.has_value());
    CPPUNIT_ASSERT_MESSAGE("Result 'b' should exist", b.has_value());
    CPPUNIT_ASSERT_MESSAGE("The callable's result should reach Python (true)", a.value());
    CPPUNIT_ASSERT_MESSAGE("The callable's result should reach Python (false)", !b.value());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The callable should have been called twice", std::size_t(2), calls.size());
    CPPUNIT_ASSERT_EQUAL(1, calls[0].first);
    CPPUNIT_ASSERT_EQUAL(4, calls[0].second);
    CPPUNIT_ASSERT_EQUAL(4, calls[1].first);
    CPPUNIT_ASSERT_EQUAL(4, calls[1].second);

    pythonContext.BindFunction("progress_callback", [](int, int) -> bool
    {
      throw std::runtime_error("boom");
    });

    CPPUNIT_ASSERT_THROW_MESSAGE("An exception escaping the callable should surface from Execute()",
                                 pythonContext.Execute("progress_callback(0, 1)\n"),
                                 mitk::Exception);

    pythonContext.BindFunction("progress_callback", [&pythonContext](int done, int)
    {
      pythonContext.Execute("called_back = " + std::to_string(done) + "\n");
      return true;
    });

    pythonContext.Execute("progress_callback(3, 4)\n");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The callable should be able to call back into the context",
                                 3, pythonContext.GetVariableAsInt("called_back").value_or(0));

    pythonContext.BindFunction("progress_callback", {});
    pythonContext.Execute("is_none = progress_callback is None\n");

    auto isNone = pythonContext.GetVariableAsBool("is_none");

    CPPUNIT_ASSERT_MESSAGE("Result 'is_none' should exist", isNone.has_value());
    CPPUNIT_ASSERT_MESSAGE("Binding an empty function should unbind the variable", isNone.value());
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

  void TestSeveralVirtualEnvs()
  {
    // The interpreter takes over the environment variables once, when it
    // starts. Start it before any virtual environment is activated, so that
    // every environment of this test is one that comes after the start.
    mitk::PythonContext interpreterStarter;

    const std::string venvNameA = "mitk_pythoncontext_test_a";
    const std::string venvNameB = "mitk_pythoncontext_test_b";
    const std::string packageNameA = "mitk_pythoncontext_test_package_a";
    const std::string packageNameB = "mitk_pythoncontext_test_package_b";

    const ScopedFakeVirtualEnv venvA(venvNameA, packageNameA);
    const ScopedFakeVirtualEnv venvB(venvNameB, packageNameB);

    mitk::PythonContext pythonContextA(venvNameA);
    pythonContextA.Activate(false);

    mitk::PythonContext pythonContextB(venvNameB);
    pythonContextB.Activate(false);

    pythonContextB.Execute(
      "import importlib.util\n"
      "found_a = importlib.util.find_spec('" + packageNameA + "') is not None\n"
      "found_b = importlib.util.find_spec('" + packageNameB + "') is not None\n");

    const auto foundA = pythonContextB.GetVariableAsBool("found_a");
    const auto foundB = pythonContextB.GetVariableAsBool("found_b");

    CPPUNIT_ASSERT_MESSAGE("Result 'found_a' should exist", foundA.has_value());
    CPPUNIT_ASSERT_MESSAGE("Result 'found_b' should exist", foundB.has_value());
    CPPUNIT_ASSERT_MESSAGE("The package of the environment that was activated first should be importable", foundA.value());
    CPPUNIT_ASSERT_MESSAGE("The package of the environment that was activated second should be importable", foundB.value());
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPythonContext)
