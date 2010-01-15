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

#ifndef OSGI_FRAMEWORK_BUNDLEEVENTS_H_
#define OSGI_FRAMEWORK_BUNDLEEVENTS_H_

#include "cherryOSGiDll.h"

#include "Message.h"

#include "BundleEvent.h"

namespace osgi {

namespace framework {

struct CHERRY_OSGI BundleEvents
{
  Message1<const BundleEvent&> bundleInstalled;
  Message1<const BundleEvent&> bundleLoaded;
  Message1<const BundleEvent&> bundleResolved;
  Message1<const BundleEvent&> bundleResolving;
  Message1<const BundleEvent&> bundleStarted;
  Message1<const BundleEvent&> bundleStarting;
  Message1<const BundleEvent&> bundleStopped;
  Message1<const BundleEvent&> bundleStopping;
  Message1<const BundleEvent&> bundleUninstalled;
  Message1<const BundleEvent&> bundleUninstalling;
  Message1<const BundleEvent&> bundleUnloaded;
};

} } // namespace

#endif /*OSGI_FRAMEWORK_BUNDLEEVENTS_H_*/
