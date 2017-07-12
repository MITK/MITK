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
#include "QmitkSemanticRelationsDataModel.h"

#include "QmitkCustomVariants.h"

// qt
#include <QPixmap>

QmitkSemanticRelationsDataModel::QmitkSemanticRelationsDataModel(QObject* parent /*= nullptr*/)
  : QAbstractTableModel(parent)
  , m_SemanticRelations(nullptr)
  , m_DataStorage(nullptr)
{
  // nothing here
}

QmitkSemanticRelationsDataModel::~QmitkSemanticRelationsDataModel()
{
  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->ChangedNodeEvent.RemoveListener(mitk::MessageDelegate1<QmitkSemanticRelationsDataModel, const mitk::DataNode*>(this, &QmitkSemanticRelationsDataModel::DataChanged));
    m_DataStorage->RemoveNodeEvent.RemoveListener(mitk::MessageDelegate1<QmitkSemanticRelationsDataModel, const mitk::DataNode*>(this, &QmitkSemanticRelationsDataModel::DataChanged));
  }
}

Qt::ItemFlags QmitkSemanticRelationsDataModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags;
  mitk::DataNode* dataNode = GetCurrentDataNode(index);
  if (nullptr != dataNode)
  {
    flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }

  return flags;
}

int QmitkSemanticRelationsDataModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return m_InformationTypes.size();
}

int QmitkSemanticRelationsDataModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return m_ControlPoints.size();
}

QVariant QmitkSemanticRelationsDataModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }

  mitk::DataNode* dataNode = GetCurrentDataNode(index);
  if (nullptr == dataNode)
  {
    return QVariant();
  }

  if (Qt::ToolTipRole == role)
  {
    // TODO: add tool tip
    return QVariant();
  }
  else if (Qt::UserRole == role)
  {
    // user role always returns a reference to the data node,
    // which can be used to modify the data node in the data storage
    return QVariant::fromValue(dataNode);
  }
  else if (Qt::DecorationRole == role)
  {
    auto it = m_PixmapMap.find(DICOMHelper::GetIDFromData(dataNode));
    if (it != m_PixmapMap.end())
    {
      return QVariant(it->second);
    }
  }
  return QVariant();
}

QVariant QmitkSemanticRelationsDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (Qt::Horizontal == orientation && Qt::DisplayRole == role)
  {
    if (m_ControlPoints.size() > section)
    {
      SemanticTypes::ControlPoint currentControlPoint = m_ControlPoints.at(section);
      // generate a string from the control point
      std::string currentControlPointAsString = currentControlPoint.AsString();
      return QVariant(QString::fromStdString(currentControlPointAsString));
    }
  }
  if (Qt::Vertical == orientation && Qt::DisplayRole == role)
  {
    if (m_InformationTypes.size() > section)
    {
      SemanticTypes::InformationType currentInformationType = m_InformationTypes.at(section);
      return QVariant(QString::fromStdString(currentInformationType));
    }
  }
  return QVariant();
}

void QmitkSemanticRelationsDataModel::SetSemanticRelations(mitk::SemanticRelations* semanticRelations)
{
  m_SemanticRelations = semanticRelations;
}

// TODO: listener needed?
void QmitkSemanticRelationsDataModel::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    // given data storage is a new data storage
    if (m_DataStorage.IsNotNull())
    {
      // remove listener from old data storage
      m_DataStorage->ChangedNodeEvent.RemoveListener(mitk::MessageDelegate1<QmitkSemanticRelationsDataModel, const mitk::DataNode*>(this, &QmitkSemanticRelationsDataModel::DataChanged));
      m_DataStorage->RemoveNodeEvent.RemoveListener(mitk::MessageDelegate1<QmitkSemanticRelationsDataModel, const mitk::DataNode*>(this, &QmitkSemanticRelationsDataModel::DataChanged));
    }
    // set the new data storage
    m_DataStorage = dataStorage;
    // register new data storage listener
    if (m_DataStorage.IsNotNull())
    {
      m_DataStorage->ChangedNodeEvent.AddListener(mitk::MessageDelegate1<QmitkSemanticRelationsDataModel, const mitk::DataNode*>(this, &QmitkSemanticRelationsDataModel::DataChanged));
      m_DataStorage->RemoveNodeEvent.AddListener(mitk::MessageDelegate1<QmitkSemanticRelationsDataModel, const mitk::DataNode*>(this, &QmitkSemanticRelationsDataModel::DataChanged));
    }
    DataChanged();
  }
}

void QmitkSemanticRelationsDataModel::SetCurrentCaseID(const SemanticTypes::CaseID& caseID)
{
  m_CaseID = caseID;
  DataChanged();
}

void QmitkSemanticRelationsDataModel::SetPixmapOfNode(const mitk::DataNode* dataNode, const QPixmap& pixmapFromImage)
{
  SemanticTypes::ID nodeID = DICOMHelper::GetIDFromData(dataNode);
  m_PixmapMap.insert(std::make_pair(nodeID, pixmapFromImage.scaled(120, 120, Qt::IgnoreAspectRatio)));
}

void QmitkSemanticRelationsDataModel::DataChanged(const mitk::DataNode* dataNode)
{
  if (nullptr == m_SemanticRelations || nullptr == m_DataStorage)
  {
    return;
  }

  if (nullptr != dataNode)
  {
    // if the function is called with a data node, check if this is a helper object
    bool helperObject = false;
    dataNode->GetBoolProperty("helper object", helperObject);
    if (helperObject)
    {
      return;
    }
  }
  // function not called with a data node (nullptr)

  // update the model, so that the table will be filled with the new patient information
  beginResetModel();

  // update current data
  // get all control points of current case
  m_ControlPoints = m_SemanticRelations->GetAllControlPointsOfCase(m_CaseID);
  // sort the vector of control points for the timeline
  std::sort(m_ControlPoints.begin(), m_ControlPoints.end());
  // get all information types points of current case
  m_InformationTypes = m_SemanticRelations->GetAllInformationTypesOfCase(m_CaseID);

  endResetModel();
  emit ModelReset();
}

mitk::DataNode* QmitkSemanticRelationsDataModel::GetCurrentDataNode(const QModelIndex& index) const
{
  SemanticTypes::ControlPoint currentControlPoint = m_ControlPoints.at(index.column());
  SemanticTypes::InformationType currentInformationType = m_InformationTypes.at(index.row());
  std::vector<mitk::DataNode::Pointer> filteredDataNodes = m_SemanticRelations->GetFilteredData(m_CaseID, currentControlPoint, currentInformationType);
  if (filteredDataNodes.empty())
  {
    return nullptr;
  }
  return filteredDataNodes.front();
}