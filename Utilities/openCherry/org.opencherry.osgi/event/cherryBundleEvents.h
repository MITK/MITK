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
  Message<const BundleEvent> bundleInstalled;
  Message<const BundleEvent> bundleLoaded;
  Message<const BundleEvent> bundleResolved;
  Message<const BundleEvent> bundleResolving;
  Message<const BundleEvent> bundleStarted;
  Message<const BundleEvent> bundleStarting;
  Message<const BundleEvent> bundleStopped;
  Message<const BundleEvent> bundleStopping;
  Message<const BundleEvent> bundleUninstalled;
  Message<const BundleEvent> bundleUninstalling;
  Message<const BundleEvent> bundleUnloaded;
};

} // namespace cherry

#endif /*CHERRYBUNDLEEVENTS_H_*/
