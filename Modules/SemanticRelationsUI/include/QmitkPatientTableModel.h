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

// semantic relations UI module
#include "QmitkAbstractSemanticRelationsStorageModel.h"

// semantic relations module
#include <mitkSemanticTypes.h>

// mitk core
#include <mitkDataNode.h>

// qt
#include <QPixmap>

/*
* @brief The QmitkPatientTableModel is a subclass of the QmitkAbstractSemanticRelationsStorageModel and holds the semantic relations data of the currently selected case.
*
*   The QmitkPatientTableModel uses the 'data' function to return either the data node of a table cell or the thumbnail of the underlying image.
*   The horizontal header of the table shows the control points of the current case and the vertical header of the table shows the information types of the current case.
*   Using the 'GetFilteredData'-function of the SemanticRelations-class the model is able to retrieve the correct data node for each table entry.
*
*   Additionally the model creates and holds the QPixmaps of the known data nodes in order to return a thumbnail, if needed.
*/
class QmitkPatientTableModel : public QmitkAbstractSemanticRelationsStorageModel
{
  Q_OBJECT 
   
public:

  QmitkPatientTableModel(QObject* parent = nullptr);
  ~QmitkPatientTableModel();

  //////////////////////////////////////////////////////////////////////////
  // overridden functions from QAbstractItemModel
  //////////////////////////////////////////////////////////////////////////
  virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  virtual QModelIndex parent(const QModelIndex &child) const override;

  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
  //////////////////////////////////////////////////////////////////////////
  /// end override
  /////////////////////////////////////////////////////////////////////////

  void SetNodeType(const std::string& nodeType);

protected:

  // the following functions have to be overridden but are not implemented in this model
  virtual void NodePredicateChanged() override;
  virtual void NodeAdded(const mitk::DataNode* node) override;
  virtual void NodeChanged(const mitk::DataNode* node) override;
  virtual void NodeRemoved(const mitk::DataNode* node) override;
  /**
  * @brief Overridden from 'QmitkAbstractSemanticRelationsStorageModel': This function retrieves all control points
  *        and information types of the current case and stores them to define the header of the table.
  *        Furthermore all images are retrieved and the pixmap of the images are generated and stored.
  */
  virtual void SetData() override;

  void SetDataNodes();

private:

  /**
  * @brief The function uses the ID of the node to see if a pixmap was already set. If not, the given pixmap
  *        is used and stored inside a member variable. If the pixmap was already set, it will be overwritten.
  *        Using 'nullptr' as a pixmap will erase the entry for the given data node.
  *
  * @param dataNode           The data node whose pixmap should be set
  * @param  pixmapFromImage   The pixmap that shows an image of the content of the data node
  */
  void SetPixmapOfNode(const mitk::DataNode* dataNode, QPixmap* pixmapFromImage);

  void SetLesionPresence(const mitk::DataNode* dataNode, bool lesionPresence);

  bool IsLesionPresentOnDataNode(const mitk::DataNode* dataNode) const;

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

  std::map<mitk::DataNode::ConstPointer, QPixmap> m_PixmapMap;
  std::map<mitk::DataNode::ConstPointer, bool> m_LesionPresence;

  std::vector<mitk::SemanticTypes::InformationType> m_InformationTypes;
  std::vector<mitk::SemanticTypes::ControlPoint> m_ControlPoints;
  std::string m_SelectedNodeType;

};

#endif // QMITKPATIENTTABLEMODEL_H
