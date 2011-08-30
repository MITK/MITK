/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#ifndef MITKTRACKEDSERVICE_H
#define MITKTRACKEDSERVICE_H

#include "mitkTrackedServiceListener.h"
#include "mitkModuleAbstractTracked.h"
#include "mitkServiceEvent.h"

namespace mitk {

/**
 * This class is not intended to be used directly. It is exported to support
 * the MITK module system.
 */
template<class S, class T>
class TrackedService : public TrackedServiceListener,
    public ModuleAbstractTracked<ServiceReference, T, ServiceEvent>
{

public:
  TrackedService(ServiceTracker<S,T>* serviceTracker,
                    ServiceTrackerCustomizer<T>* customizer);

  /**
   * Method connected to service events for the
   * <code>ServiceTracker</code> class. This method must NOT be
   * synchronized to avoid deadlock potential.
   *
   * @param event <code>ServiceEvent</code> object from the framework.
   */
  void ServiceChanged(const ServiceEvent& event);

private:

  typedef ModuleAbstractTracked<ServiceReference, T, ServiceEvent> Superclass;

  ServiceTracker<S,T>* serviceTracker;
  ServiceTrackerCustomizer<T>* customizer;

  /**
   * Increment the tracking count and tell the tracker there was a
   * modification.
   *
   * @GuardedBy this
   */
  void Modified();

  /**
   * Call the specific customizer adding method. This method must not be
   * called while synchronized on this object.
   *
   * @param item Item to be tracked.
   * @param related Action related object.
   * @return Customized object for the tracked item or <code>null</code>
   *         if the item is not to be tracked.
   */
  T CustomizerAdding(ServiceReference item, const ServiceEvent& related);

  /**
   * Call the specific customizer modified method. This method must not be
   * called while synchronized on this object.
   *
   * @param item Tracked item.
   * @param related Action related object.
   * @param object Customized object for the tracked item.
   */
  void CustomizerModified(ServiceReference item,
                          const ServiceEvent& related, T object) ;

  /**
   * Call the specific customizer removed method. This method must not be
   * called while synchronized on this object.
   *
   * @param item Tracked item.
   * @param related Action related object.
   * @param object Customized object for the tracked item.
   */
  void CustomizerRemoved(ServiceReference item,
                         const ServiceEvent& related, T object) ;
};

}

#include "mitkTrackedService.tpp"

#endif // MITKTRACKEDSERVICE_H
