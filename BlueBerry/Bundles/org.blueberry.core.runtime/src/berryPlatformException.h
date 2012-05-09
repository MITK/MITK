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

#ifndef BERRYPLATFORMEXCEPTION_H_
#define BERRYPLATFORMEXCEPTION_H_

#include <org_blueberry_core_runtime_Export.h>

#include <ctkException.h>

namespace berry {

CTK_DECLARE_EXCEPTION(org_blueberry_core_runtime_EXPORT, PlatformException, ctkRuntimeException)

CTK_DECLARE_EXCEPTION(org_blueberry_core_runtime_EXPORT, ManifestException, PlatformException)
CTK_DECLARE_EXCEPTION(org_blueberry_core_runtime_EXPORT, BundleException, PlatformException)
CTK_DECLARE_EXCEPTION(org_blueberry_core_runtime_EXPORT, BundleStateException, PlatformException)
CTK_DECLARE_EXCEPTION(org_blueberry_core_runtime_EXPORT, BundleVersionConflictException, PlatformException)
CTK_DECLARE_EXCEPTION(org_blueberry_core_runtime_EXPORT, BundleLoadException, PlatformException)
CTK_DECLARE_EXCEPTION(org_blueberry_core_runtime_EXPORT, BundleResolveException, PlatformException)

CTK_DECLARE_EXCEPTION(org_blueberry_core_runtime_EXPORT, CoreException, PlatformException)
CTK_DECLARE_EXCEPTION(org_blueberry_core_runtime_EXPORT, InvalidServiceObjectException, CoreException)

}

#endif /*BERRYPLATFORMEXCEPTION_H_*/
