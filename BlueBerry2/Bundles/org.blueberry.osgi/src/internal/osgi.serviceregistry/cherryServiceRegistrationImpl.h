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


#ifndef CHERRYSERVICEREGISTRATIONIMPL_H_
#define CHERRYSERVICEREGISTRATIONIMPL_H_

#include <osgi/framework/ServiceRegistration.h>

#include "cherryServiceProperties.h"


namespace osgi {
namespace framework {
  struct Framework;
  struct Bundle;
  struct BundleContext;
}
}

namespace cherry {
namespace osgi {
namespace internal {

using namespace ::osgi::framework;

class ServiceRegistry;
class ServiceReferenceImpl;
class BundleContextImpl;

/**
 * A registered service.
 *
 * The framework returns a ServiceRegistration object when a
 * {@link BundleContextImpl#registerService(String, Object, Dictionary) BundleContext.registerService}
 * method is successful. This object is for the private use of
 * the registering bundle and should not be shared with other bundles.
 * <p>The ServiceRegistration object may be used to update the properties
 * for the service or to unregister the service.
 *
 * <p>If the ServiceRegistration is garbage collected the framework may remove
 * the service. This implies that if a
 * bundle wants to keep its service registered, it should keep the
 * ServiceRegistration object referenced.
 *
 * @ThreadSafe
 */
class ServiceRegistrationImpl : public ServiceRegistration {

public:

  osgiObjectMacro(cherry::osgi::internal::ServiceRegistrationImpl)

private:

  const SmartPointer<ServiceRegistry> registry;

  /** context which registered this service. */
  const SmartPointer<BundleContextImpl> context;

  /** bundle which registered this service. */
  const SmartPointer<Bundle> bundle;

  /** service classes for this registration. */
  const std::vector<std::string> clazzes;

  /** service classes hash value for this registration. */
  std::vector<std::size_t> clazzesHash;

  /** service object for this registration. */
  const Object::Pointer service;

  /** Reference to this registration. */
  /* @GuardedBy("registrationLock") */
  SmartPointer<ServiceReferenceImpl> reference;

  /** List of contexts using the service.
   * List&lt;BundleContextImpl&gt;.
   * */
  /* @GuardedBy("registrationLock") */
  std::vector<SmartPointer<BundleContext> > contextsUsing;

  /** properties for this registration. */
  /* @GuardedBy("registrationLock") */
  ServiceProperties::Pointer properties;

  /** service id. */
  const long serviceid;

  /** service ranking. */
  /* @GuardedBy("registrationLock") */
  int serviceranking;

  /* internal object to use for synchronization */
  mutable Poco::Mutex registrationLock;
  Poco::Mutex registryLock;
  Poco::Mutex servicesInUseLock;
  Poco::Mutex useLock;

  /** The registration state */
  /* @GuardedBy("registrationLock") */
  int state; // = REGISTERED;
  static const int REGISTERED; // = 0x00;
  static const int UNREGISTERING; // = 0x01;
  static const int UNREGISTERED; // = 0x02;


   /**
   * Construct a properties object from the dictionary for this
   * ServiceRegistration.
   *
   * @param p The properties for this service.
   * @return A Properties object for this ServiceRegistration.
   */
  /* @GuardedBy("registrationLock") */
  ServiceProperties::Pointer CreateProperties(const Dictionary& p);


public:

  /**
   * Construct a ServiceRegistration and register the service
   * in the framework's service registry.
   *
   */
  ServiceRegistrationImpl(SmartPointer<ServiceRegistry> registry,
      SmartPointer<BundleContextImpl> context, const std::vector<std::string>& clazzes,
      Object::Pointer service);

  /**
   * Call after constructing this object to complete the registration.
   */
  void RegisterService(const Dictionary& props);

  /**
   * Update the properties associated with this service.
   *
   * <p>The key "objectClass" cannot be modified by this method. It's
   * value is set when the service is registered.
   *
   * <p>The following steps are followed to modify a service's properties:
   * <ol>
   * <li>The service's properties are replaced with the provided properties.
   * <li>A {@link ServiceEvent} of type {@link ServiceEvent#MODIFIED}
   * is synchronously sent.
   * </ol>
   *
   * @param props The properties for this service.
   *        Changes should not be made to this object after calling this method.
   *        To update the service's properties this method should be called again.
   * @exception java.lang.IllegalStateException If
   * this ServiceRegistration has already been unregistered.
   *
   * @exception IllegalArgumentException If the <tt>properties</tt>
   * parameter contains case variants of the same key name.
   */
  void SetProperties(const Dictionary& props) throw(IllegalStateException, IllegalArgumentException);

  /**
   * Unregister the service.
   * Remove a service registration from the framework's service
   * registry.
   * All {@link ServiceReferenceImpl} objects for this registration
   * can no longer be used to interact with the service.
   *
   * <p>The following steps are followed to unregister a service:
   * <ol>
   * <li>The service is removed from the framework's service
   * registry so that it may no longer be used.
   * {@link ServiceReferenceImpl}s for the service may no longer be used
   * to get a service object for the service.
   * <li>A {@link ServiceEvent} of type {@link ServiceEvent#UNREGISTERING}
   * is synchronously sent so that bundles using this service
   * may release their use of the service.
   * <li>For each bundle whose use count for this service is greater
   * than zero:
   * <ol>
   * <li>The bundle's use count for this service is set to zero.
   * <li>If the service was registered with a {@link ServiceFactory},
   * the {@link ServiceFactory#ungetService ServiceFactory.ungetService} method
   * is called to release the service object for the bundle.
   * </ol>
   * </ol>
   *
   * @exception java.lang.IllegalStateException If
   * this ServiceRegistration has already been unregistered.
   * @see BundleContextImpl#ungetService
   */
  void UnregisterService() throw(IllegalStateException);

  /**
   * Returns a {@link ServiceReferenceImpl} object for this registration.
   * The {@link ServiceReferenceImpl} object may be shared with other bundles.
   *
   * @exception IllegalStateException If
   * this ServiceRegistration has already been unregistered.
   * @return A {@link ServiceReferenceImpl} object.
   */
  SmartPointer<ServiceReference> GetReference() const throw(IllegalStateException);

  SmartPointer<ServiceReferenceImpl> GetReferenceImpl() const;

  /**
   * Return the properties object. This is for framework internal use only.
   * @return The service registration's properties.
   */
  ServiceProperties::Pointer GetProperties() const;

  /**
   * Get the value of a service's property.
   *
   * <p>This method will continue to return property values after the
   * service has been unregistered. This is so that references to
   * unregistered service can be interrogated.
   * (For example: ServiceReference objects stored in the log.)
   *
   * @param key Name of the property.
   * @return Value of the property or <code>null</code> if there is
   * no property by that name.
   */
  Object::Pointer GetProperty(const std::string& key) const;

  /**
   * Get the list of key names for the service's properties.
   *
   * <p>This method will continue to return the keys after the
   * service has been unregistered. This is so that references to
   * unregistered service can be interrogated.
   * (For example: ServiceReference objects stored in the log.)
   *
   * @return The list of property key names.
   */
  std::vector<std::string> GetPropertyKeys() const;

  /**
   * Return the service id for this service.
   * @return The service id for this service.
   */
  long GetId() const;

  /**
   * Return the service ranking for this service.
   * @return The service ranking for this service.
   */
  int GetRanking() const;

  const std::vector<std::string>& GetClasses() const;

  const std::vector<std::size_t>& GetClassesHash() const;

  Object::Pointer GetServiceObject() const;

  /**
   * Return the bundle which registered the service.
   *
   * <p>This method will always return <code>null</code> when the
   * service has been unregistered. This can be used to
   * determine if the service has been unregistered.
   *
   * @return The bundle which registered the service.
   */
  SmartPointer<Bundle> GetBundle() const;

  /**
   * Get a service object for the using BundleContext.
   *
   * @param user BundleContext using service.
   * @return Service object
   */
  Object::Pointer GetService(SmartPointer<BundleContextImpl> user);

  /**
   * Unget a service for the using BundleContext.
   *
   * @param user BundleContext using service.
   * @return <code>false</code> if the context bundle's use count for the service
   *         is zero or if the service has been unregistered,
   *         otherwise <code>true</code>.
   */
  bool UngetService(SmartPointer<BundleContextImpl> user);

  /**
   * Release the service for the using BundleContext.
   *
   * @param user BundleContext using service.
   */
  void ReleaseService(SmartPointer<BundleContextImpl> user);

  /**
   * Return the list of bundle which are using this service.
   *
   * @return Array of Bundles using this service.
   */
  std::vector<SmartPointer<Bundle> > GetUsingBundles() const;

//  boolean isAssignableTo(Bundle client, String className) {
//    return framework.isServiceAssignableTo(bundle, client, className, service.getClass());
//  }

  /**
   * Return a String representation of this object.
   *
   * @return String representation of this object.
   */
  std::string ToString() const;

  /**
   * Compares this <code>ServiceRegistrationImpl</code> with the specified
   * <code>ServiceRegistrationImpl</code> for order.
   *
   * <p>
   * This does a reverse comparison so that the highest item is sorted to the left.
   * We keep ServiceRegistationImpls in sorted lists such that the highest
   * ranked service is at element 0 for quick retrieval.
   *
   * @param object The <code>ServiceRegistrationImpl</code> to be compared.
   * @return Returns a negative integer, zero, or a positive integer if this
   *         <code>ServiceRegistrationImpl</code> is greater than, equal to, or
   *         less than the specified <code>ServiceRegistrationImpl</code>.
   */
  bool operator<(const Object* object) const;

};

}
}
}

#endif /* CHERRYSERVICEREGISTRATIONIMPL_H_ */
