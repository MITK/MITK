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

#ifndef BERRYBUNDLEEVENT_H_
#define BERRYBUNDLEEVENT_H_

#include <org_blueberry_osgi_Export.h>

#include "berrySmartPointer.h"

namespace berry {

struct IBundle;

class BERRY_OSGI BundleEvent
{
public:
  enum EventKind { EV_BUNDLE_INSTALLED, EV_BUNDLE_LOADED, EV_BUNDLE_RESOLVING,
    EV_BUNDLE_RESOLVED, EV_BUNDLE_STARTING, EV_BUNDLE_STARTED, EV_BUNDLE_STOPPING,
    EV_BUNDLE_STOPPED, EV_BUNDLE_UNINSTALLING, EV_BUNDLE_UNINSTALLED, EV_BUNDLE_UNLOADED
  };

  BundleEvent(SmartPointer<IBundle> bundle, EventKind what);
  BundleEvent(IBundle* bundle, EventKind what);

  BundleEvent(const BundleEvent& event);

  virtual ~BundleEvent();

  BundleEvent& operator= (const BundleEvent& event);


  SmartPointer<const IBundle> GetBundle() const;
  EventKind What() const;

private:
  SmartPointer<const IBundle> m_Bundle;
  EventKind m_What;
};

} // namespace berry

#endif /*BERRYBUNDLEEVENT_H_*/
