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

namespace mitk {

template<class S, class T>
TrackedService<S,T>::TrackedService(ServiceTracker<S,T>* serviceTracker,
                  ServiceTrackerCustomizer<T>* customizer)
  : serviceTracker(serviceTracker), customizer(customizer)
{

}

template<class S, class T>
void TrackedService<S,T>::ServiceChanged(const ServiceEvent& event)
{
  /*
   * Check if we had a delayed call (which could happen when we
   * close).
   */
  if (this->closed)
  {
    return;
  }

  ServiceReference reference = event.GetServiceReference();
  MITK_DEBUG(serviceTracker->d->DEBUG) << "TrackedService::ServiceChanged["
        << event.GetType() << "]: " << reference;

  switch (event.GetType())
  {
  case ServiceEvent::REGISTERED :
  case ServiceEvent::MODIFIED :
    {
      if (!serviceTracker->d->listenerFilter.empty())
      { // service listener added with filter
        this->Track(reference, event);
        /*
       * If the customizer throws an unchecked exception, it
       * is safe to let it propagate
       */
      }
      else
      { // service listener added without filter
        if (serviceTracker->d->filter.Match(reference))
        {
          this->Track(reference, event);
          /*
         * If the customizer throws an unchecked exception,
         * it is safe to let it propagate
         */
        }
        else
        {
          this->Untrack(reference, event);
          /*
         * If the customizer throws an unchecked exception,
         * it is safe to let it propagate
         */
        }
      }
      break;
    }
  case ServiceEvent::MODIFIED_ENDMATCH :
  case ServiceEvent::UNREGISTERING :
    this->Untrack(reference, event);
    /*
     * If the customizer throws an unchecked exception, it is
     * safe to let it propagate
     */
    break;
  }
}

template<class S, class T>
void TrackedService<S,T>::Modified()
{
  Superclass::Modified(); /* increment the modification count */
  serviceTracker->d->Modified();
}

template<class S, class T>
T TrackedService<S,T>::CustomizerAdding(ServiceReference item,
                                        const ServiceEvent& /*related*/)
{
  return customizer->AddingService(item);
}

template<class S, class T>
void TrackedService<S,T>::CustomizerModified(ServiceReference item,
                                             const ServiceEvent& /*related*/,
                                             T object)
{
  customizer->ModifiedService(item, object);
}

template<class S, class T>
void TrackedService<S,T>::CustomizerRemoved(ServiceReference item,
                                            const ServiceEvent& /*related*/,
                                            T object)
{
  customizer->RemovedService(item, object);
}

} // end namespace mitk
