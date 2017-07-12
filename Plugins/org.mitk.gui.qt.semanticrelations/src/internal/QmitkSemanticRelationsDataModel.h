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

#ifndef QMITKSEMANTICRELATIONSDATAMODEL_H
#define QMITKSEMANTICRELATIONSDATAMODEL_H

// semantic relations module
#include <mitkSemanticRelations.h>
#include <mitkSemanticTypes.h>

// qt
#include <QAbstractTableModel>

// mitk core
#include <mitkDataNode.h>

class QmitkSemanticRelationsDataModel : public QAbstractTableModel
{
  Q_OBJECT 
   
public:

  QmitkSemanticRelationsDataModel(QObject* parent = nullptr);
  ~QmitkSemanticRelationsDataModel();

  //////////////////////////////////////////////////////////////////////////
  // overridden functions from QAbstractItemModel
  //////////////////////////////////////////////////////////////////////////
  virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual QVariant data(const QModelIndex &index, int role) const override;
  //virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  //////////////////////////////////////////////////////////////////////////
  /// end override
  /////////////////////////////////////////////////////////////////////////

  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);
  void SetSemanticRelations(mitk::SemanticRelations* semanticRelations);
  const SemanticTypes::CaseID& GetCurrentCaseID() { return m_CaseID; }
  void SetCurrentCaseID(const SemanticTypes::CaseID& caseID);

  void SetPixmapOfNode(const mitk::DataNode* dataNode, const QPixmap& pixmapFromImage);
  void DataChanged(const mitk::DataNode* dataNode = nullptr);
  mitk::DataNode* GetCurrentDataNode(const QModelIndex &index) const;

Q_SIGNALS:
  void ModelReset();

private:

  mitk::SemanticRelations* m_SemanticRelations;
  mitk::DataStorage::Pointer m_DataStorage;
  SemanticTypes::CaseID m_CaseID;

  std::map<std::string, QPixmap> m_PixmapMap;
  std::vector<SemanticTypes::InformationType> m_InformationTypes;
  std::vector<SemanticTypes::ControlPoint> m_ControlPoints;

};

#endif // QMITKSEMANTICRELATIONSDATAMODEL_H