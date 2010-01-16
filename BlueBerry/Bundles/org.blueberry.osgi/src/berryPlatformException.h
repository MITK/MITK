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

#ifndef BERRYPLATFORMEXCEPTION_H_
#define BERRYPLATFORMEXCEPTION_H_

#include "berryOSGiDll.h"
#include "Poco/Exception.h"

namespace berry {

POCO_DECLARE_EXCEPTION(BERRY_OSGI, PlatformException, Poco::RuntimeException);

POCO_DECLARE_EXCEPTION(BERRY_OSGI, ManifestException, PlatformException);
POCO_DECLARE_EXCEPTION(BERRY_OSGI, BundleException, PlatformException);
POCO_DECLARE_EXCEPTION(BERRY_OSGI, BundleStateException, PlatformException);
POCO_DECLARE_EXCEPTION(BERRY_OSGI, BundleVersionConflictException, PlatformException);
POCO_DECLARE_EXCEPTION(BERRY_OSGI, BundleLoadException, PlatformException);
POCO_DECLARE_EXCEPTION(BERRY_OSGI, BundleResolveException, PlatformException);

POCO_DECLARE_EXCEPTION(BERRY_OSGI, CoreException, PlatformException);
POCO_DECLARE_EXCEPTION(BERRY_OSGI, InvalidServiceObjectException, CoreException);

}

#endif /*BERRYPLATFORMEXCEPTION_H_*/
