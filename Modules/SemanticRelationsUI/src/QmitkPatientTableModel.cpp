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
#include "QmitkPatientTableModel.h"
#include "QmitkPatientTableHeaderView.h"
#include "QmitkSemanticRelationsUIHelper.h"

// semantic relations module
#include <mitkControlPointManager.h>
#include <mitkNodePredicates.h>
#include <mitkSemanticRelationException.h>
#include <mitkSemanticRelationsInference.h>
#include <mitkRelationStorage.h>

#include <QmitkCustomVariants.h>
#include <QmitkEnums.h>

// qt
#include <QColor>

// c++
#include <iostream>
#include <string>

QmitkPatientTableModel::QmitkPatientTableModel(QObject* parent /*= nullptr*/)
  : QmitkAbstractSemanticRelationsStorageModel(parent)
  , m_SelectedNodeType("Image")
{
  m_HeaderModel = new QStandardItemModel(this);
}

QmitkPatientTableModel::~QmitkPatientTableModel()
{
  // nothing here
}

QModelIndex QmitkPatientTableModel::index(int row, int column, const QModelIndex& parent/* = QModelIndex()*/) const
{
  if (hasIndex(row, column, parent))
  {
    return createIndex(row, column);
  }

  return QModelIndex();
}

QModelIndex QmitkPatientTableModel::parent(const QModelIndex& /*child*/) const
{
  return QModelIndex();
}

int QmitkPatientTableModel::rowCount(const QModelIndex& parent/* = QModelIndex()*/) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return m_InformationTypes.size();
}

int QmitkPatientTableModel::columnCount(const QModelIndex& parent/* = QModelIndex()*/) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return m_ControlPoints.size();
}

QVariant QmitkPatientTableModel::data(const QModelIndex& index, int role/* = Qt::DisplayRole*/) const
{
  // special role for returning the horizontal header
  if (QmitkPatientTableHeaderView::HorizontalHeaderDataRole == role)
  {
    return QVariant::fromValue<QStandardItemModel*>(m_HeaderModel);
  }

  if (!index.isValid())
  {
    return QVariant();
  }

  if (index.row() < 0 || index.row() >= static_cast<int>(m_InformationTypes.size())
   || index.column() < 0 || index.column() >= static_cast<int>(m_ControlPoints.size()))
  {
    return QVariant();
  }

  mitk::DataNode* dataNode = GetCurrentDataNode(index);
  if (nullptr == dataNode)
  {
    return QVariant();
  }

  if (Qt::DecorationRole == role)
  {
    auto it = m_PixmapMap.find(dataNode);
    if (it != m_PixmapMap.end())
    {
      return QVariant(it->second);
    }
  }

  if (QmitkDataNodeRole == role)
  {
    return QVariant::fromValue<mitk::DataNode::Pointer>(mitk::DataNode::Pointer(dataNode));
  }

  if (QmitkDataNodeRawPointerRole == role)
  {
    return QVariant::fromValue<mitk::DataNode*>(dataNode);
  }

  if (Qt::BackgroundColorRole == role)
  {
    auto it = m_LesionPresence.find(dataNode);
    if (it != m_LesionPresence.end())
    {
      return it->second ? QVariant(QColor(Qt::darkGreen)) : QVariant(QColor(Qt::transparent));
    }

    return QVariant(QColor(Qt::transparent));
  }

  return QVariant();
}

QVariant QmitkPatientTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (Qt::Vertical == orientation && Qt::DisplayRole == role)
  {
    if (static_cast<int>(m_InformationTypes.size()) > section)
    {
      mitk::SemanticTypes::InformationType currentInformationType = m_InformationTypes.at(section);
      return QVariant(QString::fromStdString(currentInformationType));
    }
  }

  return QVariant();
}

Qt::ItemFlags QmitkPatientTableModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags;
  mitk::DataNode* dataNode = GetCurrentDataNode(index);
  if (nullptr != dataNode)
  {
    flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }

  return flags;
}

void QmitkPatientTableModel::SetNodeType(const std::string& nodeType)
{
  m_SelectedNodeType = nodeType;
  UpdateModelData();
}

void QmitkPatientTableModel::NodePredicateChanged()
{
  UpdateModelData();
}

void QmitkPatientTableModel::SetData()
{
  // get all control points of current case
  m_ControlPoints = mitk::RelationStorage::GetAllControlPointsOfCase(m_CaseID);
  // sort the vector of control points for the timeline
  std::sort(m_ControlPoints.begin(), m_ControlPoints.end());

  // get all examination periods of current case
  m_ExaminationPeriods = mitk::RelationStorage::GetAllExaminationPeriodsOfCase(m_CaseID);
  // sort the vector of examination periods for the timeline
  mitk::SortExaminationPeriods(m_ExaminationPeriods, m_ControlPoints);

  // get all information types points of current case
  m_InformationTypes = mitk::RelationStorage::GetAllInformationTypesOfCase(m_CaseID);

  if ("Image" == m_SelectedNodeType)
  {
    m_CurrentDataNodes = m_SemanticRelationsDataStorageAccess->GetAllImagesOfCase(m_CaseID);
  }
  else if ("Segmentation" == m_SelectedNodeType)
  {
    m_CurrentDataNodes = m_SemanticRelationsDataStorageAccess->GetAllSegmentationsOfCase(m_CaseID);
  }

  SetHeaderModel();
  SetPixmaps();
  SetLesionPresences();
}

void QmitkPatientTableModel::SetHeaderModel()
{
  m_HeaderModel->clear();
  QStandardItem* rootItem = new QStandardItem("Timeline");
  QList<QStandardItem*> standardItems;

  for (const auto& examinationPeriod : m_ExaminationPeriods)
  {
    QStandardItem* examinationPeriodItem = new QStandardItem(QString::fromStdString(examinationPeriod.name));
    standardItems.push_back(examinationPeriodItem);
    rootItem->appendColumn(standardItems);
    standardItems.clear();

    const auto& currentControlPoints = examinationPeriod.controlPointUIDs;
    for (const auto& controlPointUID : currentControlPoints)
    {
      const auto& controlPoint = mitk::GetControlPointByUID(controlPointUID, m_ControlPoints);
      QStandardItem* controlPointItem = new QStandardItem(QString::fromStdString(controlPoint.ToString()));
      standardItems.push_back(controlPointItem);
      examinationPeriodItem->appendColumn(standardItems);
      standardItems.clear();
    }
  }

  m_HeaderModel->setItem(0, 0, rootItem);
}

void QmitkPatientTableModel::SetPixmaps()
{
  m_PixmapMap.clear();
  for (const auto& dataNode : m_CurrentDataNodes)
  {
    // set the pixmap for the current node
    QPixmap pixmapFromImage = QmitkSemanticRelationsUIHelper::GetPixmapFromImageNode(dataNode);
    SetPixmapOfNode(dataNode, &pixmapFromImage);
  }
}

void QmitkPatientTableModel::SetPixmapOfNode(const mitk::DataNode* dataNode, QPixmap* pixmapFromImage)
{
  if (nullptr == dataNode)
  {
    return;
  }

  std::map<mitk::DataNode::ConstPointer, QPixmap>::iterator iter = m_PixmapMap.find(dataNode);
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
    m_PixmapMap.insert(std::make_pair(dataNode, pixmapFromImage->scaled(120, 120, Qt::IgnoreAspectRatio)));
  }
}

void QmitkPatientTableModel::SetLesionPresences()
{
  m_LesionPresence.clear();
  if (!mitk::SemanticRelationsInference::InstanceExists(m_CaseID, m_Lesion))
  {
    return;
  }

  for (const auto& dataNode : m_CurrentDataNodes)
  {
    if (!mitk::SemanticRelationsInference::InstanceExists(dataNode))
    {
      continue;
    }

    // set the lesion presence for the current node
    bool lesionPresence = mitk::SemanticRelationsInference::IsLesionPresent(m_Lesion, dataNode);
    SetLesionPresenceOfNode(dataNode, lesionPresence);
  }
}

void QmitkPatientTableModel::SetLesionPresenceOfNode(const mitk::DataNode* dataNode, bool lesionPresence)
{
  std::map<mitk::DataNode::ConstPointer, bool>::iterator iter = m_LesionPresence.find(dataNode);
  if (iter != m_LesionPresence.end())
  {
    // key already existing, overwrite already stored bool value
    iter->second = lesionPresence;
  }
  else
  {
    m_LesionPresence.insert(std::make_pair(dataNode, lesionPresence));
  }
}

mitk::DataNode* QmitkPatientTableModel::GetCurrentDataNode(const QModelIndex& index) const
{
  if (!index.isValid())
  {
    return nullptr;
  }

  mitk::SemanticTypes::ControlPoint currentControlPoint = m_ControlPoints.at(index.column());
  mitk::SemanticTypes::InformationType currentInformationType = m_InformationTypes.at(index.row());
  try
  {
    std::vector<mitk::DataNode::Pointer> filteredDataNodes;
    if ("Image" == m_SelectedNodeType)
    {
      filteredDataNodes = m_SemanticRelationsDataStorageAccess->GetAllSpecificImages(m_CaseID, currentControlPoint, currentInformationType);
    }
    else if ("Segmentation" == m_SelectedNodeType)
    {
      filteredDataNodes = m_SemanticRelationsDataStorageAccess->GetAllSpecificSegmentations(m_CaseID, currentControlPoint, currentInformationType);
    }

    if (filteredDataNodes.empty())
    {
      return nullptr;
    }
    return filteredDataNodes.front();
  }
  catch (const mitk::SemanticRelationException&)
  {
    return nullptr;
  }
}
