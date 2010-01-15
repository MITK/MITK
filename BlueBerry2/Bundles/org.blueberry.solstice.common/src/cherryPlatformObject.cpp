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

#include "cherryPlatformObject.h"

#include <cherryPlatform.h>
#include "cherryIAdapterManager.h"

namespace cherry {

PlatformObject::PlatformObject()
 : Object()
{

}

Poco::Any PlatformObject::GetAdapter(const std::string& adapter)
{
  IAdapterManager::Pointer adapterManager = Platform::GetServiceRegistry().GetServiceById<IAdapterManager>(IAdapterManager::GetStaticClassName());
  if (adapterManager)
    return adapterManager->GetAdapter(Object::Pointer(this), adapter);

  return Poco::Any();
}

}
