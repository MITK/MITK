/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef US_SERVICE_PROPERTIES_H
#define US_SERVICE_PROPERTIES_H


#include <usGlobalConfig.h>
#include <MitkCppMicroServicesExports.h>
#include <unordered_map>
#include <unordered_set>

#include <cctype>

#include <usAny.h>

namespace us {

/**
 * \ingroup MicroServices
 *
 * A hash table with std::string as the key type and Any as the value
 * type. It is typically used for passing service properties to
 * ModuleContext::RegisterService.
 */
typedef std::unordered_map<std::string, Any> ServiceProperties;

/**
 * \ingroup MicroServices
 */
namespace ServiceConstants {

/**
 * \brief Service property identifying all of the class names under which a service
 * was registered in the framework. The value of this property must be of
 * type <code>std::vector&lt;std::string&gt;</code>.
 *
 * <p>
 * This property is set by the framework when a service is registered.
 */
MITKCPPMICROSERVICES_EXPORT const std::string& OBJECTCLASS(); // = "objectclass"

/**
 * \brief Service property identifying a service's registration number. The value
 * of this property must be of type <code>long int</code>.
 *
 * <p>
 * The value of this property is assigned by the framework when a service is
 * registered. The framework assigns a unique value that is larger than all
 * previously assigned values since the framework was started. These values
 * are NOT persistent across restarts of the framework.
 */
MITKCPPMICROSERVICES_EXPORT const std::string& SERVICE_ID(); // = "service.id"

/**
 * \brief Service property identifying a service's ranking number.
 *
 * <p>
 * This property may be supplied in the
 * <code>ServiceProperties</code> object passed to the
 * <code>ModuleContext::RegisterService</code> method. The value of this
 * property must be of type <code>int</code>.
 *
 * <p>
 * The service ranking is used by the framework to determine the <i>natural
 * order</i> of services, see ServiceReference::operator<(const ServiceReference&),
 * and the <i>default</i> service to be returned from a call to the
 * {@link ModuleContext::GetServiceReference} method.
 *
 * <p>
 * The default ranking is zero (0). A service with a ranking of
 * <code>std::numeric_limits<int>::max()</code> is very likely to be returned as the
 * default service, whereas a service with a ranking of
 * <code>std::numeric_limits<int>::min()</code> is very unlikely to be returned.
 *
 * <p>
 * If the supplied property value is not of type <code>int</code>, it is
 * deemed to have a ranking value of zero.
 */
MITKCPPMICROSERVICES_EXPORT const std::string& SERVICE_RANKING(); // = "service.ranking"

/**
 * \brief Service property identifying a service's scope.
 * This property is set by the framework when a service is registered. If the
 * registered object implements PrototypeServiceFactory, then the value of this
 * service property will be SCOPE_PROTOTYPE(). Otherwise, if the registered
 * object implements ServiceFactory, then the value of this service property will
 * be SCOPE_MODULE(). Otherwise, the value of this service property will be
 * SCOPE_SINGLETON().
 */
MITKCPPMICROSERVICES_EXPORT const std::string& SERVICE_SCOPE(); // = "service.scope"

/**
 * \brief Service scope is singleton. All modules using the service receive the same
 * service object.
 *
 * \sa SERVICE_SCOPE()
 */
MITKCPPMICROSERVICES_EXPORT const std::string& SCOPE_SINGLETON(); // = "singleton"

/**
 * \brief Service scope is module. Each module using the service receives a distinct
 * service object.
 *
 * \sa SERVICE_SCOPE()
 */
MITKCPPMICROSERVICES_EXPORT const std::string& SCOPE_MODULE(); // = "module"

/**
 * \brief Service scope is prototype. Each module using the service receives either
 * a distinct service object or can request multiple distinct service objects
 * via ServiceObjects.
 *
 * \sa SERVICE_SCOPE()
 */
MITKCPPMICROSERVICES_EXPORT const std::string& SCOPE_PROTOTYPE(); // = "prototype"

}

}

#endif // US_SERVICE_PROPERTIES_H
