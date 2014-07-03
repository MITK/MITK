/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkSchedulableProcess.h"

mitk::SchedulableProcess::SchedulableProcess(int priority)
  : m_Priority(priority),
    m_LastTimeSliceInMSec(-1)
{
}

mitk::SchedulableProcess::~SchedulableProcess()
{
}

int mitk::SchedulableProcess::GetPriority() const
{
  return m_Priority;
}

int mitk::SchedulableProcess::GetLastTimeSliceInMSec() const
{
  return m_LastTimeSliceInMSec;
}

void mitk::SchedulableProcess::SetLastTimeSliceInMSec(int lastTimeSliceInMSec)
{
  m_LastTimeSliceInMSec = lastTimeSliceInMSec;
}
