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


#ifndef CHERRYSERVICELOCATORCREATOR_H_
#define CHERRYSERVICELOCATORCREATOR_H_

#include "cherryIServiceLocatorCreator.h"

namespace cherry {

/**
 * A simple service locator creator.
 *
 * @since 3.4
 */
class ServiceLocatorCreator : public IServiceLocatorCreator
{

public:

  cherryObjectMacro(ServiceLocatorCreator)

  SmartPointer<IServiceLocator> CreateServiceLocator(
      const WeakPointer<IServiceLocator> parent,
      const SmartPointer<const IServiceFactory> factory,
      WeakPointer<IDisposable> owner);
};

}

#endif /* CHERRYSERVICELOCATORCREATOR_H_ */
