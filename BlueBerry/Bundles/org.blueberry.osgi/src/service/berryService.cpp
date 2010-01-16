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
