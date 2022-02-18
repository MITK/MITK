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
  std::string ProcessExecutor::GetOSDependendExecutableName(const std::string &name)
  {
#if defined(_WIN32)

    if (itksys::SystemTools::GetFilenameLastExtension(name).empty())
    {
      return name + ".exe";
    }

    return name;

#else
    auto result = itksys::SystemTools::GetFilenamePath(name);
    if (EnsureCorrectOSPathSeparator(result).empty())
    {
      return "./" + name;
    }
    else
    {
      return name;
    }

#endif
  }

  std::string ProcessExecutor::EnsureCorrectOSPathSeparator(const std::string &path)
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
  }

  int ProcessExecutor::GetExitValue() { return this->m_ExitValue; };

  bool ProcessExecutor::Execute(const std::string &executionPath, const ArgumentListType &argumentList)
  {
    std::vector<const char*> pArguments_(argumentList.size() + 1);

    for (ArgumentListType::size_type index = 0; index < argumentList.size(); ++index)
    {
      pArguments_[index] = argumentList[index].c_str();
    }
    pArguments_.push_back(nullptr); //terminating null element as required by ITK

    bool normalExit = false;

    try
    {
      itksysProcess *processID = itksysProcess_New();
      itksysProcess_SetCommand(processID, pArguments_.data());

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
      this->m_ExitValue = itksysProcess_GetExitValue(processID);
    }
    catch (...)
    {
      throw;
    }
    return normalExit;
  };

  bool ProcessExecutor::Execute(const std::string &executionPath,
                                const std::string &executableName,
                                ArgumentListType argumentList)
  {
    std::string executableName_OS = GetOSDependendExecutableName(executableName);
    argumentList.insert(argumentList.begin(), executableName_OS);

    return Execute(executionPath, argumentList);
  }

  ProcessExecutor::ProcessExecutor()
  {
    this->m_ExitValue = 0;
    this->m_SharedOutputPipes = false;
  }

  ProcessExecutor::~ProcessExecutor() = default;
} // namespace mitk
