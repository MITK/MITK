/*=========================================================================

 Program:   openCherry Platform
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

#include "cherryMultiStatus.h"

#include <sstream>

namespace cherry
{

MultiStatus::MultiStatus(const std::string& pluginId, int code,
    const std::vector<IStatus::Pointer>& newChildren,
    const std::string& message, const std::exception& exception) :
  Status(OK_TYPE, pluginId, code, message, exception), children(newChildren)
{
  Severity maxSeverity = this->GetSeverity();
  for (unsigned int i = 0; i < newChildren.size(); i++)
  {
    poco_assert(newChildren[i])
;    Severity severity = newChildren[i]->GetSeverity();
    if (severity > maxSeverity)
    maxSeverity = severity;
  }
  this->SetSeverity(maxSeverity);
}

MultiStatus::MultiStatus(const std::string& pluginId, int code, const std::string& message, const std::exception& exception)
: Status(OK_TYPE, pluginId, code, message, exception)
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
  poco_assert(status);
  std::vector<IStatus::Pointer> statuses(status->GetChildren());
  for (unsigned int i = 0; i < statuses.size(); i++)
  {
    this->Add(statuses[i]);
  }
}

std::vector<IStatus::Pointer> MultiStatus::GetChildren() const
{
  return children;
}

bool MultiStatus::IsMultiStatus() const
{
  return true;
}

void MultiStatus::Merge(IStatus::Pointer status)
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

std::string MultiStatus::ToString() const
{
  std::stringstream buf;
  buf << Status::ToString() << " children=[";
  for (unsigned int i = 0; i < children.size(); i++)
  {
    if (i != 0)
    {
      buf << " ";
    }
    buf << children[i]->ToString();
  }
  buf << "]";
  return buf.str();
}

}
