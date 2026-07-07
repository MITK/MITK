/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkQtWidgetsActivator_h
#define mitkQtWidgetsActivator_h

// Micro Services
#include <usModuleActivator.h>
#include <usModuleEvent.h>
#include <usServiceRegistration.h>
#include <usServiceTracker.h>

#include <memory>

#include <mitkIDataStorageInspectorProvider.h>

/**
 * \brief Module activator for the QtWidgets module.
 *
 * Registers data storage inspector providers (tree, list, history, favorites)
 * as micro-services when the module is loaded.
 */
class MitkQtWidgetsActivator : public us::ModuleActivator
{
public:
  /** \brief Register data storage inspector providers. */
  void Load(us::ModuleContext *context) override;

  /** \brief Unregister all data storage inspector providers. */
  void Unload(us::ModuleContext *) override;

private:
  std::unique_ptr<mitk::IDataStorageInspectorProvider> m_TreeInspector;
  std::unique_ptr<mitk::IDataStorageInspectorProvider> m_ListInspector;
  std::unique_ptr<mitk::IDataStorageInspectorProvider> m_HistoryInspector;
  std::unique_ptr<mitk::IDataStorageInspectorProvider> m_FavoriteNodesInspector;
};

#endif
