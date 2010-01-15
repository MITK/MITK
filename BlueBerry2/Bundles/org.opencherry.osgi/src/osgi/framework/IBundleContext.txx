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

#ifndef OSGI_FRAMEWORK_IBUNDLECONTEXT_TXX_
#define OSGI_FRAMEWORK_IBUNDLECONTEXT_TXX_

#include "cherryPlatform.h"
#include "service/cherryServiceRegistry.h"

namespace osgi {

namespace framework {

template<class S>
SmartPointer<S> IBundleContext::GetService(const std::string& id) const
{
  return cherry::Platform::GetServiceRegistry().GetServiceById<S>(id);
}

} }

#endif /*OSGI_FRAMEWORK_IBUNDLECONTEXT_TXX_*/
