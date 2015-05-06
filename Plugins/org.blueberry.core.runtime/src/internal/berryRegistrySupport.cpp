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

#include "berryRegistrySupport.h"
#include "berryIStatus.h"
#include "berryLog.h"

#include <QTranslator>

namespace berry {

QString RegistrySupport::Translate(const QString& key, QTranslator* resources)
{
  if (key.isEmpty())
    return QString();
  if (resources == NULL)
    return key;
  QString trimmedKey = key.trimmed();
  if (trimmedKey.isEmpty())
    return key;
  if (trimmedKey.at(0) != '%')
    return key;
  return resources->translate("0", qPrintable(key));
}

void RegistrySupport::Log(const SmartPointer<IStatus>& status, const QString& prefix)
{
  QString message = status->GetMessage();
  IStatus::Severity severity = status->GetSeverity();

  QString statusMsg;
  switch (severity)
  {
  case IStatus::ERROR_TYPE :
    statusMsg = "Error: ";
    break;
  case IStatus::WARNING_TYPE :
    statusMsg = "Warning: ";
    break;
  default :
    statusMsg = "Log: ";
    break;
  }
  statusMsg += message;

  if (!prefix.isEmpty())
    statusMsg = prefix + statusMsg;

  switch (severity)
  {
  case IStatus::ERROR_TYPE :
    BERRY_ERROR << statusMsg;
    break;
  case IStatus::WARNING_TYPE :
    BERRY_WARN << statusMsg;
    break;
  default :
    BERRY_INFO << statusMsg;
    break;
  }

  // print out children as well
  QList<IStatus::Pointer> children = status->GetChildren();
  if (!children.empty())
  {
    QString newPrefix;
    if (prefix.isEmpty())
      newPrefix = "\t";
    else
      newPrefix = prefix + "\t";
    for (int i = 0; i < children.size(); i++)
    {
      Log(children[i], newPrefix);
    }
  }
}

}
