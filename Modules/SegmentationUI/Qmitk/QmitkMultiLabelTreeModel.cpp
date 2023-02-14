/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMultiLabelTreeModel.h"

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

  explicit QmitkMultiLabelSegTreeItem(ItemType type, QmitkMultiLabelSegTreeItem* parentItem,
    mitk::Label* label = nullptr, std::string className = ""): m_ItemType(type), m_Label(label), m_parentItem(parentItem), m_ClassName(className)
  {
  };

  ~QmitkMultiLabelSegTreeItem()
  {
    for (auto item : m_childItems)
    {
      delete item;
    }
  };

  void AppendChild(QmitkMultiLabelSegTreeItem* child)
  {
      m_childItems.push_back(child);
  };

  void RemoveChild(std::size_t row)
  {
    if (row < m_childItems.size())
    {
      delete m_childItems[row];
      m_childItems.erase(m_childItems.begin() + row);
    }
  };

  int Row() const
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

  QmitkMultiLabelSegTreeItem* ParentItem()
  {
    return m_parentItem;
  };

  const QmitkMultiLabelSegTreeItem* RootItem() const
  {
    auto item = this;
    while (item->m_parentItem != nullptr)
    {
      item = item->m_parentItem;
    }
    return item;
  };

  std::size_t GetGroupID() const
  {
    auto root = this->RootItem();
    auto item = this;
    if (root == this) return 0;

    while (root != item->m_parentItem)
    {
      item = item->m_parentItem;
    }

    auto iter = std::find(root->m_childItems.begin(), root->m_childItems.end(), item);

    if (root->m_childItems.end() == iter) mitkThrow() << "Invalid internal state of QmitkMultiLabelTreeModel. Root does not have an item as child that has root as parent.";

    return std::distance(root->m_childItems.begin(), iter);
  }

  bool HandleAsInstance() const
  {
    return (ItemType::Instance == m_ItemType) || ((ItemType::Label == m_ItemType) && (m_childItems.size() == 1));
  }

  mitk::Label* GetLabel() const
  {
    if (ItemType::Instance == m_ItemType)
    {
      return m_Label;
    }
    if (ItemType::Label == m_ItemType)
    {
      if (m_childItems.empty()) mitkThrow() << "Invalid internal state of QmitkMultiLabelTreeModel. Internal label item has no instance item.";
      return m_childItems[0]->GetLabel();
    }

    return nullptr;
  };

  mitk::LabelSetImage::LabelValueType GetLabelValue() const
  {
    auto label = this->GetLabel();

    if (nullptr == label)
    {
      mitkThrow() << "Invalid internal state of QmitkMultiLabelTreeModel. Called GetLabelValue on an group item.";
    }

    return label->GetValue();
  };

  QmitkMultiLabelSegTreeItem* m_parentItem = nullptr;
  std::vector<QmitkMultiLabelSegTreeItem*> m_childItems;
  ItemType m_ItemType = ItemType::Group;
  mitk::Label::Pointer m_Label;
  std::string m_ClassName;
};


QmitkMultiLabelTreeModel::QmitkMultiLabelTreeModel(QObject *parent) : QAbstractItemModel(parent)
, m_Observed(false)
{
  m_RootItem = std::make_unique<QmitkMultiLabelSegTreeItem>();
}

QmitkMultiLabelTreeModel ::~QmitkMultiLabelTreeModel()
{
  this->SetSegmentation(nullptr);
};

int QmitkMultiLabelTreeModel::columnCount(const QModelIndex& /*parent*/) const
{
  return 4;
}

int QmitkMultiLabelTreeModel::rowCount(const QModelIndex &parent) const
{
  if (parent.column() > 0)
    return 0;

  if (m_Segmentation.IsNull())
    return 0;

  QmitkMultiLabelSegTreeItem* parentItem = m_RootItem.get();

  if (parent.isValid())
    parentItem = static_cast<QmitkMultiLabelSegTreeItem *>(parent.internalPointer());

  if (parentItem->HandleAsInstance())
  {
    return 0;
  }

  return parentItem->m_childItems.size();
}

QVariant QmitkMultiLabelTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  auto item = static_cast<QmitkMultiLabelSegTreeItem*>(index.internalPointer());

  if (!item)
    return QVariant();

  if (role == Qt::DisplayRole||role == Qt::EditRole)
  {
    if (TableColumns::NAME_COL == index.column())
    {
      switch (item->m_ItemType)
      {
        case QmitkMultiLabelSegTreeItem::ItemType::Group:
          return QVariant(QString("Group ")+QString::number(item->GetGroupID()));
        case QmitkMultiLabelSegTreeItem::ItemType::Label:
        {
          auto label = item->GetLabel();
          if (nullptr == label) mitkThrow() << "Invalid internal state. QmitkMultiLabelTreeModel item is refering to a label that does not exist.";
          return QVariant(QString::fromStdString(label->GetName()));
        }
        case QmitkMultiLabelSegTreeItem::ItemType::Instance:
          return QVariant(QString("Instance #") + QString::number(item->GetLabelValue()));
      }
    }
    else
    {
      if (item->HandleAsInstance())
      {
        auto label = item->GetLabel();

        if (TableColumns::LOCKED_COL == index.column())
        {
          return QVariant(label->GetLocked());
        }
        else if (TableColumns::COLOR_COL == index.column())
        {
          return QVariant(QColor(label->GetColor().GetRed() * 255, label->GetColor().GetGreen() * 255, label->GetColor().GetBlue() * 255));
        }
        else if (TableColumns::VISIBLE_COL == index.column())
        {
          return QVariant(label->GetVisible());
        }
      }
      else
      {

      }
    }
  }
  else if (role == ItemModelRole::LabelDataRole)
  {
    if (TableColumns::NAME_COL == index.column())
    {
      if (item->HandleAsInstance())
      {
        auto label = item->GetLabel();
        return QVariant::fromValue<void*>(label);
      }
    }
  }
  else if (role == ItemModelRole::LabelValueRole)
  {
    if (TableColumns::NAME_COL == index.column())
    {
      if (item->HandleAsInstance())
      {
        auto label = item->GetLabel();
        return QVariant(label->GetValue());
      }
    }
  }
  return QVariant();
}

mitk::Color QtToMitk(const QColor& color)
{
  mitk::Color mitkColor;

  mitkColor.SetRed(color.red() / 255.0f);
  mitkColor.SetGreen(color.green() / 255.0f);
  mitkColor.SetBlue(color.blue() / 255.0f);

  return mitkColor;
}

bool QmitkMultiLabelTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid())
    return false;

  auto item = static_cast<QmitkMultiLabelSegTreeItem*>(index.internalPointer());

  if (!item)
    return false;

  if (role == Qt::EditRole)
  {
    if (TableColumns::NAME_COL != index.column())
    {
      if (item->HandleAsInstance())
      {
        auto label = item->GetLabel();

        if (TableColumns::LOCKED_COL == index.column())
        {
          label->SetLocked(value.toBool());
        }
        else if (TableColumns::COLOR_COL == index.column())
        {
          label->SetColor(QtToMitk(value.value<QColor>()));
        }
        else if (TableColumns::VISIBLE_COL == index.column())
        {
          label->SetVisible(value.toBool());
        }
        auto groupID = m_Segmentation->GetGroupIndexOfLabel(label->GetValue());
        m_Segmentation->GetLabelSet(groupID)->UpdateLookupTable(label->GetValue());
      }
      else
      {

      }
      return true;
    }
  }
  return false;
}

QModelIndex QmitkMultiLabelTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  auto parentItem = m_RootItem.get();

  if (parent.isValid())
    parentItem = static_cast<QmitkMultiLabelSegTreeItem *>(parent.internalPointer());

  QmitkMultiLabelSegTreeItem *childItem = parentItem->m_childItems[row];
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex QmitkMultiLabelTreeModel::parent(const QModelIndex &child) const
{
  if (!child.isValid())
    return QModelIndex();

  QmitkMultiLabelSegTreeItem *childItem = static_cast<QmitkMultiLabelSegTreeItem *>(child.internalPointer());
  QmitkMultiLabelSegTreeItem *parentItem = childItem->ParentItem();

  if (parentItem == m_RootItem.get())
    return QModelIndex();

  return createIndex(parentItem->Row(), 0, parentItem);
}

QModelIndex GetIndexByItem(QmitkMultiLabelSegTreeItem* start, QmitkMultiLabelTreeModel* model)
{
  QModelIndex parentIndex = QModelIndex();
  if (nullptr != start->m_parentItem)
  {
    parentIndex = GetIndexByItem(start->m_parentItem, model);
  }
  else
  {
    return parentIndex;
  }

  return model->index(start->Row(), 0, parentIndex);
}

QmitkMultiLabelSegTreeItem* GetGroupItem(QmitkMultiLabelTreeModel::SpatialGroupIndexType groupIndex, QmitkMultiLabelSegTreeItem* root)
{
  if (nullptr != root && groupIndex < root->m_childItems.size())
  {
    return root->m_childItems[groupIndex];
  }

  return nullptr;
}

QmitkMultiLabelSegTreeItem* GetInstanceItem(QmitkMultiLabelTreeModel::LabelValueType labelValue, QmitkMultiLabelSegTreeItem* root)
{
  QmitkMultiLabelSegTreeItem* result = nullptr;

  for (auto item : root->m_childItems)
  {
    result = GetInstanceItem(labelValue, item);
    if (nullptr != result) return result;
  }

  if (root->m_ItemType == QmitkMultiLabelSegTreeItem::ItemType::Instance && root->GetLabelValue() == labelValue)
  {
    return root;
  }

  return nullptr;
}

QmitkMultiLabelSegTreeItem* GetLabelItemInGroup(const std::string& labelName, QmitkMultiLabelSegTreeItem * group)
{
  if (nullptr != group)
  {
    auto predicate = [labelName](const QmitkMultiLabelSegTreeItem* item) { return labelName == item->m_ClassName; };
    auto finding = std::find_if(group->m_childItems.begin(), group->m_childItems.end(), predicate);
    if (group->m_childItems.end() != finding)
    {
      return *finding;
    }
  }

  return nullptr;
}

Qt::ItemFlags QmitkMultiLabelTreeModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::NoItemFlags;

  if (!index.isValid())
    return Qt::NoItemFlags;

  auto item = static_cast<QmitkMultiLabelSegTreeItem*>(index.internalPointer());

  if (!item)
    return Qt::NoItemFlags;

  if (TableColumns::NAME_COL != index.column())
  {
    if (item->HandleAsInstance())
    {
      return Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }
    else
    {
      return Qt::ItemIsEnabled;
    }
    return true;
  }
  else
  {
    if (item->HandleAsInstance())
    {
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    else
    {
      return Qt::ItemIsEnabled;
    }
    return true;
  }

  return Qt::NoItemFlags;
}

QVariant QmitkMultiLabelTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
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

const mitk::LabelSetImage* QmitkMultiLabelTreeModel::GetSegmentation() const
{
  return m_Segmentation;
}


void QmitkMultiLabelTreeModel::SetSegmentation(mitk::LabelSetImage* segmentation)
{
  if (m_Segmentation != segmentation)
  {
    this->RemoveObserver();
    this->m_Segmentation = segmentation;
    this->AddObserver();

    this->UpdateInternalTree();
  }
}


/**Helper function that adds a labek into the item tree. Passes back the new created instance iten*/
QmitkMultiLabelSegTreeItem* AddLabelToGroupTree(mitk::Label* label, QmitkMultiLabelSegTreeItem* groupItem, bool& newLabelItemCreated)
{
  if (nullptr == groupItem) return nullptr;
  if (nullptr == label) return nullptr;

  newLabelItemCreated = false;

  std::set<std::string> labelNames;
  for (auto labelItem : groupItem->m_childItems)
  {
    labelNames.emplace(labelItem->GetLabel()->GetName());
  }

  QmitkMultiLabelSegTreeItem* labelItem = nullptr;
  auto finding = labelNames.find(label->GetName());
  if (finding != labelNames.end())
  { //other label with same name exists
    labelItem = groupItem->m_childItems[std::distance(labelNames.begin(), finding)];
  }
  else
  {
    newLabelItemCreated = true;
    labelItem = new QmitkMultiLabelSegTreeItem(QmitkMultiLabelSegTreeItem::ItemType::Label, groupItem, nullptr, label->GetName());

    auto predicate = [label](const std::string& name) { return name > label->GetName(); };
    auto insertFinding = std::find_if(labelNames.begin(), labelNames.end(), predicate);

    groupItem->m_childItems.insert(groupItem->m_childItems.begin() + std::distance(labelNames.begin(), insertFinding), labelItem);
  }

  auto instanceItem = new QmitkMultiLabelSegTreeItem(QmitkMultiLabelSegTreeItem::ItemType::Instance, labelItem, label);

  auto predicate = [label](const QmitkMultiLabelSegTreeItem* item) { return item->GetLabelValue() > label->GetValue(); };
  auto insertFinding = std::find_if(labelItem->m_childItems.begin(), labelItem->m_childItems.end(), predicate);
  labelItem->m_childItems.insert(labelItem->m_childItems.begin() + std::distance(labelItem->m_childItems.begin(), insertFinding), instanceItem);

  return instanceItem;
}

void QmitkMultiLabelTreeModel::GenerateInternalGroupTree(unsigned int groupID, QmitkMultiLabelSegTreeItem* groupItem)
{
  auto labelSet = m_Segmentation->GetLabelSet(groupID);

  for (auto lIter = labelSet->IteratorConstBegin(); lIter != labelSet->IteratorConstEnd(); lIter++)
  {
    if (lIter->second == m_Segmentation->GetExteriorLabel()) continue;

    bool newItemCreated = false;
    AddLabelToGroupTree(lIter->second, groupItem, newItemCreated);
  }
}

QmitkMultiLabelSegTreeItem* QmitkMultiLabelTreeModel::GenerateInternalTree()
{
  auto rootItem = new QmitkMultiLabelSegTreeItem();

  if (m_Segmentation.IsNotNull())
  {
    for (unsigned int groupID = 0; groupID < m_Segmentation->GetNumberOfLayers(); ++groupID)
    {
      auto groupItem = new QmitkMultiLabelSegTreeItem(QmitkMultiLabelSegTreeItem::ItemType::Group, rootItem);
      rootItem->AppendChild(groupItem);

      GenerateInternalGroupTree(groupID, groupItem);
    }
  }

  return rootItem;
}

void QmitkMultiLabelTreeModel::UpdateInternalTree()
{
  emit beginResetModel();
  auto newTree = this->GenerateInternalTree();
  this->m_RootItem.reset(newTree);
  emit endResetModel();
  emit modelChanged();
}

void QmitkMultiLabelTreeModel::AddObserver()
{
  if (this->m_Segmentation.IsNotNull())
  {
    if (m_Observed)
    {
      MITK_DEBUG << "Invalid observer state in QmitkMultiLabelTreeModel. There is already a registered observer. Internal logic is not correct. May be an old observer was not removed.";
    }

    this->m_Segmentation->AddLabelAddedListener(mitk::MessageDelegate1<QmitkMultiLabelTreeModel, LabelValueType>(
      this, &QmitkMultiLabelTreeModel::OnLabelAdded));
    this->m_Segmentation->AddLabelModifiedListener(mitk::MessageDelegate1<QmitkMultiLabelTreeModel, LabelValueType>(
      this, &QmitkMultiLabelTreeModel::OnLabelModified));
    this->m_Segmentation->AddLabelRemovedListener(mitk::MessageDelegate1<QmitkMultiLabelTreeModel, LabelValueType>(
      this, &QmitkMultiLabelTreeModel::OnLabelRemoved));
    this->m_Segmentation->AddGroupAddedListener(mitk::MessageDelegate1<QmitkMultiLabelTreeModel, SpatialGroupIndexType>(
      this, &QmitkMultiLabelTreeModel::OnGroupAdded));
    this->m_Segmentation->AddGroupModifiedListener(mitk::MessageDelegate1<QmitkMultiLabelTreeModel, SpatialGroupIndexType>(
      this, &QmitkMultiLabelTreeModel::OnGroupModified));
    this->m_Segmentation->AddGroupRemovedListener(mitk::MessageDelegate1<QmitkMultiLabelTreeModel, SpatialGroupIndexType>(
      this, &QmitkMultiLabelTreeModel::OnGroupRemoved));
    m_Observed = true;
  }
}

void QmitkMultiLabelTreeModel::RemoveObserver()
{
  if (this->m_Segmentation.IsNotNull())
  {
    this->m_Segmentation->RemoveLabelAddedListener(mitk::MessageDelegate1<QmitkMultiLabelTreeModel, LabelValueType>(
      this, &QmitkMultiLabelTreeModel::OnLabelAdded));
    this->m_Segmentation->RemoveLabelModifiedListener(mitk::MessageDelegate1<QmitkMultiLabelTreeModel, LabelValueType>(
      this, &QmitkMultiLabelTreeModel::OnLabelModified));
    this->m_Segmentation->RemoveLabelRemovedListener(mitk::MessageDelegate1<QmitkMultiLabelTreeModel, LabelValueType>(
      this, &QmitkMultiLabelTreeModel::OnLabelRemoved));
    this->m_Segmentation->RemoveGroupAddedListener(mitk::MessageDelegate1<QmitkMultiLabelTreeModel, SpatialGroupIndexType>(
      this, &QmitkMultiLabelTreeModel::OnGroupAdded));
    this->m_Segmentation->RemoveGroupModifiedListener(mitk::MessageDelegate1<QmitkMultiLabelTreeModel, SpatialGroupIndexType>(
      this, &QmitkMultiLabelTreeModel::OnGroupModified));
    this->m_Segmentation->RemoveGroupRemovedListener(mitk::MessageDelegate1<QmitkMultiLabelTreeModel, SpatialGroupIndexType>(
      this, &QmitkMultiLabelTreeModel::OnGroupRemoved));
  }
  m_Observed = false;
}

void QmitkMultiLabelTreeModel::OnLabelAdded(LabelValueType labelValue)
{
  SpatialGroupIndexType groupIndex = 0;
  if (m_Segmentation->IsLabeInGroup(labelValue, groupIndex))
  {
    auto label = m_Segmentation->GetLabel(labelValue);
    if (nullptr == label) mitkThrow() << "Invalid internal state. Segmentation signaled the addition of an label that does not exist in the segmentation. Invalid label value:" << labelValue;
    if (label == m_Segmentation->GetExteriorLabel()) return;

    auto groupItem = GetGroupItem(groupIndex, this->m_RootItem.get());

    bool newLabelCreated = false;
    auto instanceItem = AddLabelToGroupTree(label, groupItem, newLabelCreated);

    if (newLabelCreated)
    {
      if (groupItem->m_childItems.size() == 1)
      { //first label added
        auto groupIndex = GetIndexByItem(groupItem, this);
        emit dataChanged(groupIndex, groupIndex);
      }
      else
      { //whole new label level added to group item
        auto groupIndex = GetIndexByItem(groupItem, this);
        this->beginInsertRows(groupIndex, instanceItem->ParentItem()->Row(), instanceItem->ParentItem()->Row());
        this->endInsertRows();
      }
    }
    else
    { // instance item was added to existing label item
      auto labelIndex = GetIndexByItem(instanceItem->ParentItem(), this);
      this->beginInsertRows(labelIndex, instanceItem->Row(), instanceItem->Row());
      this->endInsertRows();
    }
  }
  else
  {
    mitkThrow() << "Group less labels are not supported in the current implementation.";
  }
}

void QmitkMultiLabelTreeModel::OnLabelModified(LabelValueType labelValue)
{
  if (labelValue == m_Segmentation->GetExteriorLabel()->GetValue()) return;

  auto instanceItem = GetInstanceItem(labelValue, this->m_RootItem.get());

  if (nullptr == instanceItem)
  {
    mitkThrow() << "Internal invalid state. QmitkMultiLabelTreeModel recieved a LabelModified signal for a label that is not represented in the model. Invalid label: " << labelValue;
  }

  auto labelItem = instanceItem->ParentItem();

  if (labelItem->m_ClassName == instanceItem->GetLabel()->GetName())
  { //only the state of the label changed, but not its position in the model tree.

    auto index = GetIndexByItem(labelItem, this);
    emit dataChanged(index, index);
  }
  else
  { //the name of the label changed and thus its place in the model tree, delete the current item and add a new one
    this->OnLabelRemoved(labelValue);
    this->OnLabelAdded(labelValue);
  }
}

void QmitkMultiLabelTreeModel::OnLabelRemoved(LabelValueType labelValue)
{
  if (labelValue == m_Segmentation->GetExteriorLabel()->GetValue()) return;
  auto instanceItem = GetInstanceItem(labelValue, this->m_RootItem.get());

  if (nullptr == instanceItem) mitkThrow() << "Internal invalid state. QmitkMultiLabelTreeModel recieved a LabelRemoved signal for a label that is not represented in the model. Invalid label: " << labelValue;

  auto labelItem = instanceItem->ParentItem();

  if (labelItem->m_childItems.size() > 1)
  {
    auto labelIndex = GetIndexByItem(labelItem, this);
    this->beginRemoveRows(labelIndex, instanceItem->Row(), instanceItem->Row());
    labelItem->RemoveChild(instanceItem->Row());
    this->endRemoveRows();
  }
  else
  { //was the only instance of the label, therefor also remove the label node from the tree.
    auto groupItem = labelItem->ParentItem();
    auto groupIndex = GetIndexByItem(groupItem, this);
    this->beginRemoveRows(groupIndex, labelItem->Row(), labelItem->Row());
    groupItem->RemoveChild(labelItem->Row());
    this->endRemoveRows();
  }
}

void QmitkMultiLabelTreeModel::OnGroupAdded(SpatialGroupIndexType groupIndex)
{
  if (m_ShowGroups)
  {
    this->beginInsertRows(QModelIndex(), groupIndex, groupIndex);
    auto rootItem = m_RootItem.get();
    auto groupItem = new QmitkMultiLabelSegTreeItem(QmitkMultiLabelSegTreeItem::ItemType::Group, rootItem);
    rootItem->AppendChild(groupItem);
    this->GenerateInternalGroupTree(groupIndex, groupItem);
    this->endInsertRows();
  }
}

void QmitkMultiLabelTreeModel::OnGroupModified(SpatialGroupIndexType groupIndex)
{
  //currently not needed
}

void QmitkMultiLabelTreeModel::OnGroupRemoved(SpatialGroupIndexType groupIndex)
{
  if (m_ShowGroups)
  {
    this->beginRemoveRows(QModelIndex(), groupIndex, groupIndex);
    auto root = m_RootItem.get();
    root->RemoveChild(groupIndex);
    this->endRemoveRows();
  }
}

