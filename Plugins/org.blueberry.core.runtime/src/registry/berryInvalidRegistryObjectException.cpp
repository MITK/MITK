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

#include "berryInvalidRegistryObjectException.h"

namespace berry {

InvalidRegistryObjectException::InvalidRegistryObjectException()
  : ctkRuntimeException("Invalid registry object")
{
}

InvalidRegistryObjectException::~InvalidRegistryObjectException() throw()
{
}

const char* InvalidRegistryObjectException::name() const throw()
{
  return "berry::InvalidRegistryObjectException";
}

InvalidRegistryObjectException* InvalidRegistryObjectException::clone() const
{
  return new InvalidRegistryObjectException(*this);
}

void InvalidRegistryObjectException::rethrow() const
{
  throw *this;
}

}
