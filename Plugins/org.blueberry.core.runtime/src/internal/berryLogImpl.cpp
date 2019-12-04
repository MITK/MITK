/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryLogImpl.h"

#include "berryIStatus.h"

#include <ctkPlugin.h>
#include <mbilog.h>

namespace berry {

LogImpl::LogImpl(const QSharedPointer<ctkPlugin>& plugin)
  : plugin(plugin)
{
}

LogImpl::~LogImpl()
{
}

void LogImpl::AddLogListener(ILogListener* /*listener*/)
{
}

QSharedPointer<ctkPlugin> LogImpl::GetBundle() const
{
  return plugin;
}

void LogImpl::Log(const SmartPointer<IStatus>& status)
{
  switch (status->GetSeverity())
  {
  case IStatus::WARNING_TYPE:
    mbilog::PseudoStream(mbilog::Warn,
                         qPrintable(status->GetFileName()),
                         status->GetLineNumber(),
                         qPrintable(status->GetMethodName()))(qPrintable(plugin->getSymbolicName()))
        << status->ToString().toStdString();
    break;
  case IStatus::ERROR_TYPE:
    mbilog::PseudoStream(mbilog::Error,
                         qPrintable(status->GetFileName()),
                         status->GetLineNumber(),
                         qPrintable(status->GetMethodName()))(qPrintable(plugin->getSymbolicName()))
        << status->ToString().toStdString();
    break;
  default:
    mbilog::PseudoStream(mbilog::Info,
                         qPrintable(status->GetFileName()),
                         status->GetLineNumber(),
                         qPrintable(status->GetMethodName()))(qPrintable(plugin->getSymbolicName()))
        << status->ToString().toStdString();
    break;
  }
}

void LogImpl::RemoveLogListener(ILogListener* /*listener*/)
{
}

}
