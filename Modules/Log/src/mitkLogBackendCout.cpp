/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkLogBackendCout.h>

mitk::LogBackendCout::LogBackendCout()
  : m_UseFullOutput(false)
{
}

mitk::LogBackendCout::~LogBackendCout()
{
}

void mitk::LogBackendCout::SetFull(bool full)
{
  m_UseFullOutput = full;
}

void mitk::LogBackendCout::ProcessMessage(const LogMessage& message)
{
  if (m_UseFullOutput)
  {
    this->FormatFull(message);
  }
  else
  {
    this->FormatSmart(message);
  }
}

mitk::LogBackendCout::OutputType mitk::LogBackendCout::GetOutputType() const
{
  return OutputType::Console;
}
