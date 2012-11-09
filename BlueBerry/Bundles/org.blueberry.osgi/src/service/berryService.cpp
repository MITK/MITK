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

#include "berryService.h"

#include <string>

namespace berry {

Service::~Service()
{

}

bool Service::IsA(const std::type_info& type) const
{
  std::string name(this->GetType().name());
  return name == type.name();
}

const std::type_info& Service::GetType() const
{
  return typeid(Service);
}

}  // berry
