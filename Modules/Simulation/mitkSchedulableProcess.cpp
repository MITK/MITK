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
#include <algorithm>

mitk::SchedulableProcess::SchedulableProcess(int priority)
  : m_Priority(priority)
{
}

mitk::SchedulableProcess::~SchedulableProcess()
{
}

int mitk::SchedulableProcess::GetPriority() const
{
  return m_Priority;
}

boost::chrono::nanoseconds mitk::SchedulableProcess::GetTotalElapsedTime() const
{
  return m_TotalElapsedTime;
}

void  mitk::SchedulableProcess::ResetTotalElapsedTime(boost::chrono::nanoseconds carryover)
{
  m_TotalElapsedTime = carryover;
}

boost::chrono::nanoseconds mitk::SchedulableProcess::GetElapsedTime() const
{
  return m_ElapsedTime;
}

void mitk::SchedulableProcess::SetElapsedTime(boost::chrono::nanoseconds elapsedTime)
{
  m_TotalElapsedTime += elapsedTime;
  m_ElapsedTime = elapsedTime;
}
