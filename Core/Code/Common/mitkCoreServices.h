/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKCORESERVICES_H
#define MITKCORESERVICES_H

#include "MitkCoreExports.h"

#include <mitkLogMacros.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceInterface.h>
#include <usServiceReference.h>

#include <cassert>

namespace mitk {

struct IShaderRepository;
class IPropertyAliases;
class IPropertyDescriptions;
class IPropertyExtensions;
class IPropertyFilters;
class ISurfaceCutterFactory;

/**
 * @brief Access MITK core services.
 *
 * This class can be used to conveniently access common
 * MITK Core service objects. Some getter methods where implementations
 * exist in the core library are guaranteed to return a non-NULL service object.
 *
 * To ensure that CoreServices::Unget() is called after the caller
 * has finished using a service object, you should use the CoreServicePointer
 * helper class which calls Unget() when it goes out of scope:
 *
 * \code
 * CoreServicePointer<IShaderRepository> shaderRepo(CoreServices::GetShaderRepository());
 * // Do something with shaderRepo
 * \endcode
 *
 * @see CoreServicePointer
 */
class MITK_CORE_EXPORT CoreServices
{
public:

  /**
   * @brief Get an IShaderRepository instance.
   * @param context The module context of the module getting the service.
   * @return A IShaderRepository instance which can be NULL.
   */
  static IShaderRepository* GetShaderRepository();

  /**
   * @brief Get an IPropertyAliases instance.
   * @param context The module context of the module getting the service.
   * @return A non-NULL IPropertyAliases instance.
   */
  static IPropertyAliases* GetPropertyAliases(us::ModuleContext* context = us::GetModuleContext());

  /**
   * @brief Get an IPropertyDescriptions instance.
   * @param context The module context of the module getting the service.
   * @return A non-NULL IPropertyDescriptions instance.
   */
  static IPropertyDescriptions* GetPropertyDescriptions(us::ModuleContext* context = us::GetModuleContext());

  /**
   * @brief Get an IPropertyExtensions instance.
   * @param context The module context of the module getting the service.
   * @return A non-NULL IPropertyExtensions instance.
   */
  static IPropertyExtensions* GetPropertyExtensions(us::ModuleContext* context = us::GetModuleContext());

  /**
   * @brief Get an IPropertyFilters instance.
   * @param context The module context of the module getting the service.
   * @return A non-NULL IPropertyFilters instance.
   */
  static IPropertyFilters* GetPropertyFilters(us::ModuleContext* context = us::GetModuleContext());

  /**
  * @brief Get an IPropertyDescriptions instance.
  * @param context The module context of the module getting the service.
  * @return A non-NULL IPropertyDescriptions instance.
  */
  static ISurfaceCutterFactory* GetSurfaceCutterFactory(us::ModuleContext* context = us::GetModuleContext());

  /**
   * @brief Unget a previously acquired service instance.
   * @param service The service instance to be released.
   * @return \c true if ungetting the service was successful, \c false otherwise.
   */
  template<class S>
  static bool Unget(S* service, us::ModuleContext* context = us::GetModuleContext())
  {
    return Unget(context, us_service_interface_iid<S>(), service);
  }

private:

  static bool Unget(us::ModuleContext* context, const std::string& interfaceId, void* service);

  // purposely not implemented
  CoreServices();
  CoreServices(const CoreServices&);
  CoreServices& operator=(const CoreServices&);
};

/**
 * @brief A RAII helper class for core service objects.
 *
 * This is class is intended for usage in local scopes; it calls
 * CoreServices::Unget(S*) in its destructor. You should not construct
 * multiple CoreServicePointer instances using the same service pointer,
 * unless it is retrieved by a new call to a CoreServices getter method.
 *
 * @see CoreServices
 */
template<class S>
class CoreServicePointer
{
public:

  explicit CoreServicePointer(S* service)
    : m_service(service)
  {
    assert(m_service);
  }

  ~CoreServicePointer()
  {
    try
    {
      CoreServices::Unget(m_service);
    }
    catch (const std::exception& e)
    {
      MITK_ERROR << e.what();
    }
    catch (...)
    {
      MITK_ERROR << "Ungetting core service failed.";
    }
  }

  S* operator->() const
  {
    return m_service;
  }

private:

  // purposely not implemented
  CoreServicePointer(const CoreServicePointer&);
  CoreServicePointer& operator=(const CoreServicePointer&);

  S* const m_service;
};

}

#endif // MITKCORESERVICES_H
