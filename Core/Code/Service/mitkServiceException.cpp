/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "mitkServiceException.h"

#include <ostream>

namespace mitk
{

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

std::ostream& operator<<(std::ostream& os, const mitk::ServiceException& exc)
{
  return os << "mitk::ServiceException: " << exc.what();
}
