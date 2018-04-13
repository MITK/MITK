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

#ifndef __QMITK_DATA_STORAGE_INSPECTOR_PROVIDER_H
#define __QMITK_DATA_STORAGE_INSPECTOR_PROVIDER_H

#include <memory>

// Microservices
#include <usGetModuleContext.h>
#include <usServiceProperties.h>
#include <usServiceRegistration.h>

// MITK
#include <QmitkIDataStorageInspectorProvider.h>


/**
 * @brief Base class for DataStorage inspector provider.
 */
template <class TInspector>
class QmitkDataStorageInspectorProviderBase : public mitk::IDataStorageInspectorProvider
{
public:
  virtual QmitkAbstractDataStorageInspector* CreateInspector() const override;

  virtual std::string GetInspectorID() const override;
  virtual std::string GetInspectorDisplayName() const override;
  virtual std::string GetInspectorDescription() const override;

  us::ServiceRegistration<mitk::IDataStorageInspectorProvider> RegisterService(
    us::ModuleContext *context = us::GetModuleContext());

  void UnregisterService();

  QmitkDataStorageInspectorProviderBase(const std::string& id, const std::string& displayName = ID, const std::string& desc= "" );
  virtual ~QmitkDataStorageInspectorProviderBase();

protected:
  QmitkDataStorageInspectorProviderBase(const QmitkDataStorageInspectorProviderBase &other);

  virtual us::ServiceProperties GetServiceProperties() const;

  /**
   * \brief Set the service ranking for this file reader.
   *
   * Default is zero and should only be chosen differently for a reason.
   * The ranking is used to determine which reader to use if several
   * equivalent providers have been found.
   * It may be used to replace a default provider from MITK in your own project.
   */
  void SetRanking(int ranking);
  int GetRanking() const;

private:
  QmitkDataStorageInspectorProviderBase &operator=(const QmitkDataStorageInspectorProviderBase &other);

  class Impl;
  std::unique_ptr<Impl> d;
};

#ifndef ITK_MANUAL_INSTANTIATION
#include "QmitkDataStorageInspectorProviderBase.tpp"
#endif

#endif /* __MODEL_FIT_PROVIDER_BASE_H */
