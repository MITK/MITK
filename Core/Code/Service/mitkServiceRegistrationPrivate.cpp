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

#include "mitkServiceRegistrationPrivate.h"

namespace mitk {

ServiceRegistrationPrivate::ServiceRegistrationPrivate(
  ModulePrivate* module, itk::LightObject* service,
  const ServiceProperties& props)
  : ref(1), service(service), module(module), reference(this),
    properties(props), available(true), unregistering(false)
{

}

ServiceRegistrationPrivate::~ServiceRegistrationPrivate()
{

}

bool ServiceRegistrationPrivate::IsUsedByModule(Module* p)
{
  return dependents.find(p) != dependents.end();
}

itk::LightObject* ServiceRegistrationPrivate::GetService()
{
  return service;
}

}
