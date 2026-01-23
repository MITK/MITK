/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIDataStorageService_h
#define mitkIDataStorageService_h

#include <MitkCoreExports.h>
#include <mitkDataStorage.h>
#include <mitkDataStorageReference.h>
#include <mitkServiceInterface.h>

#include <optional>
#include <string>
#include <vector>

namespace mitk
{
  /**
   * \brief Service interface for managing DataStorage instances.
   *
   * This service provides access to one or more DataStorage instances within
   * the application. For most use cases, GetDefaultDataStorage() or
   * GetActiveDataStorage() is sufficient.
   *
   * The service supports multiple labeled DataStorage instances for advanced
   * scenarios like multi-document applications, though typical Workbench usage
   * only uses the default storage.
   *
   * Access this service via CoreServices::GetDataStorageService().
   *
   * \code
   * auto* dsService = mitk::CoreServices::GetDataStorageService();
   * if (dsService != nullptr)
   * {
   *   mitk::CoreServicePointer<mitk::IDataStorageService> servicePtr(dsService);
   *   auto storage = servicePtr->GetActiveDataStorage();
   *   // ...
   * }
   * \endcode
   *
   * \sa CoreServices::GetDataStorageService()
   * \sa DataStorageReference
   *
   * \ingroup MicroServices_Interfaces
   */
  class MITKCORE_EXPORT IDataStorageService
  {
  public:
    virtual ~IDataStorageService();

    /**
     * \brief Get the default (primary) DataStorage.
     *
     * The default DataStorage is created when the service starts and
     * persists for the lifetime of the application.
     *
     * \return The default DataStorage. Never nullptr after initialization.
     */
    virtual DataStorage::Pointer GetDefaultDataStorage() const = 0;

    /**
     * \brief Get the currently active DataStorage.
     *
     * Returns the active DataStorage if one has been set via
     * SetActiveDataStorage(), otherwise returns the default.
     *
     * This is the recommended method for most use cases where you need
     * the "current" DataStorage.
     *
     * \return The active DataStorage, or default if none is active.
     */
    virtual DataStorage::Pointer GetActiveDataStorage() const = 0;

    /**
     * \brief Get info about the currently active DataStorage.
     *
     * Similar to GetActiveDataStorage() but returns full info including label.
     *
     * \return Info about the active DataStorage.
     */
    virtual DataStorageReference GetActiveDataStorageReference() const = 0;

    /**
     * \brief Set the active DataStorage by label.
     *
     * Selects a registered DataStorage as the active one.
     *
     * \param label The label of the storage to make active.
     *              Use empty string to revert to the default.
     * \return true if the storage was found and set as active, false otherwise.
     */
    virtual bool SetActiveDataStorage(const std::string& label) = 0;

    /**
     * \brief Register an existing DataStorage with this service.
     *
     * Use this method to add a DataStorage that was created elsewhere
     * to the service's management. The service takes shared ownership.
     *
     * \param label Human-readable label for the storage. Must be unique.
     * \param storage The DataStorage to register.
     * \return true if registered successfully, false if label already exists
     * (then the state of the service isn't changed and no storage is added.)
     */
    virtual bool AddDataStorage(const std::string& label, DataStorage::Pointer storage) = 0;

    /**
     * \brief Create a new DataStorage with the given label.
     *
     * Creates a new StandaloneDataStorage and registers it with this service.
     * For registering an existing DataStorage, use AddDataStorage() instead.
     *
     * \param label Human-readable label for the storage. Must be unique.
     * \return Info about the newly created DataStorage, or invalid info if label exists.
     */
    virtual DataStorageReference CreateDataStorage(const std::string& label) = 0;

    /**
     * \brief Get info about a specific DataStorage by label.
     *
     * \param label The label to search for.
     * \return Info about the storage, or std::nullopt if not found.
     */
    virtual std::optional<DataStorageReference> GetDataStorageReference(const std::string& label) const = 0;

    /**
     * \brief Get the label of a registered DataStorage.
     *
     * Reverse lookup: given a DataStorage pointer, find its label.
     *
     * \param storage The DataStorage to look up.
     * \return The label, or std::nullopt if the storage is not registered.
     */
    virtual std::optional<std::string> GetLabel(const DataStorage* storage) const = 0;

    /**
     * \brief Get all labels of registered DataStorages.
     *
     * \return Vector of all labels. Always contains at least the default label.
     */
    virtual std::vector<std::string> GetAllLabels() const = 0;

    /**
     * \brief Get information about all registered DataStorages.
     *
     * The returned vector always contains at least the default DataStorage.
     *
     * \return Vector of all DataStorageReference objects.
     */
    virtual std::vector<DataStorageReference> GetAllDataStorages() const = 0;

    /**
     * \brief Check if a DataStorage with the given label exists.
     *
     * \param label The label to check.
     * \return true if a storage with this label exists, false otherwise.
     */
    virtual bool HasDataStorage(const std::string& label) const = 0;

    /**
     * \brief Remove a DataStorage from the service by label.
     *
     * The default DataStorage cannot be removed.
     * If the removed storage was the active storage, the active
     * storage is reset to the default.
     *
     * \param label The label of the storage to remove.
     * \return true if removed, false if not found or is default.
     */
    virtual bool RemoveDataStorage(const std::string& label) = 0;
  };

} // namespace mitk

MITK_DECLARE_SERVICE_INTERFACE(mitk::IDataStorageService, "org.mitk.IDataStorageService")

#endif
