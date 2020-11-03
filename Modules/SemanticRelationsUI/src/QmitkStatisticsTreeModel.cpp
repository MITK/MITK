/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// semantic relations UI module
#include "QmitkStatisticsTreeModel.h"

// semantic relations module
#include <mitkControlPointManager.h>
#include <mitkLesionManager.h>
#include <mitkSemanticRelationException.h>
#include <mitkSemanticRelationsInference.h>
#include <mitkRelationStorage.h>

QmitkStatisticsTreeModel::QmitkStatisticsTreeModel(QObject* parent/* = nullptr*/)
  : QmitkAbstractSemanticRelationsStorageModel(parent)
  , m_RootItem(std::make_shared<QmitkLesionTreeItem>(mitk::LesionData()))
{
  m_StatisticsCalculator = std::make_unique<QmitkStatisticsCalculator>();
}

//////////////////////////////////////////////////////////////////////////
// overridden virtual functions from QAbstractItemModel
//////////////////////////////////////////////////////////////////////////
QModelIndex QmitkStatisticsTreeModel::index(int row, int column, const QModelIndex& itemIndex) const
{
  if (!hasIndex(row, column, itemIndex))
  {
    return QModelIndex();
  }

  auto childItem = GetItemByIndex(itemIndex)->GetChildInRow(row);
  if (nullptr == childItem)
  {
    return QModelIndex();
  }

  return createIndex(row, column, childItem.get());
}

QModelIndex QmitkStatisticsTreeModel::parent(const QModelIndex& itemIndex) const
{
  if (!itemIndex.isValid())
  {
    return QModelIndex();
  }

  auto parentItemWeakPtr = GetItemByIndex(itemIndex)->GetParent();
  if (parentItemWeakPtr.expired())
  {
    return QModelIndex();
  }

  auto parentItem = parentItemWeakPtr.lock();
  if (parentItem == m_RootItem)
  {
    return QModelIndex();
  }

  return createIndex(parentItem->GetRow(), 0, parentItem.get());
}

int QmitkStatisticsTreeModel::rowCount(const QModelIndex& itemIndex/* = QModelIndex()*/) const
{
  return GetItemByIndex(itemIndex)->ChildCount();
}

int QmitkStatisticsTreeModel::columnCount(const QModelIndex&/* itemIndex = QModelIndex() */) const
{
  if (0 == m_RootItem->ChildCount())
  {
    // no lesion items stored, no need to display columns
    return 0;
  }

  return m_ControlPoints.size() + 1;
}

QVariant QmitkStatisticsTreeModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }

  if (index.column() < 0 || index.column() > static_cast<int>(m_ControlPoints.size()))
  {
    return QVariant();
  }

  QmitkLesionTreeItem* currentItem = GetItemByIndex(index);
  if (Qt::DisplayRole == role)
  {
    if (currentItem->GetParent().expired())
    {
      return QVariant();
    }

    auto parentItem = currentItem->GetParent().lock();
    // parent exists and is the root item -> top level item
    if (m_RootItem == parentItem)
    {
      // display role fills the first columns with the lesion UID / name
      if (0 == index.column())
      {
        std::string itemString = currentItem->GetData().GetLesionName();
        if (itemString.empty())
        {
          itemString = currentItem->GetData().GetLesionUID();
        }
        return QString::fromStdString(itemString);
      }
    }
    // parent is not the root item -> volume item
    else
    {
      // display role fills the first columns with the information type
      if (0 == index.column())
      {
        if (index.row() < static_cast<int>(m_InformationTypes.size()))
        {
          return QString::fromStdString(m_InformationTypes.at(index.row()));
        }
        return "N/A";
      }
      else
      {
        // display role fills other columns with the lesion volume info
        const auto lesionVolume = currentItem->GetData().GetLesionVolume();
        if ((index.column() - 1) * index.row() < static_cast<int>(lesionVolume.size()))
        {
          return QVariant(lesionVolume.at(index.row()*m_ControlPoints.size() + (index.column() - 1)));
        }
        return "N/A";
      }
    }
  }

  return QVariant();
}

QVariant QmitkStatisticsTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (0 == m_RootItem->ChildCount())
  {
    // no lesion items stored, no need to display the header
    return QVariant();
  }

  if (Qt::Horizontal == orientation && Qt::DisplayRole == role)
  {
    if (0 == section)
    {
      return QVariant("Lesion");
    }

    if (static_cast<int>(m_ControlPoints.size()) >= section)
    {
      mitk::SemanticTypes::ControlPoint currentControlPoint = m_ControlPoints.at(section-1);
      return QVariant(QString::fromStdString(currentControlPoint.ToString()));
    }
  }

  return QVariant();
}

void QmitkStatisticsTreeModel::DataStorageChanged()
{
  if (!m_DataStorage.IsExpired())
  {
    auto dataStorage = m_DataStorage.Lock();
    m_SemanticRelationsDataStorageAccess = std::make_unique<mitk::SemanticRelationsDataStorageAccess>(dataStorage);
    m_StatisticsCalculator->SetDataStorage(dataStorage);
    UpdateModelData();
  }
}

void QmitkStatisticsTreeModel::NodeAdded(const mitk::DataNode*)
{
  emit beginResetModel();
  UpdateModelData();
  emit endResetModel();
}

void QmitkStatisticsTreeModel::NodeChanged(const mitk::DataNode*)
{
  emit beginResetModel();
  UpdateModelData();
  emit endResetModel();
}

void QmitkStatisticsTreeModel::NodeRemoved(const mitk::DataNode*)
{
  emit beginResetModel();
  UpdateModelData();
  emit endResetModel();
}

void QmitkStatisticsTreeModel::SetData()
{
  m_RootItem = std::make_shared<QmitkLesionTreeItem>(mitk::LesionData());

  // get all control points of current case
  m_ControlPoints = mitk::RelationStorage::GetAllControlPointsOfCase(m_CaseID);
  // sort the vector of control points for the timeline
  std::sort(m_ControlPoints.begin(), m_ControlPoints.end());

  // get all information types points of current case
  m_InformationTypes = mitk::RelationStorage::GetAllInformationTypesOfCase(m_CaseID);

  SetLesionData();
}

void QmitkStatisticsTreeModel::SetLesionData()
{
  m_CurrentLesions = mitk::RelationStorage::GetAllLesionsOfCase(m_CaseID);
  for (auto& lesion : m_CurrentLesions)
  {
    AddLesion(lesion);
  }
}

void QmitkStatisticsTreeModel::AddLesion(const mitk::SemanticTypes::Lesion& lesion)
{
  if (m_DataStorage.IsExpired())
  {
    return;
  }

  auto dataStorage = m_DataStorage.Lock();

  // create new lesion tree item data and modify it according to the control point data
  mitk::LesionData lesionData(lesion);
  m_StatisticsCalculator->ComputeLesionVolume(lesionData, m_CaseID);

  // add the 1. level lesion item to the root item
  std::shared_ptr<QmitkLesionTreeItem> newLesionTreeItem = std::make_shared<QmitkLesionTreeItem>(lesionData);
  m_RootItem->AddChild(newLesionTreeItem);

  for (size_t i = 0; i < m_InformationTypes.size(); ++i)
  {
    std::shared_ptr<QmitkLesionTreeItem> volumeItem = std::make_shared<QmitkLesionTreeItem>(lesionData);
    newLesionTreeItem->AddChild(volumeItem);
  }
}

QmitkLesionTreeItem* QmitkStatisticsTreeModel::GetItemByIndex(const QModelIndex& index) const
{
  if (index.isValid())
  {
    auto item = static_cast<QmitkLesionTreeItem*>(index.internalPointer());
    if (nullptr != item)
    {
      return item;
    }
  }

  return m_RootItem.get();
}
