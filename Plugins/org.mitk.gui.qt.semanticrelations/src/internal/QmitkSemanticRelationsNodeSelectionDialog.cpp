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

#include "QmitkSemanticRelationsNodeSelectionDialog.h"

#include <mitkDataStorageInspectorGenerator.h>
#include <QmitkDataStorageTreeInspector.h>
#include <QmitkNodeSelectionPreferenceHelper.h>

QmitkSemanticRelationsNodeSelectionDialog::QmitkSemanticRelationsNodeSelectionDialog(QWidget* parent, QString title, QString hint)
  : QmitkNodeSelectionDialog(parent, title, hint)
{
  // nothing here
}

void QmitkSemanticRelationsNodeSelectionDialog::SetCaseID(const mitk::SemanticTypes::CaseID& caseID)
{
  for (auto panel : m_Panels)
  {
    QmitkPatientTableInspector* patientTableInspector = dynamic_cast<QmitkPatientTableInspector*>(panel);
    if (nullptr != patientTableInspector)
    {
      patientTableInspector->SetCaseID(caseID);
    }
  }
}
