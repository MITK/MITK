/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "berryStatus.h"

#include "internal/berryIRuntimeConstants.h"

#include <sstream>

namespace berry
{

const std::vector<IStatus::Pointer> Status::theEmptyStatusArray = std::vector<IStatus::Pointer>();

const IStatus::Pointer Status::OK_STATUS(new Status(IStatus::OK_TYPE,
    IRuntimeConstants::PI_RUNTIME(), 0, "OK"));
const IStatus::Pointer Status::CANCEL_STATUS(new Status(IStatus::CANCEL_TYPE,
    IRuntimeConstants::PI_RUNTIME(), 1, ""));

Status::Status(const Severity& severity, const std::string& pluginId, int code,
    const std::string& message, const std::exception& exception)
{
  SetSeverity(severity);
  SetPlugin(pluginId);
  SetCode(code);
  SetMessage(message);
  SetException(exception);
}

Status::Status(const Severity& severity, const std::string& pluginId,
    const std::string& message, const std::exception& exception)
{
  SetSeverity(severity);
  SetPlugin(pluginId);
  SetMessage(message);
  SetException(exception);
  SetCode(0);
}

std::vector<IStatus::Pointer> Status::GetChildren() const
{
  return theEmptyStatusArray;
}

int Status::GetCode() const
{
  return code;
}

std::exception Status::GetException() const
{
  return exception;
}

/* (Intentionally not javadoc'd)
 * Implements the corresponding method on <code>IStatus</code>.
 */
std::string Status::GetMessage() const
{
  return message;
}

std::string Status::GetPlugin() const
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

void Status::SetCode(int code)
{
  this->code = code;
}

void Status::SetException(const std::exception& exception)
{
  this->exception = exception;
}

void Status::SetMessage(const std::string& message)
{
  this->message = message;
}

void Status::SetPlugin(const std::string& pluginId)
{
  poco_assert(!pluginId.empty())
;  this->pluginId = pluginId;
}

void Status::SetSeverity(const Severity& severity)
{
  this->severity = severity;
}

std::string Status::ToString() const
{

  std::stringstream ss;
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

  ss << ": " << pluginId << " code=" << code << " " << message << " " << exception.what();
  return ss.str();
}

}
