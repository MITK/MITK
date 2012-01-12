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

#ifndef MITKSERVICEREFERENCE_H
#define MITKSERVICEREFERENCE_H

#include <MitkExports.h>
#include <mitkAny.h>
#include <mitkConfig.h>

#ifdef MITK_HAS_UNORDERED_MAP_H
namespace std {
#elif defined(__GNUC__)
namespace __gnu_cxx {
#else
namespace itk {
#endif

template<class key> struct hash;

}

namespace mitk {

class Module;
class ServiceRegistrationPrivate;
class ServiceReferencePrivate;

/**
 * \ingroup MicroServices
 *
 * A reference to a service.
 *
 * <p>
 * The framework returns <code>ServiceReference</code> objects from the
 * <code>ModuleContext::GetServiceReference</code> and
 * <code>ModuleContext::GetServiceReferences</code> methods.
 * <p>
 * A <code>ServiceReference</code> object may be shared between modules and
 * can be used to examine the properties of the service and to get the service
 * object.
 * <p>
 * Every service registered in the framework has a unique
 * <code>ServiceRegistration</code> object and may have multiple, distinct
 * <code>ServiceReference</code> objects referring to it.
 * <code>ServiceReference</code> objects associated with a
 * <code>ServiceRegistration</code> are considered equal
 * (more specifically, their <code>operator==()</code>
 * method will return <code>true</code> when compared).
 * <p>
 * If the same service object is registered multiple times,
 * <code>ServiceReference</code> objects associated with different
 * <code>ServiceRegistration</code> objects are not equal.
 *
 * @see ModuleContext::GetServiceReference
 * @see ModuleContext::GetServiceReferences
 * @see ModuleContext::GetService
 * @remarks This class is thread safe.
 */
class MITK_CORE_EXPORT ServiceReference {

public:

  /**
   * Creates an invalid ServiceReference object. You can use
   * this object in boolean expressions and it will evaluate to
   * <code>false</code>.
   */
  ServiceReference();

  ServiceReference(const ServiceReference& ref);

  /**
   * Converts this ServiceReference instance into a boolean
   * expression. If this instance was default constructed or
   * the service it references has been unregistered, the conversion
   * returns <code>false</code>, otherwise it returns <code>true</code>.
   */
  operator bool() const;

  /**
   * Releases any resources held or locked by this
   * <code>ServiceReference</code> and renders it invalid.
   */
  ServiceReference& operator=(int null);

  ~ServiceReference();

  /**
   * Returns the property value to which the specified property key is mapped
   * in the properties <code>ServiceProperties</code> object of the service
   * referenced by this <code>ServiceReference</code> object.
   *
   * <p>
   * Property keys are case-insensitive.
   *
   * <p>
   * This method continues to return property values after the service has
   * been unregistered. This is so references to unregistered services can
   * still be interrogated.
   *
   * @param key The property key.
   * @return The property value to which the key is mapped; an invalid Any
   *         if there is no property named after the key.
   */
  Any GetProperty(const std::string& key) const;

  /**
   * Returns a list of the keys in the <code>ServiceProperties</code>
   * object of the service referenced by this <code>ServiceReference</code>
   * object.
   *
   * <p>
   * This method will continue to return the keys after the service has been
   * unregistered. This is so references to unregistered services can
   * still be interrogated.
   *
   * @param keys A vector being filled with the property keys.
   */
  void GetPropertyKeys(std::vector<std::string>& keys) const;

  /**
   * Returns the module that registered the service referenced by this
   * <code>ServiceReference</code> object.
   *
   * <p>
   * This method must return <code>0</code> when the service has been
   * unregistered. This can be used to determine if the service has been
   * unregistered.
   *
   * @return The module that registered the service referenced by this
   *         <code>ServiceReference</code> object; <code>0</code> if that
   *         service has already been unregistered.
   * @see ModuleContext::RegisterService(const std::vector<std::string>&, itk::LightObject*, const ServiceProperties&)
   */
  Module* GetModule() const;

  /**
   * Returns the modules that are using the service referenced by this
   * <code>ServiceReference</code> object. Specifically, this method returns
   * the modules whose usage count for that service is greater than zero.
   *
   * @param modules A list of modules whose usage count for the service referenced
   *         by this <code>ServiceReference</code> object is greater than
   *         zero.
   */
  void GetUsingModules(std::vector<Module*>& modules) const;

  /**
   * Compares this <code>ServiceReference</code> with the specified
   * <code>ServiceReference</code> for order.
   *
   * <p>
   * If this <code>ServiceReference</code> and the specified
   * <code>ServiceReference</code> have the same {@link ServiceProperties::SERVICE_ID
   * service id} they are equal. This <code>ServiceReference</code> is less
   * than the specified <code>ServiceReference</code> if it has a lower
   * {@link ServiceProperties::SERVICE_RANKING service ranking} and greater if it has a
   * higher service ranking. Otherwise, if this <code>ServiceReference</code>
   * and the specified <code>ServiceReference</code> have the same
   * {@link ServiceProperties::SERVICE_RANKING service ranking}, this
   * <code>ServiceReference</code> is less than the specified
   * <code>ServiceReference</code> if it has a higher
   * {@link ServiceProperties::SERVICE_ID service id} and greater if it has a lower
   * service id.
   *
   * @param reference The <code>ServiceReference</code> to be compared.
   * @return Returns a false or true if this
   *         <code>ServiceReference</code> is less than or greater
   *         than the specified <code>ServiceReference</code>.
   */
  bool operator<(const ServiceReference& reference) const;

  bool operator==(const ServiceReference& reference) const;

  ServiceReference& operator=(const ServiceReference& reference);


protected:

  friend class ModulePrivate;
  friend class ModuleContext;
  friend class ServiceFilter;
  friend class ServiceRegistrationPrivate;
  friend class ServiceListeners;
  friend class LDAPFilter;

  template<class S, class T> friend class ServiceTracker;
  template<class S, class T> friend class ServiceTrackerPrivate;
  template<class S, class R, class T> friend class ModuleAbstractTracked;

#ifdef MITK_HAS_UNORDERED_MAP_H
  friend class std::hash<ServiceReference>;
#elif defined(__GNUC__)
  friend class __gnu_cxx::hash<ServiceReference>;
#else
  friend struct itk::hash<ServiceReference>;
#endif


  ServiceReference(ServiceRegistrationPrivate* reg);

  ServiceReferencePrivate* d;

};

}

MITK_CORE_EXPORT std::ostream& operator<<(std::ostream& os, const mitk::ServiceReference& serviceRef);

#ifdef MITK_HAS_UNORDERED_MAP_H
namespace std {
#elif defined(__GNUC__)
namespace __gnu_cxx {
#else
namespace itk {
#endif

template<> struct MITK_CORE_EXPORT hash<mitk::ServiceReference>
{
  std::size_t operator()(const mitk::ServiceReference& sr) const;
};

}

#endif // MITKSERVICEREFERENCE_H
