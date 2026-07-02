/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCoreServices_h
#define mitkCoreServices_h

#include <MitkCoreExports.h>

#include <mitkCommon.h>
#include <mitkLog.h>

#include <mitkServiceInterface.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceReference.h>

#include <cassert>

namespace mitk
{
  struct IMimeTypeProvider;
  class IDataStorageService;
  class INodeSelectionService;
  class IPropertyAliases;
  class IPropertyDescriptions;
  class IPropertyDeserialization;
  class IPropertyExtensions;
  class IPropertyFilters;
  class IPropertyPersistence;
  class IPropertyRelations;
  class IPropertyTransience;
  class IPreferencesService;

  /**
   * \brief Provides convenient static access to common MITK core service objects.
   *
   * Most getter methods are guaranteed to return a non-nullptr service object
   * when the corresponding implementation is registered in the core library.
   * To ensure proper lifetime management, wrap the returned pointer in a
   * CoreServicePointer which calls Unget() automatically on destruction:
   *
   * \code
   * CoreServicePointer<IMimeTypeProvider> mime(CoreServices::GetMimeTypeProvider());
   * // Use mime->...
   * \endcode
   *
   * \sa CoreServicePointer
   * \ingroup Core
   */
  class MITKCORE_EXPORT CoreServices
  {
  public:

    /**
     * @brief Get an IDataStorageService instance.
     * @param context The module context of the module getting the service.
     * @return An IDataStorageService instance, or nullptr if no implementation is registered.
     * @note Unlike other core services, this may return nullptr if no plugin has registered
     *       an implementation (e.g., in headless/testing scenarios).
     */
    static IDataStorageService* GetDataStorageService(us::ModuleContext* context = us::GetModuleContext());

    /**
     * @brief Get an INodeSelectionsService instance.
     * @param context The module context of the module getting the service.
     * @return A non-nullptr INodeSelectionsService instance.
     */
    static INodeSelectionService* GetNodeSelectionService(us::ModuleContext* context = us::GetModuleContext());

    /**
     * @brief Get an IPropertyAliases instance.
     * @param context The module context of the module getting the service.
     * @return A non-nullptr IPropertyAliases instance.
     */
    static IPropertyAliases *GetPropertyAliases(us::ModuleContext *context = us::GetModuleContext());

    /**
     * @brief Get an IPropertyDescriptions instance.
     * @param context The module context of the module getting the service.
     * @return A non-nullptr IPropertyDescriptions instance.
     */
    static IPropertyDescriptions *GetPropertyDescriptions(us::ModuleContext *context = us::GetModuleContext());

    /**
     * @brief Get an IPropertyDeserialization instance.
     * @param context The module context of the module getting the service.
     * @return A non-nullptr IPropertyDeserialization instance.
     */
    static IPropertyDeserialization* GetPropertyDeserialization(us::ModuleContext* context = us::GetModuleContext());

    /**
     * @brief Get an IPropertyExtensions instance.
     * @param context The module context of the module getting the service.
     * @return A non-nullptr IPropertyExtensions instance.
     */
    static IPropertyExtensions *GetPropertyExtensions(us::ModuleContext *context = us::GetModuleContext());

    /**
     * @brief Get an IPropertyFilters instance.
     * @param context The module context of the module getting the service.
     * @return A non-nullptr IPropertyFilters instance.
     */
    static IPropertyFilters *GetPropertyFilters(us::ModuleContext *context = us::GetModuleContext());

    /**
    * @brief Get an IPropertyPersistence instance.
    * @param context The module context of the module getting the service.
    * @return A non-nullptr IPropertyPersistence instance.
    */
    static IPropertyPersistence *GetPropertyPersistence(us::ModuleContext *context = us::GetModuleContext());

    /**
    * @brief Get an IPropertyRelations instance.
    * @param context The module context of the module getting the service.
    * @return A non-nullptr IPropertyRelations instance.
    */
    static IPropertyRelations *GetPropertyRelations(us::ModuleContext *context = us::GetModuleContext());

    /**
    * @brief Get an IPropertyTransience instance.
    * @param context The module context of the module getting the service.
    * @return A non-nullptr IPropertyTransience instance.
    */
    static IPropertyTransience *GetPropertyTransience(us::ModuleContext *context = us::GetModuleContext());

    /**
     * @brief Get an IMimeTypeProvider instance.
     * @param context The module context of the module getting the service.
     * @return A non-nullptr IMimeTypeProvider instance.
     */
    static IMimeTypeProvider *GetMimeTypeProvider(us::ModuleContext *context = us::GetModuleContext());

    /**
     * @brief Get an IPreferencesService instance.
     * @param context The module context of the module getting the service.
     * @return A non-nullptr IPreferencesService instance.
     * @sa IPreferences
     */
    static IPreferencesService *GetPreferencesService(us::ModuleContext *context = us::GetModuleContext());

    /**
     * \brief Release a previously acquired service instance.
     *
     * \tparam S The service interface type.
     * \param[in] service The service instance to release.
     * \param[in] context The module context used to acquire the service.
     * \return True if the service was successfully released, false otherwise.
     */
    template <class S>
    static bool Unget(S *service, us::ModuleContext *context = us::GetModuleContext())
    {
      return Unget(context, us_service_interface_iid<S>(), service);
    }

  private:
    static bool Unget(us::ModuleContext *context, const std::string &interfaceId, void *service);

    // purposely not implemented
    CoreServices();
    CoreServices(const CoreServices &);
    CoreServices &operator=(const CoreServices &);
  };

  /**
   * \brief RAII wrapper for core service objects.
   *
   * Intended for local-scope use: automatically calls CoreServices::Unget()
   * in its destructor. Do not construct multiple CoreServicePointer instances
   * from the same raw pointer unless each was obtained from a separate
   * CoreServices getter call.
   *
   * For optional services (e.g. IDataStorageService), check validity first:
   * \code
   * CoreServicePointer<IDataStorageService> dsService(CoreServices::GetDataStorageService());
   * if (dsService)
   * {
   *   auto storage = dsService->GetActiveDataStorage();
   * }
   * \endcode
   *
   * \tparam S The service interface type.
   *
   * \sa CoreServices
   * \ingroup Core
   */
  template <class S>
  class MITK_LOCAL CoreServicePointer
  {
  public:
    /**
     * \brief Construct a RAII wrapper around a service pointer.
     * \param[in] service The service pointer to manage (may be nullptr).
     * \param[in] context The module context used for Unget().
     */
    explicit CoreServicePointer(S *service, us::ModuleContext* context = us::GetModuleContext())
      : m_Service(service),
        m_Context(context)
    {
    }

    /**
     * \brief Destructor. Releases the managed service via CoreServices::Unget().
     */
    ~CoreServicePointer()
    {
      if (m_Service != nullptr)
      {
        try
        {
          CoreServices::Unget(m_Service, m_Context);
        }
        catch (const std::exception &e)
        {
          MITK_ERROR << e.what();
        }
        catch (...)
        {
          MITK_ERROR << "Ungetting core service failed.";
        }
      }
    }

    /**
     * \brief Check if this pointer holds a valid (non-null) service.
     * \return True if the service pointer is not nullptr.
     */
    explicit operator bool() const
    {
      return m_Service != nullptr;
    }

    /**
     * \brief Dereference the managed service pointer.
     * \return The raw service pointer.
     * \pre The service pointer must not be nullptr.
     */
    S *operator->() const
    {
      return m_Service;
    }

    /**
     * \brief Get the raw service pointer.
     * \return The service pointer (may be nullptr for optional services).
     */
    S *Get() const
    {
      return m_Service;
    }

    // Non-copyable
    CoreServicePointer(const CoreServicePointer&) = delete;
    CoreServicePointer& operator=(const CoreServicePointer&) = delete;

  private:
    S *const m_Service;            ///< The managed service pointer.
    us::ModuleContext* m_Context;  ///< Module context for Unget().
  };
}

#endif
