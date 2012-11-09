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

#ifndef BERRYBUNDLEEVENTS_H_
#define BERRYBUNDLEEVENTS_H_

#include <org_blueberry_osgi_Export.h>

#include "berryMessage.h"

#include "berryBundleEvent.h"

namespace berry {

struct BERRY_OSGI BundleEvents
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

} // namespace berry

#endif /*BERRYBUNDLEEVENTS_H_*/
