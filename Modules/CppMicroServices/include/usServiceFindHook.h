/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USSERVICEFINDHOOK_H
#define USSERVICEFINDHOOK_H

#include <usServiceInterface.h>
#include <usShrinkableVector.h>

#include <string>

namespace us {

class Module;
class ModuleContext;
class ServiceReferenceBase;

/**
 * \ingroup MicroServices
 *
 * \brief Service Find Hook Service.
 *
 * <p>
 * Modules registering this service will be called during service find
 * (get service references) operations.
 *
 * \remarks Implementations of this interface are required to be thread-safe.
 */
struct MITKCPPMICROSERVICES_EXPORT ServiceFindHook
{

  /** \brief Destructor. */
  virtual ~ServiceFindHook();

  /**
   * \brief Find hook method. This method is called during the service find operation
   * (for example, ModuleContext::GetServiceReferences<S>()). This method can
   * filter the result of the find operation.
   *
   * \param[in] context The module context of the module performing the find
   *        operation.
   * \param[in] name The class name of the services to find or an empty string to
   *        find all services.
   * \param[in] filter The filter criteria of the services to find or an empty string
   *        for no filter criteria.
   * \param[in] references A list of Service References to be returned as a result of the
   *        find operation. The implementation of this method may remove
   *        service references from the list to prevent the references from being
   *        returned to the module performing the find operation.
   */
  virtual void Find(const ModuleContext* context, const std::string& name,
                    const std::string& filter, ShrinkableVector<ServiceReferenceBase>& references) = 0;
};

}

#endif // USSERVICEFINDHOOK_H
