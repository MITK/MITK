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

#include "cherryPlatformException.h"

#include <typeinfo>

namespace cherry {

POCO_IMPLEMENT_EXCEPTION(PlatformException, Poco::RuntimeException, "Platform exception");

POCO_IMPLEMENT_EXCEPTION(ManifestException, PlatformException, "Manifest syntax error");
POCO_IMPLEMENT_EXCEPTION(BundleException, PlatformException, "Bundle error");
POCO_IMPLEMENT_EXCEPTION(BundleStateException, PlatformException, "Bundle state invalid");
POCO_IMPLEMENT_EXCEPTION(BundleVersionConflictException, PlatformException, "Bundle version conflict");
POCO_IMPLEMENT_EXCEPTION(BundleLoadException, PlatformException, "Bundle load error");
POCO_IMPLEMENT_EXCEPTION(BundleResolveException, PlatformException, "Cannot resolve bundle");

POCO_IMPLEMENT_EXCEPTION(CoreException, PlatformException, "Core exception");
POCO_IMPLEMENT_EXCEPTION(InvalidServiceObjectException, CoreException, "Invalid service object");

}
