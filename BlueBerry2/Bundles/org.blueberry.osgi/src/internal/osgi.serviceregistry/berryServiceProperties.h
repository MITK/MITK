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


#ifndef BERRYSERVICEPROPERTIES_H_
#define BERRYSERVICEPROPERTIES_H_

#include <osgi/util/Headers.h>

#include <Poco/Mutex.h>

namespace berry {
namespace osgi {
namespace internal {

using namespace ::osgi::util;

/**
 * Hashtable for service properties.
 *
 * Supports case-insensitive key lookup.
 */
class ServiceProperties : public Headers {

private:

  mutable Poco::Mutex mutex;

  /**
   * Attempt to clone the value if necessary and possible.
   *
   * For some strange reason, you can test to see of an Object is
   * Cloneable but you can't call the clone method since it is
   * protected on Object!
   *
   * @param value object to be cloned.
   * @return cloned object or original object if we didn't clone it.
   */
  static Object::Pointer CloneValue(Object::Pointer value);


public:

  osgiObjectMacro(berry::osgi::internal::ServiceProperties)

  /**
   * Create a properties object for the service.
   *
   * @param props The properties for this service.
   */
  ServiceProperties(const Dictionary& props);

  /**
   * Get a clone of the value of a service's property.
   *
   * @param key header name.
   * @return Clone of the value of the property or <code>null</code> if there is
   * no property by that name.
   */
  Object::Pointer GetProperty(const std::string& key) const;

  /**
   * Put a clone of the property value into this property object.
   *
   * @param key Name of property.
   * @param value Value of property.
   * @return previous property value.
   */
  Object::Pointer SetProperty(const std::string& key, Object::Pointer value);

  std::string ToString() const;
};

}
}
}


#endif /* BERRYSERVICEPROPERTIES_H_ */
