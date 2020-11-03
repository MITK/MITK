/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkModuleActivator.h"

// semantic relations ui module
#include "QmitkPatientTableInspector.h"

// qt widgets module
#include "QmitkDataStorageInspectorProviderBase.h"

void mitk::SemanticRelationsUIActivator::Load(us::ModuleContext* /*context*/)
{
  m_PatientTableInspector.reset(new QmitkDataStorageInspectorProviderBase<QmitkPatientTableInspector>("org.mitk.QmitkPatientTableInspector",
                                "Patient table inspector", "Displays the content of the semantic relations storage in a control-point - modality table."));
 }

void mitk::SemanticRelationsUIActivator::Unload(us::ModuleContext* /*context*/)
{
}

US_EXPORT_MODULE_ACTIVATOR(mitk::SemanticRelationsUIActivator)
