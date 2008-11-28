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

#ifndef CHERRYPLATFORMEXCEPTION_H_
#define CHERRYPLATFORMEXCEPTION_H_

#include "cherryOSGiDll.h"
#include "Poco/Exception.h"

namespace cherry {

POCO_DECLARE_EXCEPTION(CHERRY_OSGI, PlatformException, Poco::RuntimeException);

POCO_DECLARE_EXCEPTION(CHERRY_OSGI, ManifestException, PlatformException);
POCO_DECLARE_EXCEPTION(CHERRY_OSGI, BundleException, PlatformException);
POCO_DECLARE_EXCEPTION(CHERRY_OSGI, BundleStateException, PlatformException);
POCO_DECLARE_EXCEPTION(CHERRY_OSGI, BundleVersionConflictException, PlatformException);
POCO_DECLARE_EXCEPTION(CHERRY_OSGI, BundleLoadException, PlatformException);
POCO_DECLARE_EXCEPTION(CHERRY_OSGI, BundleResolveException, PlatformException);

POCO_DECLARE_EXCEPTION(CHERRY_OSGI, CoreException, PlatformException);
POCO_DECLARE_EXCEPTION(CHERRY_OSGI, InvalidServiceObjectException, CoreException);

}

#endif /*CHERRYPLATFORMEXCEPTION_H_*/
