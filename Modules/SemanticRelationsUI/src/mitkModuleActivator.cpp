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
