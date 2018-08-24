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

// semantic relations module
#include <mitkISemanticRelationsObserver.h>
#include <mitkSemanticRelations.h>
#include <mitkSemanticTypes.h>

// qt widgets module
#include "QmitkAbstractDataStorageModel.h"

/*
* @brief The QmitkAbstractSemanticRelationsStorageModel is a subclass of 'QmitkAbstractDataStorageModel' and provides additional
*        functionality to set and store a semantic relations instance, the current case ID and the current lesion.
*/
class QmitkAbstractSemanticRelationsStorageModel : public QmitkAbstractDataStorageModel, public mitk::ISemanticRelationsObserver
{
  Q_OBJECT

public:

  QmitkAbstractSemanticRelationsStorageModel(QObject* parent = nullptr);
  virtual ~QmitkAbstractSemanticRelationsStorageModel();

  /*
  * @brief Updates this model with the data from the semantic relations.
  *
  *       Overridden from 'ISemanticRelationsObserver'.
  *       In order for the Update-function to be called, this model has to be added as a observer of SemanticRelation
  *       (e.g. m_SemanticRelations->AddObserver(m_SemanticRelationsStorageModel);)
  *
  * @par caseID    The current case ID to identify the currently active patient / case.
  */
  virtual void Update(const mitk::SemanticTypes::CaseID& caseID) override;

  std::shared_ptr<mitk::SemanticRelations> GetSemanticRelations() const { return m_SemanticRelations; }
  /**
  * @brief Sets the current case ID which is needed to access the semantic relations storage.
  *
  * @param caseID    A case ID as string
  */
  void SetCaseID(const mitk::SemanticTypes::CaseID& caseID);

  const mitk::SemanticTypes::CaseID& GetCaseID() const { return m_CaseID; }
  /**
  * @brief Sets the current lesion which can be used to show on which images the lesion is visible.
  *
  * @param lesion   The selected lesion
  */
  void SetLesion(const mitk::SemanticTypes::Lesion& lesion);

  const mitk::SemanticTypes::Lesion& GetLesion() const { return m_Lesion; }

  /*
  * @brief Updates the semantic relations storage model with the current data from the semantic relations model,
  *        if the case ID is equal to the currently selected case ID of the table model.
  */
  void UpdateModelData(const mitk::SemanticTypes::CaseID& caseID);
  /*
  * @brief Updates the semantic relations storage model with the current data from the semantic relations model
  *        and the current case ID.
  */
  void UpdateModelData();

Q_SIGNALS:
  void ModelUpdated();

protected:

  /**
  * @brief Creates a new 'SemanticRelations' instance with the new data storage and updates the model data.
  *   This functions is called inside the 'SetDataStorage'-function from the parent class.
  */
  virtual void DataStorageChanged() override;

  /**
  * @brief This function is called if the model data is updated. It can be used by subclasses to define
  *        the way the data of a specific model is generated. It typically consists of access to the
  *        semantic relations storage to retrieve certain information.
  */
  virtual void SetData() = 0;

  std::shared_ptr<mitk::SemanticRelations> m_SemanticRelations;
  mitk::SemanticTypes::CaseID m_CaseID;
  mitk::SemanticTypes::Lesion m_Lesion;
};

#endif // QMITKABSTRACTSEMANTICRELATIONSSTORAGEMODEL_H
