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

// qt
#include <QPixmap>

QmitkPatientTableModel::QmitkPatientTableModel(std::shared_ptr<mitk::SemanticRelations> semanticRelations, QObject* parent /*= nullptr*/)
  : QAbstractTableModel(parent)
  , m_SemanticRelations(semanticRelations)
{
  // nothing here
}

QmitkPatientTableModel::~QmitkPatientTableModel()
{
  // nothing here
}

Qt::ItemFlags QmitkPatientTableModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags;
  mitk::DataNode* dataNode = GetCurrentDataNode(index);
  if (nullptr != dataNode)
  {
    flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }

  return flags;
}

int QmitkPatientTableModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return m_InformationTypes.size();
}

int QmitkPatientTableModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return m_ControlPoints.size();
}

QVariant QmitkPatientTableModel::data(const QModelIndex &index, int role /*=Qt::DisplayRole*/) const
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

  if (Qt::UserRole == role)
  {
    // user role always returns a reference to the data node,
    // which can be used to modify the data node in the data storage
    return QVariant::fromValue(dataNode);
  }
  else if (Qt::DecorationRole == role)
  {
    auto it = m_PixmapMap.find(mitk::GetIDFromDataNode(dataNode));
    if (it != m_PixmapMap.end())
    {
      return QVariant(it->second);
    }
  }
  return QVariant();
}

QVariant QmitkPatientTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (Qt::Horizontal == orientation && Qt::DisplayRole == role)
  {
    if (m_ControlPoints.size() > section)
    {
      mitk::SemanticTypes::ControlPoint currentControlPoint = m_ControlPoints.at(section);
      // generate a string from the control point
      std::string currentControlPointAsString = mitk::GetControlPointAsString(currentControlPoint);
      return QVariant(QString::fromStdString(currentControlPointAsString));
    }
  }
  if (Qt::Vertical == orientation && Qt::DisplayRole == role)
  {
    if (m_InformationTypes.size() > section)
    {
      mitk::SemanticTypes::InformationType currentInformationType = m_InformationTypes.at(section);
      return QVariant(QString::fromStdString(currentInformationType));
    }
  }
  return QVariant();
}

void QmitkPatientTableModel::SetCurrentCaseID(const mitk::SemanticTypes::CaseID& caseID)
{
  m_CaseID = caseID;
  SetPatientData();
}

void QmitkPatientTableModel::SetPixmapOfNode(const mitk::DataNode* dataNode, QPixmap* pixmapFromImage)
{
  mitk::SemanticTypes::ID nodeID = mitk::GetIDFromDataNode(dataNode);
  std::map<std::string, QPixmap>::iterator iter = m_PixmapMap.find(nodeID);
  if (iter != m_PixmapMap.end())
  {
    // key already existing
    if (nullptr != pixmapFromImage)
    {
      // overwrite already stored pixmap
      iter->second = pixmapFromImage->scaled(120, 120, Qt::IgnoreAspectRatio);
    }
    else
    {
      // remove key if no pixmap is given
      m_PixmapMap.erase(iter);
    }
  }
  else
  {
    m_PixmapMap.insert(std::make_pair(nodeID, pixmapFromImage->scaled(120, 120, Qt::IgnoreAspectRatio)));
  }
}

void QmitkPatientTableModel::SetPatientData()
{
  if (nullptr == m_SemanticRelations)
  {
    return;
  }

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
  emit ModelUpdated();
}

void QmitkPatientTableModel::Update(const mitk::SemanticTypes::CaseID& caseID)
{
  // if the case ID of updated instance is equal to the currently active caseID
  if (caseID == m_CaseID)
  {
    SetPatientData();
  }
}

mitk::DataNode* QmitkPatientTableModel::GetCurrentDataNode(const QModelIndex& index) const
{
  mitk::SemanticTypes::ControlPoint currentControlPoint = m_ControlPoints.at(index.column());
  mitk::SemanticTypes::InformationType currentInformationType = m_InformationTypes.at(index.row());
  std::vector<mitk::DataNode::Pointer> filteredDataNodes = m_SemanticRelations->GetFilteredData(m_CaseID, currentControlPoint, currentInformationType);
  if (filteredDataNodes.empty())
  {
    return nullptr;
  }
  return filteredDataNodes.front();
}
