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

#ifndef QMITKABSTRACTSEMANTICRELATIONSSTORAGEMODEL_H
#define QMITKABSTRACTSEMANTICRELATIONSSTORAGEMODEL_H

// mitk semantic relations UI
#include "MitkSemanticRelationsUIExports.h"

// semantic relations module
#include <mitkISemanticRelationsObserver.h>
#include <mitkSemanticRelationsDataStorageAccess.h>
#include <mitkSemanticRelationsIntegration.h>
#include <mitkSemanticTypes.h>

// qt widgets module
#include "QmitkAbstractDataStorageModel.h"

/*
* @brief The QmitkAbstractSemanticRelationsStorageModel is a subclass of 'QmitkAbstractDataStorageModel' and provides additional
*        functionality to set and store a semantic relations instance, the current case ID and the current lesion.
*/
class MITKSEMANTICRELATIONSUI_EXPORT QmitkAbstractSemanticRelationsStorageModel : public QmitkAbstractDataStorageModel, public mitk::ISemanticRelationsObserver
{
  Q_OBJECT

public:

  QmitkAbstractSemanticRelationsStorageModel(QObject* parent = nullptr);
  virtual ~QmitkAbstractSemanticRelationsStorageModel();

  /*
  * @brief Update this model with the data from the semantic relations.
  *
  *       Overridden from 'ISemanticRelationsObserver'.
  *       In order for the Update-function to be called, this model has to be added as a observer of SemanticRelation
  *       (e.g. m_SemanticRelations->AddObserver(m_SemanticRelationsStorageModel);)
  *
  * @par caseID    The current case ID to identify the currently active patient / case.
  */
  virtual void Update(const mitk::SemanticTypes::CaseID& caseID) override;

  /**
  * @brief Set the current case ID which is needed to access the semantic relations storage.
  *
  * @param caseID    A case ID as string
  */
  void SetCaseID(const mitk::SemanticTypes::CaseID& caseID);

  const mitk::SemanticTypes::CaseID& GetCaseID() const { return m_CaseID; }
  /**
  * @brief Set the current lesion which can be used to show on which images the lesion is visible.
  *
  * @param lesion   The selected lesion
  */
  void SetLesion(const mitk::SemanticTypes::Lesion& lesion);

  const mitk::SemanticTypes::Lesion& GetLesion() const { return m_Lesion; }
  /**
  * @brief Set the current data node selection which can be used to show which lesions
  *        are visible on the node selection.
  *
  * @param dataNodeSelection    The selected data nodes
  */
  void SetDataNodeSelection(const QList<mitk::DataNode::Pointer>& dataNodeSelection);

  const QList<mitk::DataNode::Pointer>& GetSelectedDataNodes() const { return m_SelectedDataNodes; };
  /*
  * @brief Update the semantic relations storage model with the current data from the semantic relations model,
  *        if the case ID is equal to the currently selected case ID of the table model.
  */
  void UpdateModelData(const mitk::SemanticTypes::CaseID& caseID);
  /*
  * @brief Update the semantic relations storage model with the current data from the semantic relations model
  *        and the current case ID.
  */
  void UpdateModelData();

Q_SIGNALS:
  void ModelUpdated();

protected:

  /**
  * @brief Create a new 'SemanticRelationsDataStorageAccess' instance with the new data storage and
  *   update the model data.
  *   This functions is called inside the 'SetDataStorage'-function from the parent class.
  */
  virtual void DataStorageChanged() override;

  /**
  * @brief This function is called if the model data is updated. It can be used by subclasses to define
  *        the way the data of a specific model is generated. It typically consists of access to the
  *        semantic relations storage to retrieve certain information.
  */
  virtual void SetData() = 0;

  std::unique_ptr<mitk::SemanticRelationsDataStorageAccess> m_SemanticRelationsDataStorageAccess;
  std::unique_ptr<mitk::SemanticRelationsIntegration> m_SemanticRelationsIntegration;

  mitk::SemanticTypes::CaseID m_CaseID;
  QList<mitk::DataNode::Pointer> m_SelectedDataNodes;
  mitk::SemanticTypes::Lesion m_Lesion;
};

#endif // QMITKABSTRACTSEMANTICRELATIONSSTORAGEMODEL_H
