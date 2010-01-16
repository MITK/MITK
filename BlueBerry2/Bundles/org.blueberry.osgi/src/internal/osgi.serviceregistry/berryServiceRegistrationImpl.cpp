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

#include "berryServiceRegistrationImpl.h"
#include "berryServiceReferenceImpl.h"
#include "berryServiceRegistry.h"
#include "berryServiceUse.h"

#include "../osgi.framework/berryBundleContextImpl.h"
#include "../osgi.framework/berryBundleHost.h"

#include <osgi/framework/Constants.h>
#include <osgi/framework/Objects.h>
#include <osgi/framework/ObjectVector.h>
#include <osgi/framework/ObjectString.h>

#include <supplement/framework.debug/berryDebug.h>

namespace berry
{
namespace osgi
{
namespace internal
{

const int ServiceRegistrationImpl::REGISTERED = 0x00;
const int ServiceRegistrationImpl::UNREGISTERING = 0x01;
const int ServiceRegistrationImpl::UNREGISTERED = 0x02;

ServiceProperties::Pointer ServiceRegistrationImpl::CreateProperties(
    const Dictionary& p)
{
  ServiceProperties::Pointer props(new ServiceProperties(p));

  props->operator[](Constants::OBJECTCLASS) = new ObjectVector<std::string>(clazzes);
  props->operator[](Constants::SERVICE_ID) = new ObjectLong(serviceid);
  props->SetReadOnly();
  Object::Pointer ranking = props->GetProperty(Constants::SERVICE_RANKING);

  if (ObjectInt::Pointer r = ranking.Cast<ObjectInt>())
  {
    serviceranking = r->GetValue();
  }
  else
  {
    serviceranking = 0;
  }

  return props;
}

ServiceRegistrationImpl::ServiceRegistrationImpl(
    SmartPointer<ServiceRegistry> registry,
    SmartPointer<BundleContextImpl> context,
    const std::vector<std::string>& clazzes, Object::Pointer service) :
  registry(registry), context(context), bundle(context->GetBundleImpl()),
      clazzes(clazzes), service(service), serviceid(
          registry->GetNextServiceId()), state(REGISTERED)
{
  Poco::Mutex::ScopedLock lock(registrationLock);
  /* We leak this from the constructor here, but it is ok
   * because the ServiceReferenceImpl constructor only
   * stores the value in a final field without
   * otherwise using it.
   */
  this->reference = new ServiceReferenceImpl(this);

}

void ServiceRegistrationImpl::RegisterService(const Dictionary& props)
{
  ServiceReferenceImpl::Pointer ref;
  {
    Poco::Mutex::ScopedLock lock(registryLock);
    context->CheckValid();
    {
      Poco::Mutex::ScopedLock lock2(registrationLock);
      ref = reference; /* used to publish event outside sync */
      this->properties = CreateProperties(props); /* must be valid after unregister is called. */
    }
    if (Debug::DEBUG && Debug::DEBUG_SERVICES)
    {
      std::cout << "registerService[" << bundle << "](" << this << ")";
    }
    registry->AddServiceRegistration(context, ServiceRegistrationImpl::Pointer(this));
  }

  /* must not hold the registrations lock when this event is published */
  registry->PublishServiceEvent(ServiceEvent::REGISTERED, ref);
}

void ServiceRegistrationImpl::SetProperties(const Dictionary& props) throw(IllegalStateException, IllegalArgumentException)
{
  ServiceReferenceImpl::Pointer ref;
  {
    Poco::Mutex::ScopedLock lock(registrationLock);
    if (state != REGISTERED)
    { /* in the process of unregistering */
      throw IllegalStateException("The service has been unregistered");
    }

    ref = reference; /* used to publish event outside sync */
    this->properties = CreateProperties(props);
  }

  /* must not hold the registrationLock when this event is published */
  registry->PublishServiceEvent(ServiceEvent::MODIFIED, ref);
}

void ServiceRegistrationImpl::UnregisterService() throw(IllegalStateException)
{
  ServiceReferenceImpl::Pointer ref;
  {
    Poco::Mutex::ScopedLock lock(registryLock);
    {
      Poco::Mutex::ScopedLock lock2(registrationLock);
      if (state != REGISTERED)
      { /* in the process of unregisterING */
        throw IllegalStateException("The service has been unregistered");
      }

      /* remove this object from the service registry */
      if (Debug::DEBUG && Debug::DEBUG_SERVICES)
      {
        std::cout << "unregisterService[" << bundle << "](" << this << ")";
      }

      registry->RemoveServiceRegistration(context, ServiceRegistrationImpl::Pointer(this));

      state = UNREGISTERING; /* mark unregisterING */
      ref = reference; /* used to publish event outside sync */
    }
  }

  /* must not hold the registrationLock when this event is published */
  registry->PublishServiceEvent(ServiceEvent::UNREGISTERING, ref);

  std::vector<BundleContextImpl::Pointer> users;
  {
    Poco::Mutex::ScopedLock lock(registrationLock);
    /* we have published the ServiceEvent, now mark the service fully unregistered */
    state = UNREGISTERED;

    if (!contextsUsing.empty())
    {
      if (Debug::DEBUG && Debug::DEBUG_SERVICES)
      {
        std::cout << "unregisterService: releasing users";
      }
      for (std::vector<BundleContext::Pointer>::iterator i =
          contextsUsing.begin(); i != contextsUsing.end(); ++i)
      {
        users.push_back(i->Cast<BundleContextImpl> ());
      }
    }
  }

  /* must not hold the registrationLock while releasing services */
  for (std::vector<BundleContextImpl::Pointer>::iterator i = users.begin(); i
      != users.end(); ++i)
  {
    ReleaseService(*i);
  }

  {
    Poco::Mutex::ScopedLock lock(registrationLock);
    contextsUsing.clear();
    reference = 0; /* mark registration dead */
  }

  /* The properties field must remain valid after unregister completes. */
}

SmartPointer<ServiceReference> ServiceRegistrationImpl::GetReference() const throw(IllegalStateException)
{
  return GetReferenceImpl();
}

SmartPointer<ServiceReferenceImpl> ServiceRegistrationImpl::GetReferenceImpl() const
{
  /* use reference instead of unregistered so that ServiceFactorys, called
   * by releaseService after the registration is unregistered, can
   * get the ServiceReference. Note this technically may violate the spec
   * but makes more sense.
   */
  Poco::Mutex::ScopedLock lock(registrationLock);
  if (!reference)
  {
    throw IllegalStateException("The service has been unregistered");
  }

  return reference;
}

ServiceProperties::Pointer ServiceRegistrationImpl::GetProperties() const
{
  Poco::Mutex::ScopedLock lock(registrationLock);
  return properties;
}

Object::Pointer ServiceRegistrationImpl::GetProperty(const std::string& key) const
{
  Poco::Mutex::ScopedLock lock(registrationLock);
  return properties->GetProperty(key);
}

std::vector<std::string> ServiceRegistrationImpl::GetPropertyKeys() const
{
  Poco::Mutex::ScopedLock lock(registrationLock);
  std::set<Dictionary::Key> keys = properties->KeySet();
  return std::vector<std::string>(keys.begin(), keys.end());
}

long ServiceRegistrationImpl::GetId() const
{
  return serviceid;
}

int ServiceRegistrationImpl::GetRanking() const
{
  Poco::Mutex::ScopedLock lock(registrationLock);
  return serviceranking;
}

const std::vector<std::string>& ServiceRegistrationImpl::GetClasses() const
{
  return clazzes;
}

Object::Pointer ServiceRegistrationImpl::GetServiceObject() const
{
  return service;
}

SmartPointer<Bundle> ServiceRegistrationImpl::GetBundle() const
{
  Poco::Mutex::ScopedLock lock(registrationLock);
  if (!reference)
  {
    return Bundle::Pointer(0);
  }

  return bundle;

}

Object::Pointer ServiceRegistrationImpl::GetService(SmartPointer<
    BundleContextImpl> user)
{
  {
    Poco::Mutex::ScopedLock lock(registrationLock);
    if (state == UNREGISTERED)
    { /* service unregistered */
      return Object::Pointer(0);
    }
  }

  if (Debug::DEBUG && Debug::DEBUG_SERVICES)
  {
    std::cout << "GetService[" << user->GetBundleImpl() << "](" << this << ")";
  }

  BundleContextImpl::ServiceUseMap servicesInUse = user->GetServicesInUseMap();
  if (servicesInUse.empty())
  { /* user is closed */
    user->CheckValid(); /* throw exception */
  }
  /* Use a while loop to support retry if a call to a ServiceFactory fails */
  while (true)
  {
    ServiceUse::Pointer use;
    bool added = false;
    /* Obtain the ServiceUse object for this service by bundle user */
    {
      Poco::Mutex::ScopedLock lock(servicesInUseLock);
      user->CheckValid();
      use = servicesInUse[ServiceRegistrationImpl::Pointer(this)];
      if (!use)
      {
        /* if this is the first use of the service
         * optimistically record this service is being used. */
        use = new ServiceUse(user, ServiceRegistrationImpl::Pointer(this));
        added = true;
        {
          Poco::Mutex::ScopedLock lock2(registrationLock);
          servicesInUse.insert(std::make_pair(ServiceRegistrationImpl::Pointer(
              this), use));
          contextsUsing.push_back(user);
        }
      }
    }

    /* Obtain and return the service object */
    {
      Poco::Mutex::ScopedLock lock(useLock);
      /* if another thread removed the ServiceUse, then
       * go back to the top and start again */
      {
        Poco::Mutex::ScopedLock lock2(servicesInUseLock);
        user->CheckValid();
        if (servicesInUse[ServiceRegistrationImpl::Pointer(this)] != use)
        {
          continue;
        }
      }
      Object::Pointer serviceObject = use->GetService();
      /* if the service factory failed to return an object and
       * we created the service use, then remove the
       * optimistically added ServiceUse. */
      if (!serviceObject && added)
      {
        Poco::Mutex::ScopedLock lock3(servicesInUseLock);
        Poco::Mutex::ScopedLock lock4(registrationLock);
        servicesInUse.erase(ServiceRegistrationImpl::Pointer(this));
        contextsUsing.erase(std::find(contextsUsing.begin(), contextsUsing.end(), user));
      }
      return serviceObject;
    }
  }
}

bool ServiceRegistrationImpl::UngetService(SmartPointer<BundleContextImpl> user)
{
  {
    Poco::Mutex::ScopedLock lock(registrationLock);
    if (state == UNREGISTERED)
    {
      return false;
    }
  }

  if (Debug::DEBUG && Debug::DEBUG_SERVICES)
  {
    std::cout << "UngetService[" << user->GetBundleImpl() << "](" << this
        << ")";
  }

  BundleContextImpl::ServiceUseMap servicesInUse = user->GetServicesInUseMap();
  if (servicesInUse.empty())
  {
    return false;
  }

  ServiceUse::Pointer use;
  {
    Poco::Mutex::ScopedLock lock(servicesInUseLock);
    use = servicesInUse[ServiceRegistrationImpl::Pointer(this)];
    if (!use)
    {
      return false;
    }
  }

  {
    Poco::Mutex::ScopedLock lock(useLock);
    if (use->UngetService())
    {
      /* use count is now zero */
      {
        Poco::Mutex::ScopedLock lock2(servicesInUseLock);
        Poco::Mutex::ScopedLock lock3(registrationLock);
        servicesInUse.erase(ServiceRegistrationImpl::Pointer(this));
        contextsUsing.erase(std::find(contextsUsing.begin(), contextsUsing.end(), user));
      }
    }
  }
  return true;
}

void ServiceRegistrationImpl::ReleaseService(
    SmartPointer<BundleContextImpl> user)
{
  {
    Poco::Mutex::ScopedLock lock(registrationLock);
    if (!reference)
    { /* registration dead */
      return;
    }
  }

  if (Debug::DEBUG && Debug::DEBUG_SERVICES)
  {
    std::cout << "ReleaseService[" << user->GetBundleImpl() << "](" << this
        << ")";
  }

  BundleContextImpl::ServiceUseMap servicesInUse = user->GetServicesInUseMap();
  if (servicesInUse.empty())
  {
    return;
  }
  ServiceUse::Pointer use;
  {
    Poco::Mutex::ScopedLock lock2(servicesInUseLock);
    Poco::Mutex::ScopedLock lock3(registrationLock);
    use = servicesInUse[ServiceRegistrationImpl::Pointer(this)];
    servicesInUse.erase(ServiceRegistrationImpl::Pointer(this));
    if (!use)
    {
      return;
    }
    // contextsUsing may have been nulled out by use.releaseService() if the registrant bundle
    // is listening for events and unregisters the service
    if (!contextsUsing.empty())
      contextsUsing.erase(std::find(contextsUsing.begin(), contextsUsing.end(), user));

  }
  {
    Poco::Mutex::ScopedLock lock(useLock);
    use->ReleaseService();
  }
}

std::vector<SmartPointer<Bundle> > ServiceRegistrationImpl::GetUsingBundles() const
{
  Poco::Mutex::ScopedLock lock(registrationLock);
  if (state == UNREGISTERED || contextsUsing.empty()) /* service unregistered */
    return std::vector<Bundle::Pointer>();

  /* Copy list of BundleContext into an array of Bundle. */
  std::vector<Bundle::Pointer> bundles;
  for (std::vector<BundleContext::Pointer>::const_iterator i =
      contextsUsing.begin(); i != contextsUsing.end(); ++i)
    bundles.push_back(i->Cast<BundleContextImpl>()->GetBundleImpl());

  return bundles;
}

std::string ServiceRegistrationImpl::ToString() const
{
  std::size_t size = clazzes.size();
  std::stringstream sb;

  sb << "{";

  for (std::size_t i = 0; i < size; ++i)
  {
    if (i > 0)
    {
      sb << ", ";
    }
    sb << clazzes[i];
  }

  sb << "}=" << GetProperties()->ToString();

  return sb.str();
}

bool ServiceRegistrationImpl::operator<(const Object* object) const
{
  if (const ServiceRegistrationImpl* other = dynamic_cast<const ServiceRegistrationImpl*>(object))
  {
    const int thisRanking = this->GetRanking();
    const int otherRanking = other->GetRanking();
    if (thisRanking != otherRanking)
    {
      return (thisRanking < otherRanking);
    }

    const long thisId = this->GetId();
    const long otherId = other->GetId();
    return (thisId < otherId);
  }
  else
  {
    return false;
  }
}

}
}
}
