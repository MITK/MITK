/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

namespace us {

template<class S, class TTT>
TrackedService<S,TTT>::TrackedService(ServiceTracker<S,TTT>* serviceTracker,
                  ServiceTrackerCustomizer<S,T>* customizer)
  : serviceTracker(serviceTracker), customizer(customizer)
{

}

template<class S, class TTT>
void TrackedService<S,TTT>::ServiceChanged(const ServiceEvent event)
{
  /*
   * Check if we had a delayed call (which could happen when we
   * close).
   */
  if (this->closed)
  {
    return;
  }

  ServiceReference<S> reference = event.GetServiceReference(InterfaceType<S>());
  if (!reference)
  {
    return;
  }

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

template<class S, class TTT>
void TrackedService<S,TTT>::Modified()
{
  Superclass::Modified(); /* increment the modification count */
  serviceTracker->d->Modified();
}

template<class S, class TTT>
typename TrackedService<S,TTT>::T
TrackedService<S,TTT>::CustomizerAdding(ServiceReference<S> item,
                                        const ServiceEvent& /*related*/)
{
  return customizer->AddingService(item);
}

template<class S, class TTT>
void TrackedService<S,TTT>::CustomizerModified(ServiceReference<S> item,
                                               const ServiceEvent& /*related*/,
                                               T object)
{
  customizer->ModifiedService(item, object);
}

template<class S, class TTT>
void TrackedService<S,TTT>::CustomizerRemoved(ServiceReference<S> item,
                                              const ServiceEvent& /*related*/,
                                              T object)
{
  customizer->RemovedService(item, object);
}

}
