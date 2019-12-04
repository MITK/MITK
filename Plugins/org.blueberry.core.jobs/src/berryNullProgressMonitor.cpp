/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryNullProgressMonitor.h"

namespace berry
{

NullProgressMonitor::NullProgressMonitor() :
  m_cancelled(false)
{
}

void NullProgressMonitor::BeginTask(const std::string&  /*name*/, int  /*totalWork*/)
{
  // do nothing
}

void NullProgressMonitor::Done()
{
  // do nothing
}

void NullProgressMonitor::InternalWorked(double  /*work*/)
{
  // do nothing
}

bool NullProgressMonitor::IsCanceled()
{
  return m_cancelled;
}

void NullProgressMonitor::SetCanceled(bool newcancelled)
{
  this->m_cancelled = newcancelled;
}

void NullProgressMonitor::SetTaskName(const std::string&  /*name*/)
{
  // do nothing
}

void NullProgressMonitor::SubTask(const std::string&  /*name*/)
{
  // do nothing
}

void NullProgressMonitor::Worked(int  /*work*/)
{
  // do nothing
}

}
