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
#include "QmitkLesionTreeModel.h"

// semantic relations module
#include <mitkControlPointManager.h>

QmitkLesionTreeModel::QmitkLesionTreeModel(QObject* parent/* = nullptr*/)
  : QmitkAbstractSemanticRelationsStorageModel(parent)
  , m_RootItem(std::make_shared<QmitkLesionTreeItem>("RootItem"))
{
  // nothing here
}

QmitkLesionTreeModel::~QmitkLesionTreeModel()
{
  // nothing here
}

//////////////////////////////////////////////////////////////////////////
// overridden virtual functions from QAbstractItemModel
//////////////////////////////////////////////////////////////////////////
QModelIndex QmitkLesionTreeModel::index(int row, int column, const QModelIndex& itemIndex) const
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

QModelIndex QmitkLesionTreeModel::parent(const QModelIndex& itemIndex) const
{
  if (!itemIndex.isValid())
  {
    return QModelIndex();
  }

  auto parentItem = GetItemByIndex(itemIndex)->GetParent();

  if (parentItem.expired())
  {
    return QModelIndex();
  }

  /*
  if (sharedParentPtr == m_RootItem)
  {
    return QModelIndex();
  }
  */
  auto sharedParentItem = parentItem.lock();
  return createIndex(sharedParentItem->GetRow(), 0, sharedParentItem.get());
}

int QmitkLesionTreeModel::rowCount(const QModelIndex& itemIndex/* = QModelIndex()*/) const
{
  return GetItemByIndex(itemIndex)->ChildCount();
}

int QmitkLesionTreeModel::columnCount(const QModelIndex&/* itemIndex = QModelIndex() */) const
{
  return m_ControlPoints.size() + 1;
}

QVariant QmitkLesionTreeModel::data(const QModelIndex& index, int role) const
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
    // display role fills the columns with certain strings or numbers
    if (0 == index.column())
    {
      return QVariant::fromValue(QString::fromStdString(currentItem->GetLesionID()));
      //return QVariant::fromValue(QString::fromStdString(currentItem->GetData().GetLesionName()));
    }
    else if (1 == index.column())
    {
      //return currentItem->GetData().GetStoredInDatabase();
    }
    else if (2 == index.column())
    {
      //return currentItem->GetData().GetNumberOfLoadedDataNodes();
    }

    return "";
  }

  return QVariant();
}

QVariant QmitkLesionTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (Qt::Horizontal == orientation && Qt::DisplayRole == role)
  {
    if (0 == section)
    {
      return QVariant("Lesion");
    }

    if (m_ControlPoints.size() >= section)
    {
      mitk::SemanticTypes::ControlPoint currentControlPoint = m_ControlPoints.at(section-1);
      // generate a string from the control point
      std::string currentControlPointAsString = mitk::GetControlPointAsString(currentControlPoint);
      return QVariant(QString::fromStdString(currentControlPointAsString));
    }
  }

  return QVariant();
}

void QmitkLesionTreeModel::NodePredicateChanged()
{
  // does not react to node predicate changes
}

void QmitkLesionTreeModel::NodeAdded(const mitk::DataNode* node)
{
  // does not react to data storage changes
}

void QmitkLesionTreeModel::NodeChanged(const mitk::DataNode* node)
{
  // does not react to data storage changes
}

void QmitkLesionTreeModel::NodeRemoved(const mitk::DataNode* node)
{
  // does not react to data storage changes
}

void QmitkLesionTreeModel::SetData()
{
  m_RootItem = std::make_shared<QmitkLesionTreeItem>("RootItem");

  // get all control points of current case
  m_ControlPoints = m_SemanticRelations->GetAllControlPointsOfCase(m_CaseID);
  // sort the vector of control points for the timeline
  std::sort(m_ControlPoints.begin(), m_ControlPoints.end());
  
  SetLesionData();
}

void QmitkLesionTreeModel::SetLesionData()
{
  std::vector<mitk::SemanticTypes::Lesion> allLesions = m_SemanticRelations->GetAllLesionsOfCase(m_CaseID);

  for (const auto& lesion : allLesions)
  {
    AddLesion(lesion);
  }
}

void QmitkLesionTreeModel::AddLesion(mitk::SemanticTypes::Lesion lesion)
{
  if (nullptr == m_SemanticRelations)
  {
    return;
  }

  m_RootItem->AddChild(std::make_shared<QmitkLesionTreeItem>(lesion.UID));
}

QmitkLesionTreeItem* QmitkLesionTreeModel::GetItemByIndex(const QModelIndex& index) const
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
