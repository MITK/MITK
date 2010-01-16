/*=========================================================================
 
 Program:   BlueBerry Platform
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

#include "berryFilteredServiceListener.h"
#include "berryServiceReferenceImpl.h"
#include "berryServiceRegistrationImpl.h"
#include "../osgi.framework/berryBundleContextImpl.h"

#include <osgi/framework/Constants.h>
#include <supplement/framework.debug/berryDebug.h>

#include <sstream>

namespace berry {
namespace osgi {
namespace internal {

 FilteredServiceListener::FilteredServiceListener(SmartPointer<BundleContextImpl> context, ServiceListener::Pointer listener, const std::string& filterstring) throw(InvalidSyntaxException)
 : listener(listener), context(context), objectClassHash(0), matched(false), removed(false)
 {
    if (!filterstring.empty()) {
      FilterImpl::Pointer filterImpl = FilterImpl::NewInstance(filterstring);
      std::string clazz = filterImpl->GetRequiredObjectClass();
      if (clazz.empty()) {
        this->filter = filterImpl;
      } else {
        this->objectClass = clazz;
        this->objectClassHash = Poco::hash(clazz);
            if (filterstring == GetObjectClassFilterString(this->objectClass))
              {
              this->filter = filterImpl;
              }
      }
    }

  }

  void FilteredServiceListener::ServiceChanged(ServiceEvent::Pointer event) {
    ServiceReferenceImpl::Pointer reference = event->GetServiceReference().Cast<ServiceReferenceImpl>();

    // first check if we can short circuit the filter match if the required objectClass does not match the event
    bool breakif = false;
    if (!objectClassHash) {
      const std::vector<std::size_t>& classesHash = reference->GetClassesHash();
      std::size_t size = classesHash.size();
      for (std::size_t i = 0; i < size; i++) {
        if (classesHash[i] == objectClassHash)
        {
          breakif = true;
          break;
        }
      }
      if (!breakif)
        return; // no class in this event matches a required part of the filter; we do not need to deliver this event
    }

    if (Debug::DEBUG && Debug::DEBUG_EVENTS) {
      std::cout << "FilterServiceEvent(" << GetClassName() << "@" << std::hex << Object::HashCode() << ", \"" << GetFilter() << "\", " << reference->GetRegistration()->GetProperties() << ")";
    }


      if (filter) {

      bool deliverEvent = false;
      const bool match = filter->Match(event->GetServiceReference());
      {
        Poco::FastMutex::ScopedLock lock(matchMutex);

        if (match) { // if the filter matches now
          matched = true; // remember that the filter matched
          deliverEvent = true; // filter matched, so deliver event
        }
        else if (matched) { // if the filter does not match now, but it previously matched
          matched = false; // remember that the filter no longer matches
          if (event->GetType() == ServiceEvent::MODIFIED) {
            event = ServiceEvent::Pointer(new ServiceEvent(ServiceEvent::MODIFIED_ENDMATCH, reference));
            deliverEvent = true; // deliver a MODIFIED_ENDMATCH event
          }
        }
      }

      if (!deliverEvent)
        return; // there is a filter and it does not match, so do NOT deliver the event
    } // no filter, so deliver event


      if (Debug::DEBUG && Debug::DEBUG_EVENTS) {
        std::cout << "DispatchFilteredServiceEvent(" << listener->GetClassName() << "@" << std::hex << listener.GetPointer() << ")";
      }

      listener->ServiceChanged(event);
  }

  std::string FilteredServiceListener::ToString() const {
    std::stringstream str;
    str << listener->GetClassName() << "@" << std::hex << listener.GetPointer() << GetFilter();
    return str.str();
  }

  BundleContext::Pointer FilteredServiceListener::GetBundleContext() {
    return context;
  }

  std::string FilteredServiceListener::GetFilter() const {
    if (filter) {
      return filter->ToString();
    }
    return GetObjectClassFilterString(objectClass);
  }

  bool FilteredServiceListener::IsRemoved() const {
    return removed;
  }

  void FilteredServiceListener::MarkRemoved() {
    removed = true;
  }

  std::string FilteredServiceListener::GetObjectClassFilterString(const std::string& className) {
    if (className.empty()) {
      return "";
    }
    return "(" + Constants::OBJECTCLASS + "=" + className + ")";
  }

}
}
}
