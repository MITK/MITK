/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKQTWIDGETSACTIVATOR_H_
#define MITKQTWIDGETSACTIVATOR_H_

// Micro Services
#include <usModuleActivator.h>
#include <usModuleEvent.h>
#include <usServiceRegistration.h>
#include <usServiceTracker.h>

#include <memory>

#include "mitkIDataStorageInspectorProvider.h"

/*
 * This is the module activator for the "QtWidgets" module.
 */
class MitkQtWidgetsActivator : public us::ModuleActivator
{
public:
  void Load(us::ModuleContext *context) override;
  void Unload(us::ModuleContext *) override;

private:
  std::unique_ptr<mitk::IDataStorageInspectorProvider> m_TreeInspector;
  std::unique_ptr<mitk::IDataStorageInspectorProvider> m_ListInspector;
  std::unique_ptr<mitk::IDataStorageInspectorProvider> m_HistoryInspector;
  std::unique_ptr<mitk::IDataStorageInspectorProvider> m_FavoriteNodesInspector;
};

#endif // MITKCOREACTIVATOR_H_
