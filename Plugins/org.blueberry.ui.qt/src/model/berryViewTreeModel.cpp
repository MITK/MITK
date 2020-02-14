/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryViewTreeModel.h"

#include "berryIViewRegistry.h"
#include "berryIViewCategory.h"
#include "berryIWorkbench.h"
#include "berryIWorkbenchWindow.h"
#include "berryIWorkbenchPage.h"

#include "internal/intro/berryIntroConstants.h"
#include "internal/berryKeywordRegistry.h"

#include <QIcon>
#include <QBrush>

namespace berry {

// --------------------------- Tree Item Classes ---------------------------

struct ViewTreeItem;

bool CompareViewTreeItem(ViewTreeItem* item1, ViewTreeItem* item2);

struct ViewTreeItem
{
  ViewTreeItem(ViewTreeModel* model)
    : m_parent(nullptr)
    , m_model(model)
  {}

  virtual ~ViewTreeItem()
  {
    QList<ViewTreeItem*> children = m_children;
    if (m_parent) m_parent->removeChild(this);
    qDeleteAll(children);
  }

  virtual QVariant data(int role)
  {
    if (role == ViewTreeModel::Keywords)
    {
      if (m_keywordCache.isEmpty())
      {
        m_keywordCache = QStringList(keywordLabels().toList());
      }
      return m_keywordCache;
    }
    return QVariant();
  }

  virtual Qt::ItemFlags flags() const
  {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }

  virtual QSet<QString> keywordLabels() const
  {
    return QSet<QString>();
  }

  void appendChild(ViewTreeItem* child)
  {
    m_children.push_back(child);
    child->m_parent = this;
    qSort(m_children.begin(), m_children.end(), CompareViewTreeItem);
  }

  void removeChild(ViewTreeItem* child)
  {
    m_children.removeAll(child);
  }

  QList<ViewTreeItem*> takeChildren()
  {
    QList<ViewTreeItem*> children = m_children;
    m_children.clear();
    return children;
  }

  ViewTreeItem* childItem(int row) const
  {
    if (row < 0 || row >= m_children.size()) return nullptr;
    return m_children.at(row);
  }

  ViewTreeItem* parentItem() const
  {
    return m_parent;
  }

  int childCount() const
  {
    return m_children.size();
  }

  int row() const
  {
    if (m_parent) return m_parent->rowIndex(this);
    return 0;
  }

  int rowIndex(const ViewTreeItem* child) const
  {
    return m_children.indexOf(const_cast<ViewTreeItem*>(child));
  }

  QList<ViewTreeItem*> m_children;
  ViewTreeItem* m_parent;
  ViewTreeModel* m_model;

private:

  QStringList m_keywordCache;

};

bool CompareViewTreeItem(ViewTreeItem* item1, ViewTreeItem* item2)
{
  return item1->data(Qt::DisplayRole).toString() < item2->data(Qt::DisplayRole).toString();
}

struct RootTreeItem : ViewTreeItem
{
  RootTreeItem(ViewTreeModel* model) : ViewTreeItem(model) {}

  QVariant data(int /*role*/) override { return QVariant(); }
};

struct DescriptorTreeItem : ViewTreeItem
{
  DescriptorTreeItem(ViewTreeModel* model, IViewDescriptor::Pointer descriptor, ViewTreeItem* parent = nullptr);

  QVariant data(int role) override;

protected:

  QSet<QString> keywordLabels() const override;

  IViewDescriptor::Pointer m_descriptor;
};

struct CategoryTreeItem : ViewTreeItem
{
  CategoryTreeItem(ViewTreeModel* model, IViewCategory::Pointer category, ViewTreeItem* parent = nullptr);

  QVariant data(int role) override;

  Qt::ItemFlags flags() const override;

protected:

  QSet<QString> keywordLabels() const override;

  /**
   * Removes the temporary intro view from the list so that it cannot be activated except through
   * the introduction command.
   */
  void RemoveIntroView(QList<IViewDescriptor::Pointer>& list);

private:

  void CreateChildren();

  IViewCategory::Pointer m_category;

};

// --------------------------- Tree Model Classes ---------------------------

struct ViewTreeModel::Impl
{
  Impl(const IWorkbenchWindow* window)
    : window(window)
    , viewRegistry(*window->GetWorkbench()->GetViewRegistry())
  {

  }

  const IWorkbenchWindow* window;
  IViewRegistry& viewRegistry;

  QScopedPointer<RootTreeItem> rootItem;
};

ViewTreeModel::ViewTreeModel(const IWorkbenchWindow* window, QObject* parent)
  : QAbstractItemModel(parent)
  , d(new Impl(window))
{
  d->rootItem.reset(new RootTreeItem(this));

  QList<CategoryTreeItem*> categoryItems;

  QList<IViewCategory::Pointer> categories = d->viewRegistry.GetCategories();
  for (auto category : categories)
  {
    if (category->GetViews().isEmpty()) continue;
    CategoryTreeItem* categoryItem = new CategoryTreeItem(this, category);
    if (categoryItem->childCount() == 0)
    {
      delete categoryItem;
    }
    else
    {
      categoryItems.push_back(categoryItem);
    }
  }


  // if there is only one category, return it's children directly
  if (categoryItems.size() == 1)
  {
    QList<ViewTreeItem*> items = categoryItems.front()->takeChildren();
    for (auto item : items)
    {
      d->rootItem->appendChild(item);
    }
    qDeleteAll(categoryItems);
  }
  else
  {
    for (auto category : categoryItems)
    {
      d->rootItem->appendChild(category);
    }
  }
}

ViewTreeModel::~ViewTreeModel()
{

}

QVariant ViewTreeModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) return QVariant();

  return static_cast<ViewTreeItem*>(index.internalPointer())->data(role);
}

Qt::ItemFlags ViewTreeModel::flags(const QModelIndex& index) const
{
  if (!index.isValid()) return nullptr;

  return static_cast<ViewTreeItem*>(index.internalPointer())->flags();
}

QVariant ViewTreeModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
  if (role == Qt::DisplayRole && section == 0)
  {
    return "View";
  }
  return QVariant();
}

QModelIndex ViewTreeModel::index(int row, int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
  {
    return QModelIndex();
  }

  ViewTreeItem* parentItem = nullptr;
  if (!parent.isValid())
  {
    parentItem = d->rootItem.data();
  }
  else
  {
    parentItem = static_cast<ViewTreeItem*>(parent.internalPointer());
  }

  ViewTreeItem* childItem = parentItem->childItem(row);
  if (childItem)
  {
    return createIndex(row, column, childItem);
  }
  return QModelIndex();
}

QModelIndex ViewTreeModel::parent(const QModelIndex& child) const
{
  if (!child.isValid())
  {
    return QModelIndex();
  }

  ViewTreeItem* childItem = static_cast<ViewTreeItem*>(child.internalPointer());
  ViewTreeItem* parentItem = childItem->parentItem();

  if (parentItem == d->rootItem.data())
  {
    return QModelIndex();
  }
  return createIndex(parentItem->row(), 0, parentItem);
}

int ViewTreeModel::rowCount(const QModelIndex& parent) const
{
  ViewTreeItem* parentItem = nullptr;
  if (parent.column() > 0) return 0;

  if (!parent.isValid())
  {
    parentItem = d->rootItem.data();
  }
  else
  {
    parentItem = static_cast<ViewTreeItem*>(parent.internalPointer());
  }
  return parentItem->childCount();
}

int ViewTreeModel::columnCount(const QModelIndex& /*parent*/) const
{
  return 1;
}

const IWorkbenchWindow*ViewTreeModel::GetWorkbenchWindow() const
{
  return d->window;
}

// --------------------------- DescriptorTreeItem  ---------------------------

DescriptorTreeItem::DescriptorTreeItem(ViewTreeModel* model, IViewDescriptor::Pointer descriptor, ViewTreeItem* parent)
  : ViewTreeItem(model)
  , m_descriptor(descriptor)
{
  if (parent) parent->appendChild(this);
}

QVariant DescriptorTreeItem::data(int role)
{
  if (role == Qt::DisplayRole)
  {
    return m_descriptor->GetLabel();
  }
  else if (role == Qt::DecorationRole)
  {
    return m_descriptor->GetImageDescriptor();
  }
  else if (role == Qt::ForegroundRole)
  {
    IWorkbenchPage::Pointer page = this->m_model->GetWorkbenchWindow()->GetActivePage();
    if (page.IsNotNull())
    {
      if (page->FindViewReference(m_descriptor->GetId()).IsNotNull())
      {
        return QBrush(QColor(Qt::gray));
      }
    }
  }
  else if (role == ViewTreeModel::Description)
  {
    return m_descriptor->GetDescription();
  }
  else if (role == ViewTreeModel::Id)
  {
    return m_descriptor->GetId();
  }
  return ViewTreeItem::data(role);
}

QSet<QString> DescriptorTreeItem::keywordLabels() const
{
  KeywordRegistry* registry = KeywordRegistry::GetInstance();
  QStringList ids = m_descriptor->GetKeywordReferences();
  QSet<QString> keywords;
  keywords.insert(m_descriptor->GetLabel());
  for(auto id : ids)
  {
    QString label = registry->GetKeywordLabel(id);
    for (auto keyword : label.split(' ', QString::SkipEmptyParts))
    {
      keywords.insert(keyword);
    }
  }
  return keywords;
}

// --------------------------- CategoryTreeItem ---------------------------

CategoryTreeItem::CategoryTreeItem(ViewTreeModel* model, IViewCategory::Pointer category, ViewTreeItem* parent)
  : ViewTreeItem(model)
  , m_category(category)
{
  if (parent) parent->appendChild(this);
  this->CreateChildren();
}

QVariant CategoryTreeItem::data(int role)
{
  if (role == Qt::DisplayRole)
  {
    return m_category->GetLabel();
  }
  else if (role == Qt::DecorationRole)
  {
    return QIcon::fromTheme("folder");
  }
  else if (role == ViewTreeModel::Id)
  {
    return m_category->GetId();
  }
  return ViewTreeItem::data(role);
}

Qt::ItemFlags CategoryTreeItem::flags() const
{
  return Qt::ItemIsEnabled;
}

QSet<QString> CategoryTreeItem::keywordLabels() const
{
  QSet<QString> keywords;
  for(auto child : this->m_children)
  {
    for (auto keyword : child->data(ViewTreeModel::Keywords).toStringList())
    {
      keywords.insert(keyword);
    }
  }
  return keywords;
}

void CategoryTreeItem::CreateChildren()
{
  auto viewDescriptors = m_category->GetViews();
  RemoveIntroView(viewDescriptors);
  for(auto viewDescriptor : viewDescriptors)
  {
    new DescriptorTreeItem(this->m_model, viewDescriptor, this);
  }
}

void CategoryTreeItem::RemoveIntroView(QList<IViewDescriptor::Pointer>& list)
{
  for (auto view = list.begin(); view != list.end();)
  {
    if ((*view)->GetId() == IntroConstants::INTRO_VIEW_ID)
    {
      view = list.erase(view);
    }
    else ++view;
  }
}

}
