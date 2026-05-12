/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPythonContext.h>
#include <mitkPythonHelper.h>
#include <mitkTestingConfig.h>
#include <mitkTestingMacros.h>

#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

namespace
{
  std::string QuotePath(const fs::path& path)
  {
    return "\"" + path.string() + "\"";
  }

  int RunPythonCommand(const fs::path& pythonExecutable, const std::vector<std::string>& arguments)
  {
    std::ostringstream command;

#if defined(_WIN32)
    command << "if 1==1 ";
#endif

    command << QuotePath(pythonExecutable);

    for (const auto& argument : arguments)
      command << ' ' << argument;

    MITK_INFO << "Running command: " << command.str();
    return std::system(command.str().c_str());
  }

  void EnsurePytestInstalled()
  {
    const auto pythonExecutable = mitk::PythonHelper::GetExecutablePath();
    MITK_TEST_CONDITION_REQUIRED(!pythonExecutable.empty(), "Virtual environment Python executable is available")

    if (RunPythonCommand(pythonExecutable, {"-c", "\"import pytest, jsonschema\""}) == 0)
    {
      MITK_INFO << "pytest and jsonschema are already present in the dedicated test virtual environment.";
      return;
    }

    MITK_TEST_CONDITION_REQUIRED(
      RunPythonCommand(pythonExecutable, {"-m", "ensurepip", "--upgrade"}) == 0,
      "Bootstrapping pip in the dedicated test virtual environment")

    MITK_TEST_CONDITION_REQUIRED(
      RunPythonCommand(pythonExecutable, {"-m", "pip", "install", "--disable-pip-version-check", "pytest", "jsonschema"}) == 0,
      "Installing pytest and jsonschema in the dedicated test virtual environment")
  }
}

int mitkPythonBindingsTest(int, char*[])
{
  MITK_TEST_BEGIN("mitkPythonBindingsTest")

  const fs::path pytestRunner = fs::path(MITK_PYTHON_PYTEST_RUNNER);
  MITK_TEST_CONDITION_REQUIRED(fs::exists(pytestRunner), "Pytest runner script exists in the source tree")

  mitk::PythonContext pythonContext("mitk_pytest");

  EnsurePytestInstalled();

  bool activateOk = true;
  try { pythonContext.Activate(); }
  catch (const mitk::Exception& e) { MITK_ERROR << e.GetDescription(); activateOk = false; }
  MITK_TEST_CONDITION_REQUIRED(activateOk, "Python context activated (numpy + mitk importable)")

  bool setDataDirOk = true;
  try
  {
    pythonContext.Execute("_mitk_data_dir = '" + std::string(MITK_DATA_DIR) + "'");
  }
  catch (const mitk::Exception& e) { MITK_ERROR << e.GetDescription(); setDataDirOk = false; }
  MITK_TEST_CONDITION_REQUIRED(setDataDirOk, "Exposed MITK test data directory to the Python binding tests")

  bool pytestOk = true;
  try { pythonContext.ExecuteFile(pytestRunner); }
  catch (const mitk::Exception& e) { MITK_ERROR << e.GetDescription(); pytestOk = false; }
  MITK_TEST_CONDITION_REQUIRED(pytestOk, "pytest suite passed")

  MITK_TEST_END()
}
