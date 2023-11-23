/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegmentAnythingProcessExecutor.h"

#include <itksys/SystemTools.hxx>

bool mitk::SegmentAnythingProcessExecutor::Execute(const std::string &executionPath, const ArgumentListType &argumentList)
{
  std::vector<const char *> pArguments_(argumentList.size() + 1);

  for (ArgumentListType::size_type index = 0; index < argumentList.size(); ++index)
  {
    pArguments_[index] = argumentList[index].c_str();
  }
  pArguments_.push_back(nullptr); // terminating null element as required by ITK
  bool normalExit = false;
  try
  {
    m_ProcessID = itksysProcess_New();
    itksysProcess_SetCommand(m_ProcessID, pArguments_.data());

    /* Place the process in a new process group for seamless interruption when required. */
    itksysProcess_SetOption(m_ProcessID, itksysProcess_Option_CreateProcessGroup, 1);

    itksysProcess_SetWorkingDirectory(m_ProcessID, executionPath.c_str());
    if (this->m_SharedOutputPipes)
    {
      itksysProcess_SetPipeShared(m_ProcessID, itksysProcess_Pipe_STDOUT, 1);
      itksysProcess_SetPipeShared(m_ProcessID, itksysProcess_Pipe_STDERR, 1);
    }
    itksysProcess_Execute(m_ProcessID);
    char *rawOutput = nullptr;
    int outputLength = 0;
    double timer = m_Timeout;
    while (!m_Stop)
    {
      double *timeout = &timer;
      *timeout = m_Timeout; //re-assigning timeout since itksysProcess calls will tamper with input timeout argument.
      int dataStatus = itksysProcess_WaitForData(m_ProcessID, &rawOutput, &outputLength, timeout);
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
    }
    timer = m_Timeout; //re-assigning timeout since itksysProcess calls will tamper with input timeout argument.
    itksysProcess_Kill(m_ProcessID);
    itksysProcess_WaitForExit(m_ProcessID, &timer);
    auto state = static_cast<itksysProcess_State_e>(itksysProcess_GetState(m_ProcessID));
    normalExit = (state == itksysProcess_State_Exited);
    this->m_ExitValue = itksysProcess_GetExitValue(m_ProcessID);
  }
  catch (...)
  {
    throw;
  }
  return normalExit;
};

mitk::SegmentAnythingProcessExecutor::SegmentAnythingProcessExecutor(double &timeout)
{
  this->SetTimeout(timeout);
}

void mitk::SegmentAnythingProcessExecutor::SetTimeout(double &timeout)
{
  m_Timeout = timeout;
}
