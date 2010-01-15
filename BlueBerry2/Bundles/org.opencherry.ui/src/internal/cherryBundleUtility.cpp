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

#include "cherryBundleUtility.h"

#include "cherryWorkbenchPlugin.h"
#include <cherryPlatform.h>

namespace cherry
{

bool BundleUtility::IsActive(IBundle::Pointer bundle)
{
  if (!bundle)
  {
    return false;
  }
  return bundle->GetState() == IBundle::BUNDLE_ACTIVE;
}

bool BundleUtility::IsActivated(IBundle::Pointer bundle)
{
  if (bundle && bundle->GetState() == IBundle::BUNDLE_STARTING)
    return true;
    //return WorkbenchPlugin::GetDefault()->IsStarting(bundle);

  return bundle && (bundle->GetState() == IBundle::BUNDLE_ACTIVE
      || bundle->GetState() == IBundle::BUNDLE_STOPPING);
}

bool BundleUtility::IsReady(IBundle::Pointer bundle)
{
  return bundle && IsReady(bundle->GetState());
}

bool BundleUtility::IsReady(IBundle::State bundleState)
{
  return (bundleState == IBundle::BUNDLE_RESOLVED ||
          bundleState == IBundle::BUNDLE_STARTING ||
          bundleState == IBundle::BUNDLE_ACTIVE ||
          bundleState == IBundle::BUNDLE_STOPPING);
}

bool BundleUtility::IsActive(const std::string& bundleId)
{
  return IsActive(Platform::GetBundle(bundleId));
}

bool BundleUtility::IsActivated(const std::string& bundleId)
{
  return IsActivated(Platform::GetBundle(bundleId));
}

bool BundleUtility::IsReady(const std::string& bundleId)
{
  return IsReady(Platform::GetBundle(bundleId));
}

//void BundleUtility::Log(const std::string& bundleId,
//    const Poco::Exception* exception)
//{
//  IBundle::Pointer bundle = Platform::GetBundle(bundleId);
//  if (!bundle)
//  {
//    return;
//  }
//
//  IStatus status =
//      new Status(IStatus.ERROR, bundleId, IStatus.ERROR, exception.getMessage()
//          == null ? "" : exception.getMessage(), //$NON-NLS-1$
//      exception);
//  Platform.getLog(bundle).log(status);
//}

}
