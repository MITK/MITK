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

// semantic relations plugin
#include "QmitkPatientTableModel.h"

#include "QmitkCustomVariants.h"

QmitkSemanticRelationsModel::QmitkSemanticRelationsModel(QObject* parent /*= nullptr*/)
  : QmitkAbstractDataStorageModel(parent)
  , m_SemanticRelations(nullptr)
{
  // nothing here
}

QmitkSemanticRelationsModel::~QmitkSemanticRelationsModel()
{
  // nothing here
}

void QmitkSemanticRelationsModel::SetCaseID(const mitk::SemanticTypes::CaseID& caseID)
{
  m_CaseID = caseID;
  UpdateModelData();
}

void QmitkSemanticRelationsModel::UpdateModelData(const mitk::SemanticTypes::CaseID& caseID)
{
  // if the case ID of updated instance is equal to the currently active caseID
  if (caseID == m_CaseID)
  {
    UpdateModelData();
  }
}

void QmitkSemanticRelationsModel::UpdateModelData()
{
  if (nullptr == m_SemanticRelations)
  {
    return;
  }

  // update the model, so that the table will be filled with the new patient information
  beginResetModel();

  SetData();

  endResetModel();
  emit ModelUpdated();
}

void QmitkSemanticRelationsModel::DataStorageChanged()
{
  m_SemanticRelations = std::make_shared<mitk::SemanticRelations>(m_DataStorage.Lock());
  UpdateModelData();
}
