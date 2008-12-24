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

#ifndef CHERRYBUNDLEEVENT_H_
#define CHERRYBUNDLEEVENT_H_

#include "../cherryOSGiDll.h"

#include "../cherrySmartPointer.h"

namespace cherry {

struct IBundle;

class CHERRY_OSGI BundleEvent
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

} // namespace cherry

#endif /*CHERRYBUNDLEEVENT_H_*/
