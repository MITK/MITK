/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkProcessExecutor.h"

#include <fstream>
#include <iostream>
#include <itksys/Process.h>
#include <itksys/SystemTools.hxx>

namespace mitk
{
  std::string ProcessExecutor::getOSDependendExecutableName(const std::string &name)
  {
#if defined(_WIN32)

    if (itksys::SystemTools::GetFilenameLastExtension(name).empty())
    {
      return name + ".exe";
    }

    return name;

#else
    auto result = itksys::SystemTools::GetFilenamePath(name);
    if (ensureCorrectOSPathSeparator(result).empty())
    {
      return "./" + name;
    }
    else
    {
      return name;
    }

#endif
  };

  std::string ProcessExecutor::ensureCorrectOSPathSeparator(const std::string &path)
  {
    std::string ret = path;

#ifdef _WIN32
    const std::string curSep = "\\";
    const char wrongSep = '/';
#else
    const std::string curSep = "/";
    const char wrongSep = '\\';
#endif

    std::string::size_type pos = ret.find_first_of(wrongSep);

    while (pos != std::string::npos)
    {
      ret.replace(pos, 1, curSep);

      pos = ret.find_first_of(wrongSep);
    }

    return ret;
  };

  int ProcessExecutor::getExitValue() { return this->_exitValue; };

  bool ProcessExecutor::execute(const std::string &executionPath, const ArgumentListType &argumentList)
  {
    // convert to char* array with terminating null element;
    const char **pArguments = new const char *[argumentList.size() + 1];
    pArguments[argumentList.size()] = nullptr;

    for (ArgumentListType::size_type index = 0; index < argumentList.size(); ++index)
    {
      pArguments[index] = argumentList[index].c_str();
    }

    bool normalExit = false;

    try
    {
      itksysProcess *processID = itksysProcess_New();
      itksysProcess_SetCommand(processID, pArguments);

      itksysProcess_SetWorkingDirectory(processID, executionPath.c_str());

      if (this->m_SharedOutputPipes)
      {
        itksysProcess_SetPipeShared(processID, itksysProcess_Pipe_STDOUT, 1);
        itksysProcess_SetPipeShared(processID, itksysProcess_Pipe_STDERR, 1);
      }

      itksysProcess_Execute(processID);

      char *rawOutput = nullptr;
      int outputLength = 0;
      while (true)
      {
        int dataStatus = itksysProcess_WaitForData(processID, &rawOutput, &outputLength, nullptr);

        if (dataStatus == itksysProcess_Pipe_STDOUT)
        {
          std::string data(rawOutput, outputLength);
          this->InvokeEvent(ExternalProcessStdOutEvent(data));
        }
        else if (dataStatus == itksysProcess_Pipe_STDERR)
        {
          std::string data(rawOutput, outputLength);
          this->InvokeEvent(ExternalProcessStdErrEvent(data));
        }
        else
        {
          break;
        }
      }

      itksysProcess_WaitForExit(processID, nullptr);

      auto state = static_cast<itksysProcess_State_e>(itksysProcess_GetState(processID));

      normalExit = (state == itksysProcess_State_Exited);
      this->_exitValue = itksysProcess_GetExitValue(processID);
    }
    catch (...)
    {
      delete[] pArguments;
      throw;
    }

    delete[] pArguments;

    return normalExit;
  };

  bool ProcessExecutor::execute(const std::string &executionPath,
                                const std::string &executableName,
                                ArgumentListType argumentList)
  {
    std::string executableName_OS = getOSDependendExecutableName(executableName);
    argumentList.insert(argumentList.begin(), executableName_OS);

    return execute(executionPath, argumentList);
  };

  ProcessExecutor::ProcessExecutor()
  {
    this->_exitValue = 0;
    this->m_SharedOutputPipes = false;
  };

  ProcessExecutor::~ProcessExecutor() = default;
} // namespace mitk
