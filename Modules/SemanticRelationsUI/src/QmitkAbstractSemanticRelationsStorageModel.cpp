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
  , m_SemanticRelations(nullptr)
{
  // nothing here
}

QmitkAbstractSemanticRelationsStorageModel::~QmitkAbstractSemanticRelationsStorageModel()
{
  if (nullptr != m_SemanticRelations)
  {
    m_SemanticRelations->RemoveObserver(this);
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

void QmitkAbstractSemanticRelationsStorageModel::DataStorageChanged()
{
  if (nullptr != m_SemanticRelations)
  {
    m_SemanticRelations->RemoveObserver(this);
  }

  m_SemanticRelations = std::make_shared<mitk::SemanticRelations>(m_DataStorage.Lock());
  m_SemanticRelations->AddObserver(this);
  UpdateModelData();
}
