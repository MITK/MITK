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

#include "cherryServiceLocatorCreator.h"

#include "cherryServiceLocator.h"
#include "../services/cherryIServiceFactory.h"
#include "../services/cherryIDisposable.h"

namespace cherry {

SmartPointer<IServiceLocator>
ServiceLocatorCreator::CreateServiceLocator(
      const WeakPointer<IServiceLocator> parent,
      const SmartPointer<const IServiceFactory> factory,
      WeakPointer<IDisposable> owner)
{
  IServiceLocator::WeakPtr weakParent(parent);
  IServiceLocator::Pointer locator(new ServiceLocator(weakParent, factory, owner));
  return locator;
}

}
