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
#include "QmitkDataStorageFavoriteNodesInspector.h"

void MitkQtWidgetsActivator::Load(us::ModuleContext * /*context*/)
{
  m_TreeInspector.reset(new QmitkDataStorageInspectorProviderBase<QmitkDataStorageListInspector>("org.mitk.QmitkDataStorageListInspector", "Simple list", "Displays the filtered content of the data storage in a simple list."));
  m_ListInspector.reset(new QmitkDataStorageInspectorProviderBase<QmitkDataStorageTreeInspector>("org.mitk.QmitkDataStorageTreeInspector", "Rendering tree", "Displays the filtered content of the data storage as the current rendering tree. \n(Equals the old data manager view)"));
  m_FavoriteNodesInspector.reset(new QmitkDataStorageInspectorProviderBase<QmitkDataStorageFavoriteNodesInspector>("org.mitk.QmitkDataStorageFavoriteNodesInspector", "Favorite nodes list", "Displays the favorite nodes of the data storage in a simple list."));
}

void MitkQtWidgetsActivator::Unload(us::ModuleContext *)
{
}

US_EXPORT_MODULE_ACTIVATOR(MitkQtWidgetsActivator)
