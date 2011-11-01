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


#include "mitkTrackedService.h"

#include "mitkModuleContext.h"
#include "mitkLDAPFilter.h"

#include <stdexcept>

namespace mitk {

template<class S, class T>
const bool ServiceTrackerPrivate<S,T>::DEBUG = true;

template<class S, class T>
ServiceTrackerPrivate<S,T>::ServiceTrackerPrivate(
    ServiceTracker<S,T>* st, ModuleContext* context,
    const ServiceReference& reference,
    ServiceTrackerCustomizer<T>* customizer)
  : context(context), customizer(customizer), trackReference(reference),
    trackedService(0), cachedReference(0), cachedService(0), q_ptr(st)
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

template<class S, class T>
ServiceTrackerPrivate<S,T>::ServiceTrackerPrivate(
    ServiceTracker<S,T>* st,
    ModuleContext* context, const std::string& clazz,
    ServiceTrackerCustomizer<T>* customizer)
      : context(context), customizer(customizer), trackClass(clazz),
        trackReference(0), trackedService(0), cachedReference(0),
        cachedService(0), q_ptr(st)
{
  this->customizer = customizer ? customizer : q_func();
  this->listenerFilter = std::string("(") + mitk::ServiceConstants::OBJECTCLASS() + "="
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

template<class S, class T>
ServiceTrackerPrivate<S,T>::ServiceTrackerPrivate(
    ServiceTracker<S,T>* st,
    ModuleContext* context, const LDAPFilter& filter,
    ServiceTrackerCustomizer<T>* customizer)
      : context(context), filter(filter), customizer(customizer),
        listenerFilter(filter.ToString()), trackReference(0),
        trackedService(0), cachedReference(0), cachedService(0), q_ptr(st)
{
  this->customizer = customizer ? customizer : q_func();
  if (context == 0)
  {
    throw std::invalid_argument("The module context cannot be null.");
  }
}

template<class S, class T>
ServiceTrackerPrivate<S,T>::~ServiceTrackerPrivate()
{

}

template<class S, class T>
std::list<ServiceReference> ServiceTrackerPrivate<S,T>::GetInitialReferences(
  const std::string& className, const std::string& filterString)
{
  return context->GetServiceReferences(className, filterString);
}

template<class S, class T>
void ServiceTrackerPrivate<S,T>::GetServiceReferences_unlocked(std::list<ServiceReference>& refs,
                                                               TrackedService<S,T>* t) const
{
  if (t->Size() == 0)
  {
    return;
  }
  t->GetTracked(refs);
}

template<class S, class T>
itk::SmartPointer<TrackedService<S,T> > ServiceTrackerPrivate<S,T>::Tracked() const
{
  return trackedService;
}

template<class S, class T>
void ServiceTrackerPrivate<S,T>::Modified()
{
  cachedReference = 0; /* clear cached value */
  cachedService = 0; /* clear cached value */
  MITK_DEBUG(DEBUG) << "ServiceTracker::Modified(): " << filter;
}

} // end namespace mitk
