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


#include "mitkServiceRegistration.h"

namespace mitk {

/**
 * \ingroup MicroServices
 *
 * Allows services to provide customized service objects in the module
 * environment.
 *
 * <p>
 * When registering a service, a <code>ServiceFactory</code> object can be
 * used instead of a service object, so that the module developer can gain
 * control of the specific service object granted to a module that is using the
 * service.
 *
 * <p>
 * When this happens, the
 * <code>ModuleContext::GetService(const ServiceReference&)</code> method calls the
 * <code>ServiceFactory::GetService</code> method to create a service object
 * specifically for the requesting module. The service object returned by the
 * <code>ServiceFactory</code> is cached by the framework until the module
 * releases its use of the service.
 *
 * <p>
 * When the module's use count for the service equals zero (including the module
 * unloading or the service being unregistered), the
 * <code>ServiceFactory::UngetService</code> method is called.
 *
 * <p>
 * <code>ServiceFactory</code> objects are only used by the framework and are
 * not made available to other modules in the module environment. The framework
 * may concurrently call a <code>ServiceFactory</code>.
 *
 * @see ModuleContext#GetService
 * @remarks This class is thread safe.
 */
class ServiceFactory
{

public:

  /**
   * Creates a new service object.
   *
   * <p>
   * The Framework invokes this method the first time the specified
   * <code>module</code> requests a service object using the
   * <code>ModuleContext::GetService(const ServiceReference&)</code> method. The
   * service factory can then return a specific service object for each
   * module.
   *
   * <p>
   * The framework caches the value returned (unless it is 0),
   * and will return the same service object on any future call to
   * <code>ModuleContext::GetService</code> for the same modules. This means the
   * framework must not allow this method to be concurrently called for the
   * same module.
   *
   * @param module The module using the service.
   * @param registration The <code>ServiceRegistration</code> object for the
   *        service.
   * @return A service object that <strong>must</strong> be an instance of all
   *         the classes named when the service was registered.
   * @see ModuleContext#GetService
   */
  virtual itk::LightObject* GetService(Module* module, const ServiceRegistration& registration) = 0;

  /**
   * Releases a service object.
   *
   * <p>
   * The framework invokes this method when a service has been released by a
   * module. The service object may then be destroyed.
   *
   * @param module The Module releasing the service.
   * @param registration The <code>ServiceRegistration</code> object for the
   *        service.
   * @param service The service object returned by a previous call to the
   *        <code>ServiceFactory::getService</code> method.
   * @see ModuleContext#UngetService
   */
  virtual void UngetService(Module* module, const ServiceRegistration& registration,
                            itk::LightObject* service) = 0;
};

}

