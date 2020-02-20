/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSemanticRelationsNodeSelectionDialog.h"

// semantic relations ui module
#include <QmitkAbstractSemanticRelationsStorageInspector.h>

// semantic relations ui module
#include <QmitkPatientTableInspector.h>

QmitkSemanticRelationsNodeSelectionDialog::QmitkSemanticRelationsNodeSelectionDialog(QWidget* parent, QString title, QString hint)
  : QmitkNodeSelectionDialog(parent, title, hint)
{
  // nothing here
}

void QmitkSemanticRelationsNodeSelectionDialog::SetCaseID(const mitk::SemanticTypes::CaseID& caseID)
{
  for (auto panel : m_Panels)
  {
    QmitkAbstractSemanticRelationsStorageInspector* semanticRelationsStorageInspector = dynamic_cast<QmitkAbstractSemanticRelationsStorageInspector*>(panel);
    if (nullptr != semanticRelationsStorageInspector)
    {
      semanticRelationsStorageInspector->SetCaseID(caseID);
    }
  }
}

void QmitkSemanticRelationsNodeSelectionDialog::SetLesion(const mitk::SemanticTypes::Lesion& lesion)
{
  for (auto panel : m_Panels)
  {
    QmitkAbstractSemanticRelationsStorageInspector* semanticRelationsStorageInspector = dynamic_cast<QmitkAbstractSemanticRelationsStorageInspector*>(panel);
    if (nullptr != semanticRelationsStorageInspector)
    {
      semanticRelationsStorageInspector->SetLesion(lesion);
    }
  }
}
