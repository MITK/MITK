/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include <usServiceTrackerPrivate.h>
#include <usTrackedService_p.h>
#include <usServiceException.h>
#include <usModuleContext.h>

#include <string>
#include <stdexcept>
#include <limits>

namespace us {

template<class S, class TTT>
ServiceTracker<S,TTT>::~ServiceTracker()
{
  Close();
  delete d;
}

template<class S, class TTT>
ServiceTracker<S,TTT>::ServiceTracker(ModuleContext* context,
                                      const ServiceReferenceType& reference,
                                      _ServiceTrackerCustomizer* customizer)
  : d(new _ServiceTrackerPrivate(this, context, reference, customizer))
{
}

template<class S, class TTT>
ServiceTracker<S,TTT>::ServiceTracker(ModuleContext* context, const std::string& clazz,
                                      _ServiceTrackerCustomizer* customizer)
  : d(new _ServiceTrackerPrivate(this, context, clazz, customizer))
{
}

template<class S, class TTT>
ServiceTracker<S,TTT>::ServiceTracker(ModuleContext* context, const LDAPFilter& filter,
                                      _ServiceTrackerCustomizer* customizer)
  : d(new _ServiceTrackerPrivate(this, context, filter, customizer))
{
}

template<class S, class TTT>
ServiceTracker<S,TTT>::ServiceTracker(ModuleContext *context, _ServiceTrackerCustomizer* customizer)
  : d(new _ServiceTrackerPrivate(this, context, us_service_interface_iid<S>(), customizer))
{
  std::string clazz = us_service_interface_iid<S>();
  if (clazz.empty()) throw ServiceException("The service interface class has no US_DECLARE_SERVICE_INTERFACE macro");
}

template<class S, class TTT>
void ServiceTracker<S,TTT>::Open()
{
  _TrackedService* t;
  {
    (void)(typename _ServiceTrackerPrivate::Lock(d));
    if (d->trackedService)
    {
      return;
    }


    t = new _TrackedService(this, d->customizer);
    {
      (void)(typename _TrackedService::Lock(*t));
      try {
        d->context->AddServiceListener(t, &_TrackedService::ServiceChanged, d->listenerFilter);
        std::vector<ServiceReferenceType> references;
        if (!d->trackClass.empty())
        {
          references = d->GetInitialReferences(d->trackClass, std::string());
        }
        else
        {
          if (d->trackReference.GetModule() != nullptr)
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

template<class S, class TTT>
void ServiceTracker<S,TTT>::Close()
{
  _TrackedService* outgoing;
  std::vector<ServiceReferenceType> references;
  {
    (void)(typename _ServiceTrackerPrivate::Lock(d));
    outgoing = d->trackedService;
    if (outgoing == nullptr)
    {
      return;
    }
    outgoing->Close();
    references = GetServiceReferences();
    d->trackedService = nullptr;
    try
    {
      d->context->RemoveServiceListener(outgoing, &_TrackedService::ServiceChanged);
    }
    catch (const std::logic_error& /*e*/)
    {
      /* In case the context was stopped. */
    }
  }
  d->Modified(); /* clear the cache */
  {
    (void)(typename _TrackedService::Lock(outgoing));
    outgoing->NotifyAll(); /* wake up any waiters */
  }
  for(typename std::vector<ServiceReferenceType>::const_iterator ref = references.begin();
      ref != references.end(); ++ref)
  {
    outgoing->Untrack(*ref, ServiceEvent());
  }

  delete outgoing;
  d->trackedService = nullptr;
}

template<class S, class TTT>
typename ServiceTracker<S,TTT>::T
ServiceTracker<S,TTT>::WaitForService(unsigned long timeoutMillis)
{
  T object = GetService();
  while (!TTT::IsValid(object))
  {
    _TrackedService* t = d->Tracked();
    if (t == nullptr)
    { /* if ServiceTracker is not open */
      return TTT::DefaultValue();
    }
    {
      (void)(typename _TrackedService::Lock(t));
      if (t->Size() == 0)
      {
        t->Wait(timeoutMillis);
      }
    }
    object = GetService();
  }
  return object;
}

template<class S, class TTT>
std::vector<typename ServiceTracker<S,TTT>::ServiceReferenceType>
ServiceTracker<S,TTT>::GetServiceReferences() const
{
  std::vector<ServiceReferenceType> refs;
  _TrackedService* t = d->Tracked();
  if (t == nullptr)
  { /* if ServiceTracker is not open */
    return refs;
  }
  {
    (void)(typename _TrackedService::Lock(t));
    d->GetServiceReferences_unlocked(refs, t);
  }
  return refs;
}

template<class S, class TTT>
typename ServiceTracker<S,TTT>::ServiceReferenceType
ServiceTracker<S,TTT>::GetServiceReference() const
{
  ServiceReferenceType reference;
  {
    (void)(typename _ServiceTrackerPrivate::Lock(d));
    reference = d->cachedReference;
  }
  if (reference.GetModule() != nullptr)
  {
    return reference;
  }
  std::vector<ServiceReferenceType> references = GetServiceReferences();
  std::size_t length = references.size();
  if (length == 0)
  { /* if no service is being tracked */
    throw ServiceException("No service is being tracked");
  }
  typename std::vector<ServiceReferenceType>::const_iterator selectedRef = references.begin();
  if (length > 1)
  { /* if more than one service, select highest ranking */
    std::vector<int> rankings(length);
    int count = 0;
    int maxRanking = std::numeric_limits<int>::min();
    typename std::vector<ServiceReferenceType>::const_iterator refIter = references.begin();
    for (std::size_t i = 0; i < length; i++)
    {
      Any rankingAny = refIter->GetProperty(ServiceConstants::SERVICE_RANKING());
      int ranking = 0;
      if (rankingAny.Type() == typeid(int))
      {
        ranking = any_cast<int>(rankingAny);
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
      ++refIter;
    }
    if (count > 1)
    { /* if still more than one service, select lowest id */
      long int minId = std::numeric_limits<long int>::max();
      refIter = references.begin();
      for (std::size_t i = 0; i < length; i++)
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
        ++refIter;
      }
    }
  }

  {
    (void)(typename _ServiceTrackerPrivate::Lock(d));
    d->cachedReference = *selectedRef;
    return d->cachedReference;
  }
}

template<class S, class TTT>
typename ServiceTracker<S,TTT>::T
ServiceTracker<S,TTT>::GetService(const ServiceReferenceType& reference) const
{
  _TrackedService* t = d->Tracked();
  if (t == nullptr)
  { /* if ServiceTracker is not open */
    return TTT::DefaultValue();
  }
  {
    (void)(typename _TrackedService::Lock(t));
    return t->GetCustomizedObject(reference);
  }
}

template<class S, class TTT>
std::vector<typename ServiceTracker<S,TTT>::T> ServiceTracker<S,TTT>::GetServices() const
{
  std::vector<T> services;
  _TrackedService* t = d->Tracked();
  if (t == nullptr)
  { /* if ServiceTracker is not open */
    return services;
  }
  {
    (void)(typename _TrackedService::Lock(t));
    std::vector<ServiceReferenceType> references;
    d->GetServiceReferences_unlocked(references, t);
    for(typename std::vector<ServiceReferenceType>::const_iterator ref = references.begin();
        ref != references.end(); ++ref)
    {
      services.push_back(t->GetCustomizedObject(*ref));
    }
  }
  return services;
}

template<class S, class TTT>
typename ServiceTracker<S,TTT>::T
ServiceTracker<S,TTT>::GetService() const
{
  {
    (void)(typename _ServiceTrackerPrivate::Lock(d));
    const T& service = d->cachedService;
    if (TTT::IsValid(service))
    {
      return service;
    }
  }

  try
  {
    ServiceReferenceType reference = GetServiceReference();
    if (reference.GetModule() == nullptr)
    {
      return TTT::DefaultValue();
    }
    {
      (void)(typename _ServiceTrackerPrivate::Lock(d));
      return d->cachedService = GetService(reference);
    }
  }
  catch (const ServiceException&)
  {
    return TTT::DefaultValue();
  }
}

template<class S, class TTT>
void ServiceTracker<S,TTT>::Remove(const ServiceReferenceType& reference)
{
  _TrackedService* t = d->Tracked();
  if (t == nullptr)
  { /* if ServiceTracker is not open */
    return;
  }
  t->Untrack(reference, ServiceEvent());
}

template<class S, class TTT>
int ServiceTracker<S,TTT>::Size() const
{
  _TrackedService* t = d->Tracked();
  if (t == nullptr)
  { /* if ServiceTracker is not open */
    return 0;
  }
  {
    (void)(typename _TrackedService::Lock(t));
    return static_cast<int>(t->Size());
  }
}

template<class S, class TTT>
int ServiceTracker<S,TTT>::GetTrackingCount() const
{
  _TrackedService* t = d->Tracked();
  if (t == nullptr)
  { /* if ServiceTracker is not open */
    return -1;
  }
  {
    (void)(typename _TrackedService::Lock(t));
    return t->GetTrackingCount();
  }
}

template<class S, class TTT>
void ServiceTracker<S,TTT>::GetTracked(TrackingMap& map) const
{
  _TrackedService* t = d->Tracked();
  if (t == nullptr)
  { /* if ServiceTracker is not open */
    return;
  }
  {
    (void)(typename _TrackedService::Lock(t));
    t->CopyEntries(map);
  }
}

template<class S, class TTT>
bool ServiceTracker<S,TTT>::IsEmpty() const
{
  _TrackedService* t = d->Tracked();
  if (t == nullptr)
  { /* if ServiceTracker is not open */
    return true;
  }
  {
    (void)(typename _TrackedService::Lock(t));
    return t->IsEmpty();
  }
}

template<class S, class TTT>
typename ServiceTracker<S,TTT>::T
ServiceTracker<S,TTT>::AddingService(const ServiceReferenceType& reference)
{
  return TTT::ConvertToTrackedType(d->context->GetService(reference));
}

template<class S, class TTT>
void ServiceTracker<S,TTT>::ModifiedService(const ServiceReferenceType& /*reference*/, T /*service*/)
{
  /* do nothing */
}

template<class S, class TTT>
void ServiceTracker<S,TTT>::RemovedService(const ServiceReferenceType& reference, T /*service*/)
{
  d->context->UngetService(reference);
}

}
