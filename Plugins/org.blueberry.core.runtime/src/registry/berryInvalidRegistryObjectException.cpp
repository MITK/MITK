/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
