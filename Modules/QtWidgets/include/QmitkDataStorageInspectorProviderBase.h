/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __QMITK_DATA_STORAGE_INSPECTOR_PROVIDER_BASE_H
#define __QMITK_DATA_STORAGE_INSPECTOR_PROVIDER_BASE_H

#include <memory>

// Microservices
#include <usGetModuleContext.h>
#include <usServiceProperties.h>
#include <usServiceRegistration.h>

// MITK
#include <mitkIDataStorageInspectorProvider.h>


/**
 * @brief Base class for DataStorage inspector provider.
 */
template <class TInspector>
class QmitkDataStorageInspectorProviderBase : public mitk::IDataStorageInspectorProvider
{
public:
  QmitkAbstractDataStorageInspector* CreateInspector() const override;

  std::string GetInspectorID() const override;
  std::string GetInspectorDisplayName() const override;
  std::string GetInspectorDescription() const override;

  us::ServiceRegistration<mitk::IDataStorageInspectorProvider> RegisterService(
    us::ModuleContext *context = us::GetModuleContext());

  void UnregisterService();

  QmitkDataStorageInspectorProviderBase(const std::string& id);
  QmitkDataStorageInspectorProviderBase(const std::string& id, const std::string& displayName, const std::string& desc= "" );
  ~QmitkDataStorageInspectorProviderBase() override;

protected:
  QmitkDataStorageInspectorProviderBase(const QmitkDataStorageInspectorProviderBase &other);
  QmitkDataStorageInspectorProviderBase &operator=(const QmitkDataStorageInspectorProviderBase &other) = delete;

  virtual us::ServiceProperties GetServiceProperties() const;

  /**
   * \brief Set the service ranking for this file reader.
   *
   * Default is zero and should only be chosen differently for a reason.
   * The ranking is used to determine which provider to use if several
   * equivalent providers have been found.
   * It may be used to replace a default provider from MITK in your own project.
   */
  void SetRanking(int ranking);
  int GetRanking() const;

private:
  class Impl;
  std::unique_ptr<Impl> d;
};

#ifndef ITK_MANUAL_INSTANTIATION
#include "QmitkDataStorageInspectorProviderBase.tpp"
#endif

#endif /* __QMITK_DATA_STORAGE_INSPECTOR_PROVIDER_BASE_H */
