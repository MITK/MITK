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

#include "berryServiceReferenceImpl.h"
#include "berryServiceRegistrationImpl.h"

namespace berry {
namespace osgi {
namespace internal {

ServiceReferenceImpl::ServiceReferenceImpl(ServiceRegistrationImpl* registration)
: registration(registration)
{
    /* We must not dereference registration in the constructor
     * since it is "leaked" to us in the ServiceRegistrationImpl
     * constructor.
     */
}

  Object::Pointer ServiceReferenceImpl::GetProperty(const std::string& key) const {
    return registration->GetProperty(key);
  }

  std::vector<std::string> ServiceReferenceImpl::GetPropertyKeys() const {
    return registration->GetPropertyKeys();
  }

  SmartPointer<Bundle> ServiceReferenceImpl::GetBundle() const {
    return registration->GetBundle();
  }

  std::vector<SmartPointer<Bundle> > ServiceReferenceImpl::GetUsingBundles() const {
    return registration->GetUsingBundles();
  }

  bool ServiceReferenceImpl::operator<(const Object* object) const {

    const ServiceReferenceImpl* ref = dynamic_cast<const ServiceReferenceImpl*>(object);
    if (!ref) throw IllegalArgumentException();

    const ServiceRegistrationImpl* other = ref->registration;

    const int thisRanking = registration->GetRanking();
    const int otherRanking = other->GetRanking();
    if (thisRanking != otherRanking) {
      if (thisRanking < otherRanking) {
        return -1;
      }
      return 1;
    }
    const long thisId = registration->GetId();
    const long otherId = other->GetId();
    if (thisId == otherId) {
      return 0;
    }
    if (thisId < otherId) {
      return 1;
    }
    return -1;
  }

  std::size_t ServiceReferenceImpl::HashCode() const {
    return registration->HashCode();
  }

  bool ServiceReferenceImpl::operator==(const Object* obj) const {
    if (obj == this) {
      return true;
    }

    if (const ServiceReferenceImpl* other = dynamic_cast<const ServiceReferenceImpl*>(obj)) {
    return registration == other->registration;
    }

    return false;
  }

  std::string ServiceReferenceImpl::ToString() const {
    return registration->ToString();
  }

  SmartPointer<ServiceRegistrationImpl> ServiceReferenceImpl::GetRegistration() const {
    ServiceRegistrationImpl::Pointer reg(registration);
    return reg;
  }

  const std::vector<std::string>& ServiceReferenceImpl::GetClasses() const {
    return registration->GetClasses();
  }

  const std::vector<std::size_t>& ServiceReferenceImpl::GetClassesHash() const {
    return registration->GetClassesHash();
  }

}
}
}
