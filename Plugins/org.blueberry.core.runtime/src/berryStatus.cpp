/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryStatus.h"

#include "internal/berryIRuntimeConstants.h"

#include <QTextStream>

namespace berry
{

const QList<IStatus::Pointer> Status::theEmptyStatusArray = QList<IStatus::Pointer>();

const IStatus::Pointer Status::OK_STATUS(const SourceLocation &sl)
{
  IStatus::Pointer status(new Status(IStatus::OK_TYPE,
                                     IRuntimeConstants::PI_RUNTIME(), 0, "OK", sl));
  return status;
}

const IStatus::Pointer Status::CANCEL_STATUS(const SourceLocation &sl)
{
  IStatus::Pointer status(new Status(IStatus::CANCEL_TYPE,
                                     IRuntimeConstants::PI_RUNTIME(), 1, "", sl));
  return status;
}

Status::Status(const Severity& severity, const QString& pluginId, int code,
               const QString& message, const SourceLocation& sl)
  : exception(nullptr), sourceLocation(sl)
{
  SetSeverity(severity);
  SetPlugin(pluginId);
  SetCode(code);
  SetMessage(message);
}

Status::Status(const Severity& severity, const QString& pluginId, int code,
               const QString& message, const ctkException& exception,
               const SourceLocation& sl)
  : exception(nullptr), sourceLocation(sl)
{
  SetSeverity(severity);
  SetPlugin(pluginId);
  SetCode(code);
  SetMessage(message);
  SetException(exception);
}

Status::Status(const Severity& severity, const QString& pluginId,
               const QString& message, const SourceLocation& sl)
  : exception(nullptr), sourceLocation(sl)
{
  SetSeverity(severity);
  SetPlugin(pluginId);
  SetMessage(message);
  SetCode(0);
}

Status::Status(const Severity& severity, const QString& pluginId,
               const QString& message, const ctkException& exception,
               const SourceLocation& sl)
  : exception(nullptr), sourceLocation(sl)
{
  SetSeverity(severity);
  SetPlugin(pluginId);
  SetMessage(message);
  SetException(exception);
  SetCode(0);
}

QList<IStatus::Pointer> Status::GetChildren() const
{
  return theEmptyStatusArray;
}

int Status::GetCode() const
{
  return code;
}

const ctkException* Status::GetException() const
{
  return exception;
}

/* (Intentionally not javadoc'd)
 * Implements the corresponding method on <code>IStatus</code>.
 */
QString Status::GetMessage() const
{
  return message;
}

QString Status::GetPlugin() const
{
  return pluginId;
}

IStatus::Severity Status::GetSeverity() const
{
  return severity;
}

bool Status::IsMultiStatus() const
{
  return false;
}

bool Status::IsOK() const
{
  return severity == OK_TYPE;
}

bool Status::Matches(const Severities& severityMask) const
{
  return (severity & severityMask) != 0;
}

QString Status::GetFileName() const
{
  return sourceLocation.fileName;
}

QString Status::GetMethodName() const
{
  return sourceLocation.methodName;
}

int Status::GetLineNumber() const
{
  return sourceLocation.lineNumber;
}

void Status::SetCode(int code)
{
  this->code = code;
}

void Status::SetException(const ctkException& exception)
{
  delete this->exception;
  this->exception = exception.clone();
}

void Status::SetMessage(const QString& message)
{
  this->message = message;
}

void Status::SetPlugin(const QString& pluginId)
{
  Q_ASSERT(!pluginId.isEmpty());
  this->pluginId = pluginId;
}

void Status::SetSeverity(const Severity& severity)
{
  this->severity = severity;
}

QString Status::ToString() const
{
  QString str;
  QTextStream ss(&str);
  ss << "Status ";
  switch (severity)
  {
    case OK_TYPE:
    ss << "OK";
    break;
    case ERROR_TYPE:
    ss << "ERROR";
    break;
    case WARNING_TYPE:
    ss << "WARNING";
    break;
    case INFO_TYPE:
    ss << "INFO";
    break;
    case CANCEL_TYPE:
    ss << "CANCEL";
    break;
    default:
    ss << "severity=" << severity;
  }

  ss << ": " << pluginId << " code=" << code << " " << message;
  if (exception)
  {
    ss << " " << exception->what();
  }
  return str;
}

}
