/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryPlatformException.h"

namespace berry {

CTK_IMPLEMENT_EXCEPTION(PlatformException, ctkRuntimeException, "Platform exception")

CTK_IMPLEMENT_EXCEPTION(ManifestException, PlatformException, "Manifest syntax error")
CTK_IMPLEMENT_EXCEPTION(BundleException, PlatformException, "Bundle error")
CTK_IMPLEMENT_EXCEPTION(BundleStateException, PlatformException, "Bundle state invalid")
CTK_IMPLEMENT_EXCEPTION(BundleVersionConflictException, PlatformException, "Bundle version conflict")
CTK_IMPLEMENT_EXCEPTION(BundleLoadException, PlatformException, "Bundle load error")
CTK_IMPLEMENT_EXCEPTION(BundleResolveException, PlatformException, "Cannot resolve bundle")

CTK_IMPLEMENT_EXCEPTION(CoreException, PlatformException, "Core exception")
CTK_IMPLEMENT_EXCEPTION(InvalidServiceObjectException, CoreException, "Invalid service object")

}
