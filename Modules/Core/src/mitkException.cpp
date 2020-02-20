/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkException.h"

void mitk::Exception::AddRethrowData(const char *file, unsigned int lineNumber, const char *message)
{
  mitk::Exception::ReThrowData data = {file, lineNumber, message};
  this->m_RethrowData.push_back(data);
}

int mitk::Exception::GetNumberOfRethrows()
{
  return (int)m_RethrowData.size();
}

void mitk::Exception::GetRethrowData(int rethrowNumber, std::string &file, int &line, std::string &message)
{
  if ((rethrowNumber >= (int)m_RethrowData.size()) || (rethrowNumber < 0))
  {
    file = "";
    line = 0;
    message = "";
    return;
  }

  file = m_RethrowData.at(rethrowNumber).RethrowClassname;
  line = m_RethrowData.at(rethrowNumber).RethrowLine;
  message = m_RethrowData.at(rethrowNumber).RethrowMessage;
}

std::ostream &mitk::operator<<(std::ostream &os, const mitk::Exception &e)
{
  os << e.GetDescription();
  return os;
}
