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

QModelIndex QmitkPatientTableModel::parent(const QModelIndex& child) const
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
  return m_ControlPoints.size();
}

QVariant QmitkPatientTableModel::data(const QModelIndex& index, int role/* = Qt::DisplayRole*/) const
{
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
      return it->second ? QVariant(QColor(Qt::green)) : QVariant(QColor(Qt::transparent));
    }

    return QVariant(QColor(Qt::transparent));
  }

  return QVariant();
}

QVariant QmitkPatientTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
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

void QmitkPatientTableModel::NodeAdded(const mitk::DataNode* node)
{
  // does not react to data storage changes
}

void QmitkPatientTableModel::NodeChanged(const mitk::DataNode* node)
{
  // nothing here, since the "'NodeChanged'-event is currently sent far too often
  //UpdateModelData();
}

void QmitkPatientTableModel::NodeRemoved(const mitk::DataNode* node)
{
  // does not react to data storage changes
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

void QmitkPatientTableModel::SetLesionPresence(const mitk::DataNode* dataNode, bool lesionPresence)
{
  if (nullptr == dataNode)
  {
    return;
  }

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

void QmitkPatientTableModel::SetData()
{
  // get all control points of current case
  m_ControlPoints = m_SemanticRelations->GetAllControlPointsOfCase(m_CaseID);
  // sort the vector of control points for the timeline
  std::sort(m_ControlPoints.begin(), m_ControlPoints.end());

  // get all examination periods of current case
  m_ExaminationPeriods = m_SemanticRelations->GetAllExaminationPeriodsOfCase(m_CaseID);
  // sort the vector of examination periods for the timeline
  mitk::SortExaminationPeriods(m_ExaminationPeriods, m_ControlPoints);

  // get all information types points of current case
  m_InformationTypes = m_SemanticRelations->GetAllInformationTypesOfCase(m_CaseID);

  m_PixmapMap.clear();
  m_LesionPresence.clear();

  SetDataNodes();
  SetHeaderModel();
}

void QmitkPatientTableModel::SetDataNodes()
{
  std::vector<mitk::DataNode::Pointer> allDataNodes;
  if("Image" == m_SelectedNodeType)
  {
    allDataNodes = m_SemanticRelations->GetAllImagesOfCase(m_CaseID);
  }
  else if("Segmentation" == m_SelectedNodeType)
  {
    allDataNodes = m_SemanticRelations->GetAllSegmentationsOfCase(m_CaseID);
  }

  for (const auto& dataNode : allDataNodes)
  {
    // set the pixmap for the current node
    QPixmap pixmapFromImage = QmitkSemanticRelationsUIHelper::GetPixmapFromImageNode(dataNode);
    SetPixmapOfNode(dataNode, &pixmapFromImage);

    // set the lesion presence for the current node
    if (m_SemanticRelations->InstanceExists(m_CaseID, m_Lesion))
    {
      bool lesionPresence = lesionPresence = IsLesionPresentOnDataNode(dataNode);
      SetLesionPresence(dataNode, lesionPresence);
    }
    else
    {
      m_LesionPresence.clear();
    }
  }
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
      std::string controlPointAsString = mitk::GetControlPointAsString(controlPoint);
      QStandardItem* controlPointItem = new QStandardItem(QString::fromStdString(controlPointAsString));
      standardItems.push_back(controlPointItem);
      examinationPeriodItem->appendColumn(standardItems);
      standardItems.clear();
    }
  }

  m_HeaderModel->setItem(0, 0, rootItem);
}

bool QmitkPatientTableModel::IsLesionPresentOnDataNode(const mitk::DataNode* dataNode) const
{
  if (m_DataStorage.IsExpired())
  {
    return false;
  }

  auto dataStorage = m_DataStorage.Lock();
  try
  {
    mitk::SemanticRelations::DataNodeVector allSegmentationsOfLesion = m_SemanticRelations->GetAllSegmentationsOfLesion(m_CaseID, m_Lesion);
    for (const auto& segmentation : allSegmentationsOfLesion)
    {
      if ("Segmentation" == m_SelectedNodeType)
      {
        if (dataNode == segmentation)
        {
          // found a segmentation of the node that is represented by the selected lesion
          return true;
        }
      }
      else if ("Image" == m_SelectedNodeType)
      {
        // get parent node of the current segmentation node with the node predicate
        mitk::DataStorage::SetOfObjects::ConstPointer parentNodes = dataStorage->GetSources(segmentation, mitk::NodePredicates::GetImagePredicate(), false);
        for (auto it = parentNodes->Begin(); it != parentNodes->End(); ++it)
        {
          if (dataNode == it.Value())
          {
            // found a segmentation of the node that is represented by the selected lesion
            return true;
          }
        }
      }
    }
  }
  catch (const mitk::SemanticRelationException&)
  {
    return false;
  }

  return false;
}

mitk::DataNode* QmitkPatientTableModel::GetCurrentDataNode(const QModelIndex& index) const
{
  mitk::SemanticTypes::ControlPoint currentControlPoint = m_ControlPoints.at(index.column());
  mitk::SemanticTypes::InformationType currentInformationType = m_InformationTypes.at(index.row());
  try
  {
    std::vector<mitk::DataNode::Pointer> filteredDataNodes;
    if ("Image" == m_SelectedNodeType)
    {

      filteredDataNodes = m_SemanticRelations->GetAllSpecificImages(m_CaseID, currentControlPoint, currentInformationType);
    }
    else if ("Segmentation" == m_SelectedNodeType)
    {
      filteredDataNodes = m_SemanticRelations->GetAllSpecificSegmentations(m_CaseID, currentControlPoint, currentInformationType);
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
