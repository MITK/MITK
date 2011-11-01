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


#include "mitkServiceTrackerPrivate.h"
#include "mitkTrackedService.h"
#include "mitkServiceException.h"
#include "mitkModuleContext.h"

#include <mitkLogMacros.h>

#include <stdexcept>
#include <limits>

namespace mitk {

template<class S, class T>
ServiceTracker<S,T>::~ServiceTracker()
{
  delete d;
}

template<class S, class T>
ServiceTracker<S,T>::ServiceTracker(ModuleContext* context,
                                    const ServiceReference& reference,
                                    _ServiceTrackerCustomizer* customizer)
  : d(new _ServiceTrackerPrivate(this, context, reference, customizer))
{
}

template<class S, class T>
ServiceTracker<S,T>::ServiceTracker(ModuleContext* context, const std::string& clazz,
                                    _ServiceTrackerCustomizer* customizer)
  : d(new _ServiceTrackerPrivate(this, context, clazz, customizer))
{
}

template<class S, class T>
ServiceTracker<S,T>::ServiceTracker(ModuleContext* context, const LDAPFilter& filter,
                                    _ServiceTrackerCustomizer* customizer)
  : d(new _ServiceTrackerPrivate(this, context, filter, customizer))
{
}

template<class S, class T>
ServiceTracker<S,T>::ServiceTracker(ModuleContext *context, ServiceTrackerCustomizer<T> *customizer)
  : d(new _ServiceTrackerPrivate(this, context, mitk_service_interface_iid<S>(), customizer))
{
  const char* clazz = mitk_service_interface_iid<S>();
  if (clazz == 0) throw ServiceException("The service interface class has no MITK_DECLARE_SERVICE_INTERFACE macro");
}

template<class S, class T>
void ServiceTracker<S,T>::Open()
{
  itk::SmartPointer<_TrackedService> t;
  {
    MutexLocker lock(d->mutex);
    if (d->trackedService)
    {
      return;
    }

    MITK_DEBUG(d->DEBUG) << "ServiceTracker<S,T>::Open: " << d->filter;

    t = itk::SmartPointer<_TrackedService>(
          new _TrackedService(this, d->customizer));
    {
      itk::MutexLockHolder<itk::SimpleMutexLock> lockT(*t);
      try {
        d->context->AddServiceListener(t.GetPointer(), &_TrackedService::ServiceChanged, d->listenerFilter);
        std::list<ServiceReference> references;
        if (!d->trackClass.empty())
        {
          references = d->GetInitialReferences(d->trackClass, std::string());
        }
        else
        {
          if (d->trackReference.GetModule() != 0)
          {
            references.push_back(d->trackReference);
          }
          else
          { /* user supplied filter */
            references = d->GetInitialReferences(std::string(),
                                                 (d->listenerFilter.empty()) ? d->filter.ToString() : d->listenerFilter);
          }
        }
        /* set tracked with the initial references */
        t->SetInitial(references);
      }
      catch (const std::invalid_argument& e)
      {
        throw std::runtime_error(std::string("unexpected std::invalid_argument exception: ")
            + e.what());
      }
    }
    d->trackedService = t;
  }
  /* Call tracked outside of synchronized region */
  t->TrackInitial(); /* process the initial references */
}

template<class S, class T>
void ServiceTracker<S,T>::Close()
{
  itk::SmartPointer<_TrackedService> outgoing;
  std::list<ServiceReference> references;
  {
    MutexLocker lock(d->mutex);
    outgoing = d->trackedService;
    if (outgoing.IsNull())
    {
      return;
    }
    MITK_DEBUG(d->DEBUG) << "ServiceTracker<S,T>::close:" << d->filter;
    outgoing->Close();
    GetServiceReferences(references);
    d->trackedService = 0;
    try
    {
      d->context->RemoveServiceListener(outgoing.GetPointer(), &_TrackedService::ServiceChanged);
    }
    catch (const std::logic_error& /*e*/)
    {
      /* In case the context was stopped. */
    }
  }
  d->Modified(); /* clear the cache */
  {
    itk::MutexLockHolder<itk::SimpleMutexLock> lockT(*outgoing);
    outgoing->WakeAll(); /* wake up any waiters */
  }
  for(std::list<ServiceReference>::const_iterator ref = references.begin();
      ref != references.end(); ++ref)
  {
    outgoing->Untrack(*ref, ServiceEvent());
  }

  if (d->DEBUG)
  {
    MutexLocker lock(d->mutex);
    if ((d->cachedReference.GetModule() == 0) && (d->cachedService == 0))
    {
      MITK_DEBUG(true) << "ServiceTracker<S,T>::close[cached cleared]:"
                       << d->filter;
    }
  }
}

template<class S, class T>
T ServiceTracker<S,T>::WaitForService()
{
  T object = GetService();
  while (object == 0)
  {
    itk::SmartPointer<_TrackedService> t = d->Tracked();
    if (t.IsNull())
    { /* if ServiceTracker is not open */
      return 0;
    }
    {
      itk::MutexLockHolder<itk::SimpleMutexLock> lockT(*t);
      if (t->Size() == 0)
      {
        t->Wait();
      }
    }
    object = GetService();
  }
  return object;
}

template<class S, class T>
void ServiceTracker<S,T>::GetServiceReferences(std::list<ServiceReference>& refs) const
{
  itk::SmartPointer<_TrackedService> t = d->Tracked();
  if (t.IsNull())
  { /* if ServiceTracker is not open */
    return;
  }
  {
    itk::MutexLockHolder<itk::SimpleMutexLock> lockT(*t);
    d->GetServiceReferences_unlocked(refs, t.GetPointer());
  }
}

template<class S, class T>
ServiceReference ServiceTracker<S,T>::GetServiceReference() const
{
  ServiceReference reference(0);
  {
    MutexLocker lock(d->mutex);
    reference = d->cachedReference;
  }
  if (reference.GetModule() != 0)
  {
    MITK_DEBUG(d->DEBUG) << "ServiceTracker<S,T>::getServiceReference[cached]:"
                         << d->filter;
    return reference;
  }
  MITK_DEBUG(d->DEBUG) << "ServiceTracker<S,T>::getServiceReference:" << d->filter;
  std::list<ServiceReference> references;
  GetServiceReferences(references);
  int length = references.size();
  if (length == 0)
  { /* if no service is being tracked */
    throw ServiceException("No service is being tracked");
  }
  std::list<ServiceReference>::const_iterator selectedRef;
  if (length > 1)
  { /* if more than one service, select highest ranking */
    std::vector<int> rankings(length);
    int count = 0;
    int maxRanking = std::numeric_limits<int>::min();
    std::list<ServiceReference>::const_iterator refIter = references.begin();
    for (int i = 0; i < length; i++)
    {
      bool ok = false;
      Any rankingAny = refIter->GetProperty(ServiceConstants::SERVICE_RANKING());
      int ranking = 0;
      if (rankingAny.Type() == typeid(int))
      {
        ranking = any_cast<int>(rankingAny);
        ok = true;
      }

      rankings[i] = ranking;
      if (ranking > maxRanking)
      {
        selectedRef = refIter;
        maxRanking = ranking;
        count = 1;
      }
      else
      {
        if (ranking == maxRanking)
        {
          count++;
        }
      }
    }
    if (count > 1)
    { /* if still more than one service, select lowest id */
      long int minId = std::numeric_limits<long int>::max();
      for (int i = 0; i < length; i++)
      {
        if (rankings[i] == maxRanking)
        {
          Any idAny = refIter->GetProperty(ServiceConstants::SERVICE_ID());
          long int id = 0;
          if (idAny.Type() == typeid(long int))
          {
            id = any_cast<long int>(idAny);
          }
          if (id < minId)
          {
            selectedRef = refIter;
            minId = id;
          }
        }
      }
    }
    ++refIter;
  }

  {
    MutexLocker lock(d->mutex);
    d->cachedReference = *selectedRef;
    return d->cachedReference;
  }
}

template<class S, class T>
T ServiceTracker<S,T>::GetService(const ServiceReference& reference) const
{
  itk::SmartPointer<_TrackedService> t = d->Tracked();
  if (t.IsNull())
  { /* if ServiceTracker is not open */
    return 0;
  }
  {
    itk::MutexLockHolder<itk::SimpleMutexLock> lockT(*t);
    return t->GetCustomizedObject(reference);
  }
}

template<class S, class T>
void ServiceTracker<S,T>::GetServices(std::list<T>& services) const
{
  itk::SmartPointer<_TrackedService> t = d->Tracked();
  if (t.IsNull())
  { /* if ServiceTracker is not open */
    return;
  }
  {
    itk::MutexLockHolder<itk::SimpleMutexLock> lockT(*t);
    std::list<ServiceReference> references;
    d->GetServiceReferences_unlocked(references, t.GetPointer());
    for(std::list<ServiceReference>::const_iterator ref = references.begin();
        ref != references.end(); ++ref)
    {
      services.push_back(GetService(*ref));
    }
  }
}

template<class S, class T>
T ServiceTracker<S,T>::GetService() const
{
  T service = d->cachedService;
  if (service != 0)
  {
    MITK_DEBUG(d->DEBUG) << "ServiceTracker<S,T>::getService[cached]:"
                         << d->filter;
    return service;
  }
  MITK_DEBUG(d->DEBUG) << "ServiceTracker<S,T>::getService:" << d->filter;

  try
  {
    ServiceReference reference = GetServiceReference();
    if (reference.GetModule() == 0)
    {
      return 0;
    }
    return d->cachedService = GetService(reference);
  }
  catch (const ServiceException&)
  {
    return 0;
  }
}

template<class S, class T>
void ServiceTracker<S,T>::Remove(const ServiceReference& reference)
{
  itk::SmartPointer<_TrackedService> t = d->Tracked();
  if (t.IsNull())
  { /* if ServiceTracker is not open */
    return;
  }
  t->Untrack(reference, ServiceEvent());
}

template<class S, class T>
int ServiceTracker<S,T>::Size() const
{
  itk::SmartPointer<_TrackedService> t = d->Tracked();
  if (t.IsNull())
  { /* if ServiceTracker is not open */
    return 0;
  }
  {
    itk::MutexLockHolder<itk::SimpleMutexLock> lockT(*t);
    return t->Size();
  }
}

template<class S, class T>
int ServiceTracker<S,T>::GetTrackingCount() const
{
  itk::SmartPointer<_TrackedService> t = d->Tracked();
  if (t.IsNull())
  { /* if ServiceTracker is not open */
    return -1;
  }
  {
    itk::MutexLockHolder<itk::SimpleMutexLock> lockT(*t);
    return t->GetTrackingCount();
  }
}

template<class S, class T>
void ServiceTracker<S,T>::GetTracked(TrackingMap& map) const
{
  itk::SmartPointer<_TrackedService> t = d->Tracked();
  if (t.IsNull())
  { /* if ServiceTracker is not open */
    return;
  }
  {
    itk::MutexLockHolder<itk::SimpleMutexLock> lockT(*t);
    t->CopyEntries(map);
  }
}

template<class S, class T>
bool ServiceTracker<S,T>::IsEmpty() const
{
  itk::SmartPointer<_TrackedService> t = d->Tracked();
  if (t.IsNull())
  { /* if ServiceTracker is not open */
    return true;
  }
  {
    itk::MutexLockHolder<itk::SimpleMutexLock> lockT(*t);
    return t->IsEmpty();
  }
}

template<class S, class T>
T ServiceTracker<S,T>::AddingService(const ServiceReference& reference)
{
 return dynamic_cast<T>(d->context->GetService(reference));
}

template<class S, class T>
void ServiceTracker<S,T>::ModifiedService(const ServiceReference& /*reference*/, T /*service*/)
{
  /* do nothing */
}

template<class S, class T>
void ServiceTracker<S,T>::RemovedService(const ServiceReference& reference, T /*service*/)
{
  d->context->UngetService(reference);
}

} // end namespace mitk
