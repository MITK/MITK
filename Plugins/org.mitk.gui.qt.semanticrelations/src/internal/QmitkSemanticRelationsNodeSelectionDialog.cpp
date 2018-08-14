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
