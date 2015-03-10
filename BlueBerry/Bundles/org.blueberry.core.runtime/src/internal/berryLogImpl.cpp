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
