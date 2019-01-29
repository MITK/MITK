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

// semantic relations UI module
#include "QmitkAbstractSemanticRelationsStorageModel.h"

#include "QmitkCustomVariants.h"

QmitkAbstractSemanticRelationsStorageModel::QmitkAbstractSemanticRelationsStorageModel(QObject* parent /*= nullptr*/)
  : QmitkAbstractDataStorageModel(parent)
  , m_SemanticRelationsDataStorageAccess(nullptr)
{
  m_SemanticRelationsIntegration = std::make_unique<mitk::SemanticRelationsIntegration>();
  m_SemanticRelationsIntegration->AddObserver(this);
}

QmitkAbstractSemanticRelationsStorageModel::~QmitkAbstractSemanticRelationsStorageModel()
{
  if (nullptr != m_SemanticRelationsIntegration)
  {
    m_SemanticRelationsIntegration->RemoveObserver(this);
  }
}

void QmitkAbstractSemanticRelationsStorageModel::Update(const mitk::SemanticTypes::CaseID& caseID)
{
  UpdateModelData(caseID);
}

void QmitkAbstractSemanticRelationsStorageModel::SetCaseID(const mitk::SemanticTypes::CaseID& caseID)
{
  m_CaseID = caseID;
  UpdateModelData();
}

void QmitkAbstractSemanticRelationsStorageModel::SetLesion(const mitk::SemanticTypes::Lesion& lesion)
{
  m_Lesion = lesion;
  UpdateModelData();
}

void QmitkAbstractSemanticRelationsStorageModel::SetDataNodeSelection(const QList<mitk::DataNode::Pointer>& dataNodeSelection)
{
  m_SelectedDataNodes = dataNodeSelection;
  UpdateModelData();
}

void QmitkAbstractSemanticRelationsStorageModel::UpdateModelData(const mitk::SemanticTypes::CaseID& caseID)
{
  // if the case ID of updated instance is equal to the currently active caseID
  if (caseID == m_CaseID)
  {
    UpdateModelData();
  }
}

void QmitkAbstractSemanticRelationsStorageModel::UpdateModelData()
{
  if (nullptr == m_SemanticRelationsDataStorageAccess)
  {
    return;
  }

  // update the model, so that the table will be filled with the new patient information
  beginResetModel();

  SetData();

  endResetModel();
  emit ModelUpdated();
}

void QmitkAbstractSemanticRelationsStorageModel::DataStorageChanged()
{
  m_SemanticRelationsDataStorageAccess = std::make_unique<mitk::SemanticRelationsDataStorageAccess>(m_DataStorage.Lock());
  UpdateModelData();
}
