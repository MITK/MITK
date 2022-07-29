/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMultiLabelSegmentationTreeModel.h"

#include "mitkImageStatisticsContainerManager.h"
#include "mitkProportionalTimeGeometry.h"
#include "mitkStatisticsToImageRelationRule.h"
#include "mitkStatisticsToMaskRelationRule.h"

#include "QmitkStyleManager.h"


class QmitkMultiLabelSegTreeItem
{
public:
  enum class ItemType
  {
    Group,
    Label,
    Instance
  };

  QmitkMultiLabelSegTreeItem()
  {
  };

  explicit QmitkMultiLabelSegTreeItem(QVariant label, ItemType type, unsigned int groupID,
    mitk::LabelSetImage::PixelType labelID = 0, QmitkMultiLabelSegTreeItem* parentItem = nullptr): m_label(label), m_ItemType(type),
    m_GroupID(groupID), m_LabelID(labelID), m_parentItem(parentItem)
  {
  };

  ~QmitkMultiLabelSegTreeItem()
  {
    for (auto item : m_childItems)
    {
      delete item;
    }
  };

  void appendChild(QmitkMultiLabelSegTreeItem* child)
  {
      m_childItems.push_back(child);
  };

  QmitkMultiLabelSegTreeItem* child(int row)
  {
    return m_childItems[row];
  };

  int childCount() const
  {
    return m_childItems.size();
  };

  QVariant data(int column) const
  {
    QVariant result;
    return result;
  };

  int row() const
  {
    if (m_parentItem)
    {
      auto finding = std::find(m_parentItem->m_childItems.begin(), m_parentItem->m_childItems.end(), this);
      if (finding != m_parentItem->m_childItems.end())
      {
        return std::distance(m_parentItem->m_childItems.begin(), finding);
      }
    }

    return 0;
  };

  QmitkMultiLabelSegTreeItem* parentItem()
  {
    return m_parentItem;
  };

  QVariant m_label;
  QmitkMultiLabelSegTreeItem* m_parentItem = nullptr;
  std::vector<QmitkMultiLabelSegTreeItem*> m_childItems;
  ItemType m_ItemType = ItemType::Group;
  mitk::LabelSetImage::PixelType m_LabelID = 0;
  unsigned int m_GroupID = 0;
};


QmitkMultiLabelSegmentationTreeModel::QmitkMultiLabelSegmentationTreeModel(QObject *parent) : QAbstractItemModel(parent), m_ModifiedObserverTag(0)
, m_Observed(false)
{
  m_RootItem = new QmitkMultiLabelSegTreeItem();
}

QmitkMultiLabelSegmentationTreeModel ::~QmitkMultiLabelSegmentationTreeModel()
{
  this->SetSegmentation(nullptr);
  delete m_RootItem;
};

int QmitkMultiLabelSegmentationTreeModel::columnCount(const QModelIndex& /*parent*/) const
{
  return 4;
}

int QmitkMultiLabelSegmentationTreeModel::rowCount(const QModelIndex &parent) const
{
  if (parent.column() > 0)
    return 0;

  if (m_Segmentation.IsNull())
    return 0;

  QmitkMultiLabelSegTreeItem* parentItem = m_RootItem;

  if (parent.isValid())
    parentItem = static_cast<QmitkMultiLabelSegTreeItem *>(parent.internalPointer());

  if (parentItem->m_ItemType == QmitkMultiLabelSegTreeItem::ItemType::Label && parentItem->childCount() == 1)
  { //special case there is only one instance for a label, so we can ignore the instance child leafs as information is redundant.
    return 0;
  }

  return parentItem->childCount();
}

QVariant QmitkMultiLabelSegmentationTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  QmitkMultiLabelSegTreeItem* item = static_cast<QmitkMultiLabelSegTreeItem*>(index.internalPointer());

  if (role == Qt::DisplayRole)
  {
    return item->data(index.column());
  }
  return QVariant();
}

QModelIndex QmitkMultiLabelSegmentationTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  QmitkMultiLabelSegTreeItem *parentItem = m_RootItem;

  if (parent.isValid())
    parentItem = static_cast<QmitkMultiLabelSegTreeItem *>(parent.internalPointer());

  QmitkMultiLabelSegTreeItem *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex QmitkMultiLabelSegmentationTreeModel::parent(const QModelIndex &child) const
{
  if (!child.isValid())
    return QModelIndex();

  QmitkMultiLabelSegTreeItem *childItem = static_cast<QmitkMultiLabelSegTreeItem *>(child.internalPointer());
  QmitkMultiLabelSegTreeItem *parentItem = childItem->parentItem();

  if (parentItem == m_RootItem)
    return QModelIndex();

  return createIndex(parentItem->row(), 0, parentItem);
}

Qt::ItemFlags QmitkMultiLabelSegmentationTreeModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return nullptr;

  return QAbstractItemModel::flags(index);
}

QVariant QmitkMultiLabelSegmentationTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((Qt::DisplayRole == role) && (Qt::Horizontal == orientation))
  {
    if (TableColumns::NAME_COL == section)
    {
      return "Name";
    }
    else if (TableColumns::LOCKED_COL == section)
    {
      return "Locked";
    }
    else if (TableColumns::COLOR_COL == section)
    {
      return "Color";
    }
    else if (TableColumns::VISIBLE_COL == section)
    {
      return "Visibility";
    }
  }
  return QVariant();
}

void QmitkMultiLabelSegmentationTreeModel::SetSegmentation(mitk::LabelSetImage* segmentation)
{
  if (m_Segmentation != segmentation)
  {
    this->RemoveObserver();
    this->m_Segmentation = segmentation;
    this->AddObserver();
  }

  emit beginResetModel();
  UpdateBySegmentation();
  emit endResetModel();
  emit modelChanged();
}

void QmitkMultiLabelSegmentationTreeModel::UpdateBySegmentation()
{
  // reset old model
  delete m_RootItem;
  m_RootItem = new QmitkMultiLabelSegTreeItem();

  std::map<std::string, QmitkMultiLabelSegTreeItem*> labelNameToTreeItem;

  if (m_Segmentation.IsNull()) return;

  for (unsigned int layerID = 0; layerID < m_Segmentation->GetNumberOfLayers(); ++layerID)
  {
    auto layerItem = new QmitkMultiLabelSegTreeItem("Group ", QmitkMultiLabelSegTreeItem::ItemType::Group, layerID, 0, m_RootItem);
    m_RootItem->appendChild(layerItem);

    auto labelSet = m_Segmentation->GetLabelSet(layerID);

    QmitkMultiLabelSegTreeItem* labelItem = nullptr;

    for (auto lIter = labelSet->IteratorConstBegin(); lIter != labelSet->IteratorConstEnd(); lIter++)
    {
      auto finding = labelNameToTreeItem.find(lIter->second->GetName());
      if (finding != labelNameToTreeItem.end())
      { //other label with same name exists
        labelItem = finding->second;
      }
      else
      {
        labelItem = new QmitkMultiLabelSegTreeItem(QString::fromStdString(lIter->second->GetName()), QmitkMultiLabelSegTreeItem::ItemType::Label, layerID, lIter->first, layerItem);
        layerItem->appendChild(labelItem);
        labelNameToTreeItem.emplace(lIter->second->GetName(), labelItem);
      }

      auto instanceItem = new QmitkMultiLabelSegTreeItem("", QmitkMultiLabelSegTreeItem::ItemType::Instance, layerID, lIter->first, labelItem);
      labelItem->appendChild(instanceItem);
    }
  }
}

void QmitkMultiLabelSegmentationTreeModel::OnModified(const itk::Object* /*caller*/, const itk::EventObject&)
{
  emit beginResetModel();
  UpdateBySegmentation();
  emit endResetModel();
  emit modelChanged();
}

void QmitkMultiLabelSegmentationTreeModel::AddObserver()
{
  if (this->m_Segmentation.IsNotNull())
  {
    if (m_Observed)
    {
      MITK_DEBUG << "Invalid observer state in QmitkMultiLabelSegmentationTreeModel. There is already a registered observer. Internal logic is not correct. May be an old observer was not removed.";
    }

    auto modifiedCommand = itk::MemberCommand<QmitkMultiLabelSegmentationTreeModel>::New();
    modifiedCommand->SetCallbackFunction(this, &QmitkMultiLabelSegmentationTreeModel::OnModified);

    m_ModifiedObserverTag = this->m_Segmentation->AddObserver(itk::ModifiedEvent(), modifiedCommand);
    m_Observed = true;
  }
}

void QmitkMultiLabelSegmentationTreeModel::RemoveObserver()
{
  if (this->m_Segmentation.IsNotNull())
  {
    this->m_Segmentation->RemoveObserver(m_ModifiedObserverTag);
  }
  m_Observed = false;
}
//
//treeview ableiten und selection command überschreiben. soll nur reagieren by labels bzw instancen, wenn es welche gibt
//und nur in column 0
