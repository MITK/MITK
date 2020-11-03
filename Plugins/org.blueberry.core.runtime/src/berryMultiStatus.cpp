/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryMultiStatus.h"

#include <QTextStream>

namespace berry
{

IStatus::Severity MultiStatus::GetMaxSeverity(const QList<IStatus::Pointer>& children) const
{
  Severity maxSeverity = this->GetSeverity();
  for (int i = 0; i < children.size(); i++)
  {
    Q_ASSERT(children[i]);
    Severity severity = children[i]->GetSeverity();
    if (severity > maxSeverity)
    maxSeverity = severity;
  }
  return maxSeverity;
}

MultiStatus::MultiStatus(const QString& pluginId, int code,
                         const QList<IStatus::Pointer>& newChildren,
                         const QString& message, const SourceLocation& sl)
  : Status(OK_TYPE, pluginId, code, message, sl), children(newChildren)
{
  this->SetSeverity(GetMaxSeverity(children));
}

MultiStatus::MultiStatus(const QString& pluginId, int code,
                         const QList<IStatus::Pointer>& newChildren,
                         const QString& message, const ctkException& exception,
                         const SourceLocation& sl)
  : Status(OK_TYPE, pluginId, code, message, exception, sl), children(newChildren)
{
  this->SetSeverity(GetMaxSeverity(children));
}

MultiStatus::MultiStatus(const QString& pluginId, int code, const QString& message,
                         const SourceLocation& sl)
: Status(OK_TYPE, pluginId, code, message, sl)
{
}

MultiStatus::MultiStatus(const QString& pluginId, int code,
                         const QString& message, const ctkException& exception,
                         const SourceLocation& sl)
: Status(OK_TYPE, pluginId, code, message, exception, sl)
{
}

void MultiStatus::Add(IStatus::Pointer status)
{
  poco_assert(status);
  children.push_back(status);
  Severity newSev = status->GetSeverity();
  if (newSev > this->GetSeverity())
  {
    this->SetSeverity(newSev);
  }
}

void MultiStatus::AddAll(IStatus::Pointer status)
{
  Q_ASSERT(status);
  QList<IStatus::Pointer> statuses(status->GetChildren());
  for (int i = 0; i < statuses.size(); i++)
  {
    this->Add(statuses[i]);
  }
}

QList<IStatus::Pointer> MultiStatus::GetChildren() const
{
  return children;
}

bool MultiStatus::IsMultiStatus() const
{
  return true;
}

void MultiStatus::Merge(const IStatus::Pointer& status)
{
  poco_assert(status);
  if (!status->IsMultiStatus())
  {
    this->Add(status);
  }
  else
  {
    this->AddAll(status);
  }
}

QString MultiStatus::ToString() const
{
  QString str;
  QTextStream buf(&str);
  buf << Status::ToString() << " children=[";
  for (int i = 0; i < children.size(); i++)
  {
    if (i != 0)
    {
      buf << " ";
    }
    buf << children[i]->ToString();
  }
  buf << "]";
  return str;
}

}
