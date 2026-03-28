/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include <usTrackedService_p.h>

#include <usModuleContext.h>
#include <usLDAPFilter.h>

#include <stdexcept>

namespace us {

template<class S, class TTT>
ServiceTrackerPrivate<S,TTT>::ServiceTrackerPrivate(
    ServiceTracker<S,TTT>* st, ModuleContext* context,
    const ServiceReference<S>& reference,
    ServiceTrackerCustomizer<S,T>* customizer)
  : context(context), customizer(customizer), trackReference(reference),
    trackedService(nullptr), cachedReference(), cachedService(TTT::DefaultValue()), q_ptr(st)
{
  this->customizer = customizer ? customizer : q_func();
  std::stringstream ss;
  ss << "(" << ServiceConstants::SERVICE_ID() << "="
     << any_cast<long>(reference.GetProperty(ServiceConstants::SERVICE_ID())) << ")";
  this->listenerFilter = ss.str();
  try
  {
    this->filter = LDAPFilter(listenerFilter);
  }
  catch (const std::invalid_argument& e)
  {
    /*
     * we could only get this exception if the ServiceReference was
     * invalid
     */
    std::invalid_argument ia(std::string("unexpected std::invalid_argument exception: ") + e.what());
    throw ia;
  }
}

template<class S, class TTT>
ServiceTrackerPrivate<S,TTT>::ServiceTrackerPrivate(
    ServiceTracker<S,TTT>* st,
    ModuleContext* context, const std::string& clazz,
    ServiceTrackerCustomizer<S,T>* customizer)
      : context(context), customizer(customizer), trackClass(clazz),
        trackReference(), trackedService(nullptr), cachedReference(),
        cachedService(TTT::DefaultValue()), q_ptr(st)
{
  this->customizer = customizer ? customizer : q_func();
  this->listenerFilter = std::string("(") + us::ServiceConstants::OBJECTCLASS() + "="
                        + clazz + ")";
  try
  {
    this->filter = LDAPFilter(listenerFilter);
  }
  catch (const std::invalid_argument& e)
  {
    /*
     * we could only get this exception if the clazz argument was
     * malformed
     */
    std::invalid_argument ia(
        std::string("unexpected std::invalid_argument exception: ") + e.what());
    throw ia;
  }
}

template<class S, class TTT>
ServiceTrackerPrivate<S,TTT>::ServiceTrackerPrivate(
    ServiceTracker<S,TTT>* st,
    ModuleContext* context, const LDAPFilter& filter,
    ServiceTrackerCustomizer<S,T>* customizer)
      : context(context), filter(filter), customizer(customizer),
        listenerFilter(filter.ToString()), trackReference(),
        trackedService(nullptr), cachedReference(), cachedService(TTT::DefaultValue()), q_ptr(st)
{
  this->customizer = customizer ? customizer : q_func();
  if (context == nullptr)
  {
    throw std::invalid_argument("The module context cannot be null.");
  }
}

template<class S, class TTT>
ServiceTrackerPrivate<S,TTT>::~ServiceTrackerPrivate()
{

}

template<class S, class TTT>
std::vector<ServiceReference<S> > ServiceTrackerPrivate<S,TTT>::GetInitialReferences(
  const std::string& className, const std::string& filterString)
{
  std::vector<ServiceReference<S> > result;
  std::vector<ServiceReferenceU> refs = context->GetServiceReferences(className, filterString);
  for(std::vector<ServiceReferenceU>::const_iterator iter = refs.begin();
      iter != refs.end(); ++iter)
  {
    ServiceReference<S> ref(*iter);
    if (ref)
    {
      result.push_back(ref);
    }
  }
  return result;
}

template<class S, class TTT>
void ServiceTrackerPrivate<S,TTT>::GetServiceReferences_unlocked(std::vector<ServiceReference<S> >& refs, TrackedService<S,TTT>* t) const
{
  if (t->Size() == 0)
  {
    return;
  }
  t->GetTracked(refs);
}

template<class S, class TTT>
TrackedService<S,TTT>* ServiceTrackerPrivate<S,TTT>::Tracked() const
{
  return trackedService;
}

template<class S, class TTT>
void ServiceTrackerPrivate<S,TTT>::Modified()
{
  cachedReference = 0; /* clear cached value */
  TTT::Dispose(cachedService); /* clear cached value */
}

}
