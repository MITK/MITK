/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDataStorageService_h
#define mitkDataStorageService_h

#include <MitkCoreExports.h>
#include <mitkIDataStorageService.h>
#include <mitkStorageThreadDispatcherBase.h>

#include <mutex>
#include <vector>

namespace mitk
{

/**
 * \brief Default implementation of IDataStorageService.
 *
 * This service manages one or more DataStorage instances.
 * A default DataStorage is created on construction.
 *
 * This class is exported from Core so it can be instantiated by plugins
 * or other code that needs to register the service. The actual service
 * registration is typically done by an activator.
 *
 * Example usage in a plugin activator:
 * \code
 * #include <mitkDataStorageService.h>
 * #include <mitkIDataStorageService.h>
 * #include <usModuleContext.h>
 *
 * void MyActivator::start(ctkPluginContext* context)
 * {
 *   m_DataStorageService = std::make_unique<mitk::DataStorageService>();
 *   auto* moduleContext = us::GetModuleContext();
 *   m_ServiceReg = moduleContext->RegisterService<mitk::IDataStorageService>(m_DataStorageService.get());
 * }
 * \endcode
 */
class MITKCORE_EXPORT DataStorageService : public IDataStorageService
{
public:
  /** \brief The label used for the default DataStorage. */
  static const std::string DEFAULT_LABEL;

  /**
   * \brief Constructor. Creates a default StandaloneDataStorage.
   */
  DataStorageService();

  /** \brief Destructor. */
  ~DataStorageService() override;

  /** \brief \sa IDataStorageService::GetDefaultDataStorage() */
  DataStorage::Pointer GetDefaultDataStorage() const override;

  /** \brief \sa IDataStorageService::GetActiveDataStorage() */
  DataStorage::Pointer GetActiveDataStorage() const override;

  /** \brief \sa IDataStorageService::GetActiveDataStorageReference() */
  DataStorageReference GetActiveDataStorageReference() const override;

  /** \brief \sa IDataStorageService::SetActiveDataStorage() */
  bool SetActiveDataStorage(const std::string& label) override;

  /** \brief \sa IDataStorageService::AddDataStorage() */
  bool AddDataStorage(const std::string& label, DataStorage::Pointer storage) override;

  /** \brief \sa IDataStorageService::CreateDataStorage() */
  std::optional<DataStorageReference> CreateDataStorage(const std::string& label) override;

  /** \brief \sa IDataStorageService::GetDataStorageReference() */
  std::optional<DataStorageReference> GetDataStorageReference(const std::string& label) const override;

  /** \brief \sa IDataStorageService::GetLabel() */
  std::optional<std::string> GetLabel(const DataStorage* storage) const override;

  /** \brief \sa IDataStorageService::GetAllLabels() */
  std::vector<std::string> GetAllLabels() const override;

  /** \brief \sa IDataStorageService::GetAllDataStorages() */
  std::vector<DataStorageReference> GetAllDataStorages() const override;

  /** \brief \sa IDataStorageService::HasDataStorage() */
  bool HasDataStorage(const std::string& label) const override;

  /** \brief \sa IDataStorageService::RemoveDataStorage() */
  bool RemoveDataStorage(const std::string& label) override;

  /** \brief \sa IDataStorageService::GetDispatcher() */
  StorageThreadDispatcherBase* GetDispatcher() const override;

  /**
   * \brief Set the thread dispatcher for DataStorage operations.
   *
   * This is typically called by activator code that creates
   * this service, injecting a project-specific dispatcher.
   *
   * \param dispatcher The dispatcher, or nullptr to clear.
   */
  void SetDispatcher(StorageThreadDispatcherBase* dispatcher);

private:
  /** Helper to find reference by name. Is not secured. Assumes that the caller holds m_Mutex.*/
  DataStorageReference FindStorageByLabel(const std::string& label) const;

  mutable std::mutex m_Mutex;
  DataStorageReference m_DefaultStorage;
  std::string m_ActiveLabel; // Empty means use default
  std::vector<DataStorageReference> m_Storages;
  StorageThreadDispatcherBase::Pointer m_Dispatcher;
};

} // namespace mitk

#endif
