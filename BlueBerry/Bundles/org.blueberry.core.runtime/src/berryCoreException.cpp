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

#include "berryCoreException.h"

#include "berryIStatus.h"

namespace berry {

CoreException::CoreException(const SmartPointer<IStatus>& status)
  : ctkException(status->GetMessage()), status(status)
{
  if (status->GetException())
  {
    this->setCause(*status->GetException());
  }
}

CoreException::~CoreException() throw()
{
}

const char* CoreException::name() const throw()
{
  return "Core exception";
}

CoreException* CoreException::clone() const
{
  return new CoreException(*this);
}

void CoreException::rethrow() const
{
  throw *this;
}

SmartPointer<IStatus> CoreException::GetStatus() const
{
  return status;
}

QDebug CoreException::printStackTrace(QDebug dbg) const
{
  return PrintChildren(status.GetPointer(), ctkException::printStackTrace(dbg));
}

QDebug CoreException::PrintChildren(IStatus *status, QDebug dbg) const
{
  QList<IStatus::Pointer> children = status->GetChildren();
  for (int i = 0; i < children.size(); ++i)
  {
    dbg << "Contains: " << qPrintable(children[i]->GetMessage()) << '\n';
    const ctkException* exc = children[i]->GetException();
    if (exc != NULL)
    {
      dbg << exc->printStackTrace();
    }
    PrintChildren(children[i].GetPointer(), dbg);
  }
  return dbg;
}

}
