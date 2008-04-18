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

#ifndef CHERRYBUNDLEEVENTS_H_
#define CHERRYBUNDLEEVENTS_H_

#include "../cherryDll.h"

#include "../cherryMessage.h"

#include "cherryBundleEvent.h"

namespace cherry {

struct CHERRY_API BundleEvents
{
  Message1<const BundleEvent> bundleInstalled;
  Message1<const BundleEvent> bundleLoaded;
  Message1<const BundleEvent> bundleResolved;
  Message1<const BundleEvent> bundleResolving;
  Message1<const BundleEvent> bundleStarted;
  Message1<const BundleEvent> bundleStarting;
  Message1<const BundleEvent> bundleStopped;
  Message1<const BundleEvent> bundleStopping;
  Message1<const BundleEvent> bundleUninstalled;
  Message1<const BundleEvent> bundleUninstalling;
  Message1<const BundleEvent> bundleUnloaded;
};

} // namespace cherry

#endif /*CHERRYBUNDLEEVENTS_H_*/
