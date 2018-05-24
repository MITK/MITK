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

#include "mitkQtWidgetsActivator.h"

// Micro Services
#include <usModuleActivator.h>
#include <usModuleContext.h>

// Qmitk
#include "QmitkDataStorageInspectorProviderBase.h"
#include "QmitkDataStorageListInspector.h"
#include "QmitkDataStorageTreeInspector.h"

void MitkQtWidgetsActivator::Load(us::ModuleContext * /*context*/)
{
  m_TreeInspector.reset(new QmitkDataStorageInspectorProviderBase<QmitkDataStorageListInspector>("org.mitk.QmitkDataStorageListInspector", "Simple list", "Displays the filtered content of the data storage in a simple list."));
  m_ListInspector.reset(new QmitkDataStorageInspectorProviderBase<QmitkDataStorageTreeInspector>("org.mitk.QmitkDataStorageTreeInspector", "Rendering tree", "Displays the filtered content of the data storage as the current rendering tree. \n(Equals the old data manager view)"));
}

void MitkQtWidgetsActivator::Unload(us::ModuleContext *)
{
}

US_EXPORT_MODULE_ACTIVATOR(MitkQtWidgetsActivator)