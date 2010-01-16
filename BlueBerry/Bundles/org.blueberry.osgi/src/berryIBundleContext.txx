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

#ifndef BERRYIBUNDLECONTEXT_TXX_
#define BERRYIBUNDLECONTEXT_TXX_

#include "berryPlatform.h"
#include "service/berryServiceRegistry.h"

namespace berry
{

template<class S>
SmartPointer<S> IBundleContext::GetService(const std::string& id) const
{
  return Platform::GetServiceRegistry().GetServiceById<S>(id);
}

}

#endif /*BERRYIBUNDLECONTEXT_TXX_*/
