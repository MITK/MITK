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

#ifndef QMITKPATIENTTABLEMODEL_H
#define QMITKPATIENTTABLEMODEL_H

// semantic relations module
#include <mitkSemanticRelations.h>
#include <mitkSemanticTypes.h>

// qt
#include <QAbstractTableModel>

// mitk core
#include <mitkDataNode.h>

/*
* @brief The QmitkPatientTableModel is the model for the 'QmitkPatientTableWidget' and holds the control point data and information type data of the currently selected case.
*
*   The QmitkPatientTableModel uses the 'data' function to return either the data node of a table cell or the thumbnail of the underlying image.
*   The horizontal header of the table shows the control points of the current case and the vertical header of the table shows the information types of the current case.
*   Using the 'GetFilteredData'-function of the SemanticRelations-class the model is able to retrieve the correct data node for each table entry.
*
*   Additionally the QmitkPatientTableWidget holds the QPixmaps of the known data nodes in order to return a thumbnail, if needed.
*
*   If the 'QmitkPatientTableWidget' is updated, the 'SetCurrentCaseID'-function is called, which leads to a fresh retrieval of the control point data
*   and the information type data.
*/
class QmitkPatientTableModel : public QAbstractTableModel
{
  Q_OBJECT 
   
public:

  QmitkPatientTableModel(std::shared_ptr<mitk::SemanticRelations> semanticRelations, QObject* parent = nullptr);
  ~QmitkPatientTableModel();

  //////////////////////////////////////////////////////////////////////////
  // overridden functions from QAbstractItemModel
  //////////////////////////////////////////////////////////////////////////
  virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  //virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  //////////////////////////////////////////////////////////////////////////
  /// end override
  /////////////////////////////////////////////////////////////////////////

  const mitk::SemanticTypes::CaseID& GetCurrentCaseID() const { return m_CaseID; }
  void SetCurrentCaseID(const mitk::SemanticTypes::CaseID& caseID);
  void SetPixmapOfNode(const mitk::DataNode* dataNode, QPixmap* pixmapFromImage);
  void SetPatientData();
  /*
  * @brief Updates the table model with the current control point data and information type data from the semantic relations model, 
  *        if the 'caseID' is equal to the currently selected case ID of the table model.
  *        This function can be called from an observer of SemanticRelations (e.g. a table view that uses this model)
  *        in order to propagate the Update-request to this model.
  */
  void Update(const mitk::SemanticTypes::CaseID& caseID);

Q_SIGNALS:
  void ModelUpdated();

private:

  /*
  * @brief Returns the data node that is associated with the given table entry (index).
  *
  *         The function uses the SemanticRelations-class and the current control point data and information type data to
  *         filter the nodes of the current case.
  *         The index is used to access the correct row in the table (information type) and the correct column in the table (control point).
  *
  * @par index    The QModelIndex of the table entry
  */
  mitk::DataNode* GetCurrentDataNode(const QModelIndex &index) const;

  mitk::DataStorage::Pointer m_DataStorage;
  std::shared_ptr<mitk::SemanticRelations> m_SemanticRelations;
  mitk::SemanticTypes::CaseID m_CaseID;

  std::map<std::string, QPixmap> m_PixmapMap;
  std::vector<mitk::SemanticTypes::InformationType> m_InformationTypes;
  std::vector<mitk::SemanticTypes::ControlPoint> m_ControlPoints;

};

#endif // QMITKPATIENTTABLEMODEL_H
