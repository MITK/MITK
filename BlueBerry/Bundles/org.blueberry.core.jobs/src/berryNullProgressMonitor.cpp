/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
