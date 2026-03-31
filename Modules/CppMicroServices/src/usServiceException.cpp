/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usServiceException.h>

#include <ostream>

namespace us {

ServiceException::ServiceException(const std::string& msg, const Type& type)
  : std::runtime_error(msg), type(type)
{

}

ServiceException::ServiceException(const ServiceException& o)
  : std::runtime_error(o), type(o.type)
{

}

ServiceException& ServiceException::operator=(const ServiceException& o)
{
  std::runtime_error::operator=(o);
  this->type = o.type;
  return *this;
}

ServiceException::Type ServiceException::GetType() const
{
  return type;
}

}

using namespace us;

std::ostream& operator<<(std::ostream& os, const ServiceException& exc)
{
  return os << "ServiceException: " << exc.what();
}
