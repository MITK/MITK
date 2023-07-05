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
#include <mitkLog.h>

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
    mitk::PseudoLogStream(mitk::LogLevel::Warn,
                         qPrintable(status->GetFileName()),
                         status->GetLineNumber(),
                         qPrintable(status->GetMethodName()))(qPrintable(plugin->getSymbolicName()))
        << status->ToString().toStdString();
    break;
  case IStatus::ERROR_TYPE:
    mitk::PseudoLogStream(mitk::LogLevel::Error,
                         qPrintable(status->GetFileName()),
                         status->GetLineNumber(),
                         qPrintable(status->GetMethodName()))(qPrintable(plugin->getSymbolicName()))
        << status->ToString().toStdString();
    break;
  default:
    mitk::PseudoLogStream(mitk::LogLevel::Info,
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
