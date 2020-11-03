/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <ctime>
#include <iomanip>
#include <list>

#include "mbilogBackendCout.h"

mbilog::BackendCout::BackendCout()
{
  m_useFullOutput = false;
}

mbilog::BackendCout::~BackendCout()
{
}

void mbilog::BackendCout::SetFull(bool full)
{
  m_useFullOutput = full;
}

void mbilog::BackendCout::ProcessMessage(const mbilog::LogMessage &l)
{
  if (m_useFullOutput)
    FormatFull(l);
  else
    FormatSmart(l);
}

mbilog::OutputType mbilog::BackendCout::GetOutputType() const
{
  return mbilog::Console;
}
