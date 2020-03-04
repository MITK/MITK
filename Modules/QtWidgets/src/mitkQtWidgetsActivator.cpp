/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkQtWidgetsActivator.h"

// Micro Services
#include <usModuleActivator.h>
#include <usModuleContext.h>

// Qmitk
#include "QmitkDataStorageInspectorProviderBase.h"
#include "QmitkDataStorageListInspector.h"
#include "QmitkDataStorageTreeInspector.h"
#include "QmitkDataStorageSelectionHistoryInspector.h"
#include "QmitkDataStorageFavoriteNodesInspector.h"

void MitkQtWidgetsActivator::Load(us::ModuleContext * /*context*/)
{
  Q_INIT_RESOURCE(Qmitk);
  m_TreeInspector.reset(new QmitkDataStorageInspectorProviderBase<QmitkDataStorageListInspector>("org.mitk.QmitkDataStorageListInspector", "Simple list", "Displays the filtered content of the data storage in a simple list.", ":/Qmitk/list-solid.svg"));
  m_ListInspector.reset(new QmitkDataStorageInspectorProviderBase<QmitkDataStorageTreeInspector>("org.mitk.QmitkDataStorageTreeInspector", "Rendering tree", "Displays the filtered content of the data storage as the current rendering tree. \n(Equals the old data manager view)", ":/Qmitk/tree_inspector.svg"));
  m_HistoryInspector.reset(new QmitkDataStorageInspectorProviderBase<QmitkDataStorageSelectionHistoryInspector>(QmitkDataStorageSelectionHistoryInspector::INSPECTOR_ID(), "Selection history", "Displays the filtered history of all node selections in this application session. \nThe nodes are sorted from new to old selections.\nOnly nodes that are still in the data storage will be displayed.", ":/Qmitk/history-solid.svg"));
  m_FavoriteNodesInspector.reset(new QmitkDataStorageInspectorProviderBase<QmitkDataStorageFavoriteNodesInspector>(QmitkDataStorageFavoriteNodesInspector::INSPECTOR_ID(), "Favorite nodes list", "Displays the favorite nodes of the data storage in a simple list.",":/Qmitk/star-solid.svg"));
}

void MitkQtWidgetsActivator::Unload(us::ModuleContext *)
{
}

US_EXPORT_MODULE_ACTIVATOR(MitkQtWidgetsActivator)
