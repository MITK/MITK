/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  return m_ExaminationPeriods.size();
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
   || index.column() < 0 || index.column() >= static_cast<int>(m_ExaminationPeriods.size()))
  {
    return QVariant();
  }

  mitk::DataNode* dataNode = GetCurrentDataNode(index);

  if (Qt::DecorationRole == role)
  {
    auto it = m_PixmapMap.find(dataNode);
    if (it != m_PixmapMap.end())
    {
      return QVariant(it->second);
    }

    auto emptyPixmap = QPixmap(120, 120);
    emptyPixmap.fill(Qt::transparent);
    return emptyPixmap;
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

  if (QmitkDataNodeRole == role)
  {
    return QVariant::fromValue<mitk::DataNode::Pointer>(mitk::DataNode::Pointer(dataNode));
  }

  if (QmitkDataNodeRawPointerRole == role)
  {
    return QVariant::fromValue<mitk::DataNode*>(dataNode);
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
  // get all examination periods of current case
  m_ExaminationPeriods = mitk::RelationStorage::GetAllExaminationPeriodsOfCase(m_CaseID);

  // sort all examination periods for the timeline
  mitk::SortAllExaminationPeriods(m_CaseID, m_ExaminationPeriods);

  // rename examination periods according to their new order
  std::string examinationPeriodName = "Baseline";
  for (size_t i = 0; i < m_ExaminationPeriods.size(); ++i)
  {
    auto& examinationPeriod = m_ExaminationPeriods.at(i);
    examinationPeriod.name = examinationPeriodName;
    mitk::RelationStorage::RenameExaminationPeriod(m_CaseID, examinationPeriod);
    examinationPeriodName = "Follow-up " + std::to_string(i);
  }

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

  auto examinationPeriod = m_ExaminationPeriods.at(index.column());
  auto currentInformationType = m_InformationTypes.at(index.row());
  auto controlPointsOfExaminationPeriod = examinationPeriod.controlPointUIDs;
  for (const auto& controlPointUID : controlPointsOfExaminationPeriod)
  {
    auto currentControlPoint = mitk::GetControlPointByUID(m_CaseID, controlPointUID);
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
        // try next control point
        continue;
      }
      else
      {
        // found a specific image
        return filteredDataNodes.front();
      }
    }
    catch (const mitk::SemanticRelationException&)
    {
      return nullptr;
    }
  }
  // could not find a specif image
  return nullptr;
}
