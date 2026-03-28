/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USSERVICEEXCEPTION_H
#define USSERVICEEXCEPTION_H

#include <stdexcept>

#include <usGlobalConfig.h>
#include <MitkCppMicroServicesExports.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4275)
#endif

namespace us {

/**
 * \ingroup MicroServices
 *
 * A service exception used to indicate that a service problem occurred.
 *
 * <p>
 * A <code>ServiceException</code> object is created by the framework or
 * to denote an exception condition in the service. An enum
 * type is used to identify the exception type for future extendability.
 *
 * <p>
 * This exception conforms to the general purpose exception chaining mechanism.
 */
class MITKCPPMICROSERVICES_EXPORT ServiceException : public std::runtime_error
{
public:

  enum Type {
    /**
     * No exception type is unspecified.
     */
    UNSPECIFIED = 0,
    /**
     * The service has been unregistered.
     */
    UNREGISTERED = 1,
    /**
     * The service factory produced an invalid service object.
     */
    FACTORY_ERROR = 2,
    /**
     * The service factory threw an exception.
     */
    FACTORY_EXCEPTION = 3,
    /**
     * An error occurred invoking a remote service.
     */
    REMOTE = 5,
    /**
     * The service factory resulted in a recursive call to itself for the
     * requesting module.
     */
    FACTORY_RECURSION = 6
  };

  /**
   * Creates a <code>ServiceException</code> with the specified message,
   * type and exception cause.
   *
   * @param msg The associated message.
   * @param type The type for this exception.
   */
  ServiceException(const std::string& msg, const Type& type = UNSPECIFIED);

  ServiceException(const ServiceException& o);
  ServiceException& operator=(const ServiceException& o);

  ~ServiceException() throw() override { }

  /**
   * Returns the type for this exception or <code>UNSPECIFIED</code> if the
   * type was unspecified or unknown.
   *
   * @return The type of this exception.
   */
  Type GetType() const;

private:

  /**
   * Type of service exception.
   */
  Type type;

};

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

/**
 * \ingroup MicroServices
 * @{
 */
MITKCPPMICROSERVICES_EXPORT std::ostream& operator<<(std::ostream& os, const us::ServiceException& exc);
/** @}*/

#endif // USSERVICEEXCEPTION_H
