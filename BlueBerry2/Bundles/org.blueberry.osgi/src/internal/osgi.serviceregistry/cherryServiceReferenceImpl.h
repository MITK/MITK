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


#ifndef CHERRYSERVICEREFERENCEIMPL_H_
#define CHERRYSERVICEREFERENCEIMPL_H_

#include <osgi/framework/ServiceReference.h>
#include <osgi/framework/Bundle.h>
#include <osgi/framework/BundleContext.h>


namespace cherry {
namespace osgi {
namespace internal {

using namespace ::osgi::framework;

class ServiceRegistrationImpl;

/**
 * A reference to a service.
 *
 * <p>
 * The Framework returns <code>ServiceReference</code> objects from the
 * <code>BundleContext#GetServiceReference</code> and
 * <code>BundleContext#GetServiceReferences</code> methods.
 * <p>
 * A <code>ServiceReference</code> object may be shared between bundles and
 * can be used to examine the properties of the service and to get the service
 * object.
 * <p>
 * Every service registered in the Framework has a unique
 * <code>ServiceRegistration</code> object and may have multiple, distinct
 * <code>ServiceReference</code> objects referring to it.
 * <code>ServiceReference</code> objects associated with a
 * <code>ServiceRegistration</code> object have the same <code>hashCode</code>
 * and are considered equal (more specifically, their <code>operator==()</code>
 * method will return <code>true</code> when compared).
 * <p>
 * If the same service object is registered multiple times,
 * <code>ServiceReference</code> objects associated with different
 * <code>ServiceRegistration</code> objects are not equal.
 *
 * @see BundleContext#GetServiceReference
 * @see BundleContext#GetServiceReferences
 * @see BundleContext#GetService
 * @ThreadSafe
 */
class ServiceReferenceImpl : public ServiceReference {

private:

  /** Registered Service object. */
  ServiceRegistrationImpl * const registration;

public:

  osgiObjectMacro(cherry::osgi::internal::ServiceReferenceImpl)

  /**
   * Construct a reference.
   *
   */
  ServiceReferenceImpl(ServiceRegistrationImpl* registration);

  /**
   * Returns the property value to which the specified property key is mapped
   * in the properties <code>Dictionary</code> object of the service
   * referenced by this <code>ServiceReference</code> object.
   *
   * <p>
   * Property keys are case-insensitive.
   *
   * <p>
   * This method must continue to return property values after the service has
   * been unregistered. This is so references to unregistered services (for
   * example, <code>ServiceReference</code> objects stored in the log) can
   * still be interrogated.
   *
   * @param key The property key.
   * @return The property value to which the key is mapped; <code>null</code>
   *         if there is no property named after the key.
   */
  Object::Pointer GetProperty(const std::string& key) const;

  /**
   * Returns an array of the keys in the properties <code>Dictionary</code>
   * object of the service referenced by this <code>ServiceReference</code>
   * object.
   *
   * <p>
   * This method will continue to return the keys after the service has been
   * unregistered. This is so references to unregistered services (for
   * example, <code>ServiceReference</code> objects stored in the log) can
   * still be interrogated.
   *
   * <p>
   * This method is <i>case-preserving </i>; this means that every key in the
   * returned array must have the same case as the corresponding key in the
   * properties <code>Dictionary</code> that was passed to the
   * {@link BundleContext#RegisterService(const std::vector<std::string>&,Object::Pointer,const Poco::Hashmap&)}
   * or {@link ServiceRegistration#SetProperties} methods.
   *
   * @return An array of property keys.
   */
  std::vector<std::string> GetPropertyKeys() const;

  /**
   * Returns the bundle that registered the service referenced by this
   * <code>ServiceReference</code> object.
   *
   * <p>
   * This method must return <code>null</code> when the service has been
   * unregistered. This can be used to determine if the service has been
   * unregistered.
   *
   * @return The bundle that registered the service referenced by this
   *         <code>ServiceReference</code> object; <code>null</code> if
   *         that service has already been unregistered.
   * @see BundleContext#RegisterService(const std::vector<std::string>&,Object::Pointer, const Dictionary&)
   */
  SmartPointer<Bundle> GetBundle() const;

  /**
   * Returns the bundles that are using the service referenced by this
   * <code>ServiceReference</code> object. Specifically, this method returns
   * the bundles whose usage count for that service is greater than zero.
   *
   * @return An array of bundles whose usage count for the service referenced
   *         by this <code>ServiceReference</code> object is greater than
   *         zero; <code>null</code> if no bundles are currently using that
   *         service.
   *
   */
  std::vector<SmartPointer<Bundle> > GetUsingBundles() const;

  /**
   * Tests if the bundle that registered the service referenced by this
   * <code>ServiceReference</code> and the specified bundle use the same
   * source for the package of the specified class name.
   * <p>
   * This method performs the following checks:
   * <ol>
   * <li>Get the package name from the specified class name.</li>
   * <li>For the bundle that registered the service referenced by this
   * <code>ServiceReference</code> (registrant bundle); find the source for
   * the package. If no source is found then return <code>true</code> if the
   * registrant bundle is equal to the specified bundle; otherwise return
   * <code>false</code>.</li>
   * <li>If the package source of the registrant bundle is equal to the
   * package source of the specified bundle then return <code>true</code>;
   * otherwise return <code>false</code>.</li>
   * </ol>
   *
   * @param bundle The <code>Bundle</code> object to check.
   * @param className The class name to check.
   * @return <code>true</code> if the bundle which registered the service
   *         referenced by this <code>ServiceReference</code> and the
   *         specified bundle use the same source for the package of the
   *         specified class name. Otherwise <code>false</code> is returned.
   *
   * @since 1.3
   */
  //bool IsAssignableTo(SmartPointer<Bundle> bundle, const std::string& className) {
  //  return registration.isAssignableTo(bundle, className);
  //}

  /**
   * Compares this <code>ServiceReference</code> with the specified
   * <code>ServiceReference</code> for order.
   *
   * <p>
   * If this <code>ServiceReference</code> and the specified
   * <code>ServiceReference</code> have the same
   * {@link Constants#SERVICE_ID service id} they are equal. This
   * <code>ServiceReference</code> is less than the specified
   * <code>ServiceReference</code> if it has a lower
   * {@link Constants#SERVICE_RANKING service ranking} and greater if it has a
   * higher service ranking. Otherwise, if this <code>ServiceReference</code>
   * and the specified <code>ServiceReference</code> have the same
   * {@link Constants#SERVICE_RANKING service ranking}, this
   * <code>ServiceReference</code> is less than the specified
   * <code>ServiceReference</code> if it has a higher
   * {@link Constants#SERVICE_ID service id} and greater if it has a lower
   * service id.
   *
   * @param object The <code>ServiceReference</code> to be compared.
   * @return Returns <code>true</code> if this <code>ServiceReference</code> is
   *         less than the specified <code>ServiceReference</code>.
   */
  bool operator<(const Object* object) const;

  /**
   * Returns a hash code value for the object.
   *
   * @return  a hash code value for this object.
   */
  std::size_t HashCode() const;

  /**
   * Indicates whether some other object is "equal to" this one.
   *
   * @param   obj   the reference object with which to compare.
   * @return  <code>true</code> if this object is the same as the obj
   *          argument; <code>false</code> otherwise.
   */
  bool operator==(const Object* obj) const;

  /**
   * Return a string representation of this reference.
   *
   * @return String
   */
  std::string ToString() const;

  /**
   * Return the ServiceRegistrationImpl for this ServiceReferenceImpl.
   *
   * @return The ServiceRegistrationImpl for this ServiceReferenceImpl.
   */
  SmartPointer<ServiceRegistrationImpl> GetRegistration() const;

  /**
   * Return the classes under which the referenced service was registered.
   *
   * @return array of class names.
   */
  const std::vector<std::string>& GetClasses() const;

  const std::vector<std::size_t>& GetClassesHash() const;

};

}
}
}


#endif /* CHERRYSERVICEREFERENCEIMPL_H_ */
