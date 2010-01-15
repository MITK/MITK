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

#include "cherryServiceRegistry.h"

#include "cherryServiceReferenceImpl.h"
#include "cherryServiceRegistrationImpl.h"
#include "cherryServiceUse.h"
#include "cherryFilteredServiceListener.h"

#include "../osgi.framework/cherryFramework.h"
#include "../osgi.framework/cherryBundleContextImpl.h"
#include "../osgi.framework/cherryBundleHost.h"

#include <osgi/framework/ServiceFactory.h>
#include <supplement/framework.debug/cherryDebug.h>

#include <Poco/Any.h>

namespace cherry {
namespace osgi {
namespace internal {

const int ServiceRegistry::initialCapacity = 50;
const int ServiceRegistry::initialSubCapacity = 10;
const int ServiceRegistry::SERVICEEVENT = 3;


   ServiceRegistry::ServiceRegistry(SmartPointer<Framework> framework)
   : publishedServicesByClass(initialCapacity),
   publishedServicesByContext(initialCapacity),
   serviceEventListeners(initialCapacity),
   serviceid(1), framework(framework)
   {

  }

  SmartPointer<ServiceRegistrationImpl> ServiceRegistry::RegisterService(
      SmartPointer<BundleContextImpl> context,
      const std::vector<std::string>& cl, Object::Pointer service,
      const SmartPointer<Dictionary> properties) throw(IllegalArgumentException, IllegalStateException) {
    if (!service) {
      if (Debug::DEBUG && Debug::DEBUG_SERVICES) {
        std::cout << "Service object is null" << std::endl;
      }

      throw IllegalArgumentException("The service parameter is null");
    }

    if (cl.empty()) {
      if (Debug::DEBUG && Debug::DEBUG_SERVICES) {
        std::cout << "Classes array is empty" << std::endl;
      }

      throw IllegalArgumentException("The array of service names is empty");
    }

    /* copy the array so that changes to the original will not affect us. */
    std::vector<std::string> clazzes(cl);
    std::sort(clazzes.begin(), clazzes.end());
    std::vector<std::string>::iterator newend = std::unique(clazzes.begin(), clazzes.end());
    clazzes.erase(newend, clazzes.end());

    if (!(service.Cast<ServiceFactory>())) {
      std::string invalidService = CheckServiceClass(clazzes, service);
      if (!invalidService.empty()) {
        if (Debug::DEBUG && Debug::DEBUG_SERVICES) {
          std::cout << "Service object is not an instanceof " + invalidService << std::endl;
        }
        throw IllegalArgumentException("The service object is not an instance of the service class " + invalidService);
      }
    }

    ServiceRegistrationImpl::Pointer registration(new ServiceRegistrationImpl(
        ServiceRegistry::Pointer(this), context, clazzes, service));
    registration->RegisterService(*properties);
    //if (copy.contains(listenerHookName)) {
    //  notifyNewListenerHook(registration);
    //}
    return registration;
  }

  std::vector<SmartPointer<ServiceReferenceImpl> > ServiceRegistry::GetServiceReferences(
      SmartPointer<BundleContextImpl> context, const std::string& clazz,
      const std::string& filterstring, bool allservices) throw(InvalidSyntaxException) {
    if (Debug::DEBUG && Debug::DEBUG_SERVICES) {
      std::cout << (allservices ? "GetAllServiceReferences(" : "GetServiceReferences(") << clazz << ", \"" + filterstring + "\")" << std::endl;
    }
    Filter::Pointer filter = filterstring.empty() ? Filter::Pointer(0) : context->CreateFilter(filterstring);
    std::vector<ServiceReferenceImpl::Pointer> references;
    ChangeRegistrationsToReferences(LookupServiceRegistrations(clazz, filter), references);
//    for (std::vector<ServiceReferenceImpl::Pointer>::iterator iter = references.begin(); iter != references.end(); ++iter)
//    {
//      ServiceReferenceImpl::Pointer reference = *iter;
//      if (allservices || isAssignableTo(context, reference)) {
//        try { /* test for permission to get the service */
//          checkGetServicePermission(reference);
//        } catch (SecurityException se) {
//          iter.remove();
//        }
//      } else {
//        iter.remove();
//      }
//    }

//    Collection shrinkable = new ShrinkableCollection(references);
//    notifyFindHooks(context, clazz, filterstring, allservices, shrinkable);

    return references;
  }

  SmartPointer<ServiceReferenceImpl> ServiceRegistry::GetServiceReference(
      SmartPointer<BundleContextImpl> context, const std::string& clazz) {
    if (Debug::DEBUG && Debug::DEBUG_SERVICES) {
      std::cout << "GetServiceReference(" << clazz << ")" << std::endl;
    }

    try {
      std::vector<ServiceReferenceImpl::Pointer> references = GetServiceReferences(context, clazz, "", false);

      if (!references.empty()) {
        // Since we maintain the registrations in a sorted List, the first element is always the
        // correct one to return.
        return references[0];
      }
    } catch (InvalidSyntaxException e) {
      if (Debug::DEBUG && Debug::DEBUG_GENERAL) {
        std::cout << "InvalidSyntaxException w/ null filter" << e.displayText() << std::endl;
        Debug::PrintStackTrace();
      }
    }

    return ServiceReferenceImpl::Pointer(0);
  }

  Object::Pointer ServiceRegistry::GetService(SmartPointer<BundleContextImpl> context,
      SmartPointer<ServiceReferenceImpl> reference) {
    /* test for permission to get the service */
    //checkGetServicePermission(reference);

    return reference->GetRegistration()->GetService(context);
  }

  bool ServiceRegistry::UngetService(SmartPointer<BundleContextImpl> context,
      SmartPointer<ServiceReferenceImpl> reference) {
    ServiceRegistrationImpl::Pointer registration = reference->GetRegistration();

    return registration->UngetService(context);
  }

  std::vector<SmartPointer<ServiceReferenceImpl> > ServiceRegistry::GetRegisteredServices(
      SmartPointer<BundleContextImpl> context) {
    std::vector<ServiceReferenceImpl::Pointer> references;
    ChangeRegistrationsToReferences(LookupServiceRegistrations(context), references);
//    for (Iterator iter = references.iterator(); iter.hasNext();) {
//      ServiceReferenceImpl reference = (ServiceReferenceImpl) iter.next();
//      try { /* test for permission to get the service */
//        checkGetServicePermission(reference);
//      } catch (SecurityException se) {
//        iter.remove();
//      }
//    }

    return references;
  }

  std::vector<SmartPointer<ServiceReferenceImpl> > ServiceRegistry::GetServicesInUse(
      SmartPointer<BundleContextImpl> context) {
    const BundleContextImpl::ServiceUseMap& servicesInUse = context->GetServicesInUseMap();

    std::vector<ServiceReferenceImpl::Pointer> references;
    {
    Poco::Mutex::ScopedLock lock(servicesInUseLock);
      if (servicesInUse.empty()) {
        return std::vector<ServiceReferenceImpl::Pointer>();
      }
      for (BundleContextImpl::ServiceUseMap::ConstIterator iter = servicesInUse.begin();
          iter != servicesInUse.end(); ++iter)
      {
        references.push_back(iter->first->GetReferenceImpl());
      }
    }

//    for (Iterator iter = references.iterator(); iter.hasNext();) {
//      ServiceReferenceImpl reference = (ServiceReferenceImpl) iter.next();
//      try { /* test for permission to get the service */
//        checkGetServicePermission(reference);
//      } catch (SecurityException se) {
//        iter.remove();
//      }
//    }

    return references;
  }

  void ServiceRegistry::UnregisterServices(SmartPointer<BundleContextImpl> context) {
    std::vector<ServiceRegistrationImpl::Pointer> registrations(LookupServiceRegistrations(context));
    for (std::vector<ServiceRegistrationImpl::Pointer>::iterator iter = registrations.begin();
        iter != registrations.end(); ++iter)
    {
      try {
        (*iter)->UnregisterService();
      } catch (const IllegalStateException& e) {
        /* already unregistered */
      }
    }
  }

  void ServiceRegistry::ReleaseServicesInUse(SmartPointer<BundleContextImpl> context) {
    const BundleContextImpl::ServiceUseMap& servicesInUse = context->GetServicesInUseMap();

    std::vector<ServiceRegistrationImpl::Pointer> registrations;
    {
    Poco::Mutex::ScopedLock lock(servicesInUseLock);
      if (servicesInUse.empty()) {
        return;
      }
      for (BundleContextImpl::ServiceUseMap::ConstIterator iter = servicesInUse.begin();
          iter != servicesInUse.end(); ++iter)
      {
        registrations.push_back(iter->first);
      }
    }
    if (Debug::DEBUG && Debug::DEBUG_SERVICES) {
      std::cout << "Releasing services" << std::endl;
    }
    for (std::vector<ServiceRegistrationImpl::Pointer>::iterator iter = registrations.begin();
        iter != registrations.end(); ++iter)
    {
      (*iter)->ReleaseService(context);
    }
  }

  void ServiceRegistry::AddServiceListener(SmartPointer<BundleContextImpl> context,
      SmartPointer<ServiceListener> listener, const std::string& filter)
  throw(InvalidSyntaxException) {
    if (Debug::DEBUG && Debug::DEBUG_EVENTS) {
      std::cout << "AddServiceListener[" << context->GetBundleImpl() << "]("
        << listener->GetClassName() << "@" << std::hex << listener->Object::HashCode()
        << ", \"" << filter << "\")" << std::endl;
    }

    FilteredServiceListener::Pointer filteredListener(new FilteredServiceListener(context, listener, filter));
    FilteredServiceListener::Pointer oldFilteredListener;
    {
      Poco::Mutex::ScopedLock lock(serviceEventListenersLock);
      ServiceListenerMap& listeners = serviceEventListeners[context];
      oldFilteredListener = listeners[listener];
      listeners[listener] = filteredListener;
    }

    if (oldFilteredListener) {
      oldFilteredListener->MarkRemoved();
      //Collection removedListeners = Collections.singletonList(oldFilteredListener);
      //notifyListenerHooks(removedListeners, false);
    }

    //Collection addedListeners = Collections.singletonList(filteredListener);
    //notifyListenerHooks(addedListeners, true);
  }

  void ServiceRegistry::RemoveServiceListener(SmartPointer<BundleContextImpl> context,
      SmartPointer<ServiceListener> listener) {
    if (Debug::DEBUG && Debug::DEBUG_EVENTS) {
      std::cout << "RemoveServiceListener[" << context->GetBundleImpl() << "]("
      << listener->GetClassName() << "@" << std::hex << listener->Object::HashCode() << ")" << std::endl;
    }

    FilteredServiceListener::Pointer oldFilteredListener;
    {
    Poco::Mutex::ScopedLock lock(serviceEventListenersLock);
      ServiceListenerMap& listeners = serviceEventListeners[context];
      if (listeners.empty()) {
        return; // this context has no listeners to begin with
      }
      oldFilteredListener = listeners[listener];
      listeners.erase(listener);
    }

    if (!oldFilteredListener) {
      return;
    }
    oldFilteredListener->MarkRemoved();
    //Collection removedListeners = Collections.singletonList(oldFilteredListener);
    //notifyListenerHooks(removedListeners, false);
  }

  void ServiceRegistry::RemoveAllServiceListeners(SmartPointer<BundleContextImpl> context) {
    Poco::Any removedListenersMapWrapper;
    {
    Poco::Mutex::ScopedLock lock(serviceEventListenersLock);
    ServiceListenerMap& removedListenersMap = serviceEventListeners[context];
    serviceEventListeners.erase(context);
    if (!removedListenersMap.empty())
    {
      removedListenersMapWrapper = Poco::Any(removedListenersMap);
    }
    }
    if (removedListenersMapWrapper.empty()) {
      return;
    }

    ServiceListenerMap& removedListenersMap = Poco::RefAnyCast<ServiceListenerMap>(removedListenersMapWrapper);
    for (ServiceListenerMap::iterator iter = removedListenersMap.begin();
        iter != removedListenersMap.end(); ++iter)
    {
      iter->second->MarkRemoved();
    }
    //notifyListenerHooks(removedListeners, false);
  }

  void ServiceRegistry::PublishServiceEvent(ServiceEvent::Type type, SmartPointer<ServiceReferenceImpl> reference) {
    ServiceEvent::Pointer event(new ServiceEvent(type, reference));

    /* Build the listener snapshot */
    typedef Poco::HashMap<BundleContextImpl::Pointer, std::set<Pair<Object::Pointer, Object::Pointer> > , Object::Hash> SnapshotType;
    SnapshotType listenerSnapshot;
    {
    Poco::Mutex::ScopedLock lock(serviceEventListenersLock);
      for (Poco::HashMap<BundleContextImpl::Pointer, ServiceListenerMap, Object::Hash>::Iterator iter = serviceEventListeners.begin();
          iter != serviceEventListeners.end(); ++iter)
      {
        BundleContextImpl::Pointer context = iter->first;
        ServiceListenerMap& listeners = iter->second;
        if (!listeners.empty()) {
          listenerSnapshot.insert(std::make_pair(context, listeners.entry_set<Object::Pointer, Object::Pointer>()));
        }
      }
    }

    /* shrink the snapshot.
     * keySet returns a Collection which cannot be added to and
     * removals from that collection will result in removals of the
     * entry from the snapshot.
     */
    //Collection/*<BundleContextImpl>*/shrinkable = listenerSnapshot.keySet();
    //notifyEventHooksPrivileged(event, shrinkable);
    if (listenerSnapshot.empty()) {
      return;
    }

    /* deliver the event to the snapshot */
    ListenerQueue queue(framework->NewListenerQueue());
    for (SnapshotType::Iterator iter = listenerSnapshot.begin(); iter != listenerSnapshot.end(); ++iter) {
      EventDispatcher::Pointer dispatcher(iter->first->GetEventDispatcher());
      queue.QueueListeners(iter->second, dispatcher);
    }
    queue.DispatchEventSynchronous(SERVICEEVENT, event);
  }

  long ServiceRegistry::GetNextServiceId() {
    Poco::Mutex::ScopedLock lock(mutex);
    long id = serviceid;
    serviceid++;
    return id;
  }

  /* @GuardedBy("this") */
  void ServiceRegistry::AddServiceRegistration(SmartPointer<BundleContextImpl> context,
      SmartPointer<ServiceRegistrationImpl> registration) {
    Poco::Mutex::ScopedLock lock(mutex);
    // Add the ServiceRegistrationImpl to the list of Services published by BundleContextImpl.
    std::set<ServiceRegistrationImpl::Pointer>& contextServices = publishedServicesByContext[context];

    contextServices.insert(registration);

    // Add the ServiceRegistrationImpl to the list of Services published by Class Name.
    std::vector<std::string> clazzes = registration->GetClasses();
    for (std::size_t i = 0, size = clazzes.size(); i < size; i++) {

      std::set<ServiceRegistrationImpl::Pointer>& services = publishedServicesByClass[clazzes[i]];
      services.insert(registration);
    }

    // Add the ServiceRegistrationImpl to the list of all published Services.
    allPublishedServices.insert(registration);
  }

  /* @GuardedBy("this") */
  void ServiceRegistry::RemoveServiceRegistration(
      SmartPointer<BundleContextImpl> context,
      SmartPointer<ServiceRegistrationImpl> serviceReg) {
    Poco::Mutex::ScopedLock lock(mutex);
    // Remove the ServiceRegistrationImpl from the list of Services published by BundleContextImpl.
    std::set<ServiceRegistrationImpl::Pointer>& contextServices = publishedServicesByContext[context];
    if (!contextServices.empty()) {
      contextServices.erase(serviceReg);
    }

    // Remove the ServiceRegistrationImpl from the list of Services published by Class Name.
    std::vector<std::string> clazzes = serviceReg->GetClasses();
    for (std::size_t i = 0, size = clazzes.size(); i < size; i++) {
      std::set<ServiceRegistrationImpl::Pointer>& services = publishedServicesByClass[clazzes[i]];
      services.erase(serviceReg);
    }

    // Remove the ServiceRegistrationImpl from the list of all published Services.
    allPublishedServices.erase(serviceReg);
  }

  std::vector<SmartPointer<ServiceRegistrationImpl> >
  ServiceRegistry::LookupServiceRegistrations(const std::string& clazz, SmartPointer<Filter> filter) {
    std::vector<ServiceRegistrationImpl::Pointer> result;
    {
    Poco::Mutex::ScopedLock lock(mutex);
      if (clazz.empty()) { /* all services */
        result.assign(allPublishedServices.begin(), allPublishedServices.end());
      } else {
        /* services registered under the class name */
        std::set<ServiceRegistrationImpl::Pointer>& services = publishedServicesByClass[clazz];
        result.assign(services.begin(), services.end());
      }

    }

    if (!filter) {
      return result;
    }

    for (std::vector<ServiceRegistrationImpl::Pointer>::iterator iter = result.begin(); iter != result.end(); ) {
      if (!filter->Match(ServiceReference::Pointer((*iter)->GetReferenceImpl()))) {
        iter = result.erase(iter);
      }
      else {
        ++iter;
      }
    }
    return result;
  }

  std::vector<SmartPointer<ServiceRegistrationImpl> >
  ServiceRegistry::LookupServiceRegistrations(SmartPointer<BundleContextImpl> context) {
    Poco::Mutex::ScopedLock lock(mutex);
    std::set<ServiceRegistrationImpl::Pointer>& result = publishedServicesByContext[context];

    return std::vector<ServiceRegistrationImpl::Pointer>(result.begin(), result.end());
  }

  void
  ServiceRegistry::ChangeRegistrationsToReferences(const std::vector<SmartPointer<ServiceRegistrationImpl> >& input, std::vector<SmartPointer<ServiceReferenceImpl> >& result) {
    for (std::vector<ServiceRegistrationImpl::Pointer>::const_iterator iter = input.begin(); iter != input.end(); ++iter) {
      result.push_back((*iter)->GetReferenceImpl());
    }
  }

  std::string ServiceRegistry::CheckServiceClass(const std::vector<std::string>& clazzes,
      const Object::Pointer serviceObject) {
    //TODO Reflection: only meaningful with some kind of reflection
//    ClassLoader cl = serviceObject.getClass().getClassLoader();
//
//    for (std::size_t i = 0, len = clazzes.size(); i < len; ++i) {
//      try {
//        Class serviceClazz = cl == null ? Class.forName(clazzes[i]) : cl.loadClass(clazzes[i]);
//        if (!serviceClazz.isInstance(serviceObject))
//          return clazzes[i];
//      } catch (ClassNotFoundException e) {
//        //This check is rarely done
//        if (extensiveCheckServiceClass(clazzes[i], serviceObject.getClass()))
//          return clazzes[i];
//      }
//    }
    return "";
  }

}
}
}
