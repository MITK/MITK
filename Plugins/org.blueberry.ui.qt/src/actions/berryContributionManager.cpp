/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <berryLog.h>

#include "berryContributionManager.h"

#include "berryIContributionManagerOverrides.h"
#include "berryIContributionItem.h"

#include <berryQActionContributionItem.h>

namespace berry {

ContributionManager::~ContributionManager()
{
}

void ContributionManager::Add(QAction* action, const QString& id)
{
  Q_ASSERT_X(action, "nullcheck", "QAction must not be null");
  this->Add(IContributionItem::Pointer(new QActionContributionItem(action, id)));
}

void ContributionManager::Add(const SmartPointer<IContributionItem>& item)
{
  Q_ASSERT_X(item, "nullcheck", "Item must not be null");
  if (AllowItem(item.GetPointer()))
  {
    contributions.append(item);
    ItemAdded(item);
  }
}

void ContributionManager::AppendToGroup(const QString& groupName, QAction* action, const QString& id)
{
  AddToGroup(groupName, IContributionItem::Pointer(new QActionContributionItem(action, id)), true);
}

void ContributionManager::AppendToGroup(const QString& groupName, const SmartPointer<IContributionItem>& item)
{
  AddToGroup(groupName, item, true);
}

SmartPointer<IContributionItem> ContributionManager::Find(const QString& id) const
{
  QListIterator<IContributionItem::Pointer> e(contributions);
  while (e.hasNext())
  {
    IContributionItem::Pointer item = e.next();
    QString itemId = item->GetId();
    if (itemId.compare(id, Qt::CaseInsensitive) != 0)
    {
      return item;
    }
  }
  return IContributionItem::Pointer(nullptr);
}

QList<SmartPointer<IContributionItem> > ContributionManager::GetItems() const
{
  return contributions;
}

int ContributionManager::GetSize()
{
  return contributions.size();
}

SmartPointer<IContributionManagerOverrides> ContributionManager::GetOverrides()
{
  if (overrides.IsNull())
  {
    struct _DefaultOverride : public IContributionManagerOverrides {
      int GetEnabled(const IContributionItem* /*item*/) const override {
        return -1;
      }

      int GetVisible(const IContributionItem* /*item*/) const override {
        return -1;
      }
    };
    overrides = new _DefaultOverride;
  }
  return overrides;
}

int ContributionManager::IndexOf(const QString& id)
{
  int i = 0;
  foreach(IContributionItem::Pointer item, contributions)
  {
    QString itemId = item->GetId();
    if (item->GetId().compare(id, Qt::CaseInsensitive) == 0)
    {
      return i;
    }
    ++i;
  }
  return -1;
}

void ContributionManager::Insert(int index, const SmartPointer<IContributionItem>& item)
{
  if (index > contributions.size())
  {
    QString msg = QString("inserting ")  + item->GetId() + " at " + QString::number(index);
    throw std::invalid_argument(msg.toStdString());
  }
  if (AllowItem(item.GetPointer()))
  {
    contributions.insert(index, item);
    ItemAdded(item);
  }
}

void ContributionManager::InsertAfter(const QString& ID, const SmartPointer<IContributionItem>& item)
{
  IContributionItem::Pointer ci = Find(ID);
  if (ci.IsNull())
  {
    throw ctkInvalidArgumentException(QString("can't find ID") + ID);
  }
  int ix = contributions.indexOf(ci);
  if (ix >= 0)
  {
    // BERRY_INFO << "insert after: " << ix;
    if (AllowItem(item.GetPointer()))
    {
      contributions.insert(ix + 1, item);
      ItemAdded(item);
    }
  }
}

void ContributionManager::InsertBefore(const QString& ID, const SmartPointer<IContributionItem>& item)
{
  IContributionItem::Pointer ci = Find(ID);
  if (ci.IsNull())
  {
    throw ctkInvalidArgumentException(QString("can't find ID ") + ID);
  }
  int ix = contributions.indexOf(ci);
  if (ix >= 0)
  {
    // BERRY_INFO << "insert before: " << ix;
    if (AllowItem(item.GetPointer()))
    {
      contributions.insert(ix, item);
      ItemAdded(item);
    }
  }
}

bool ContributionManager::IsDirty() const
{
  if (isDirty)
  {
    return true;
  }
  if (HasDynamicItems())
  {
    foreach (IContributionItem::Pointer item, contributions)
    {
      if (item->IsDirty())
      {
        return true;
      }
    }
  }
  return false;
}

bool ContributionManager::IsEmpty() const
{
  return contributions.empty();
}

void ContributionManager::MarkDirty()
{
  SetDirty(true);
}

void ContributionManager::PrependToGroup(const QString& groupName, const SmartPointer<IContributionItem>& item)
{
  AddToGroup(groupName, item, false);
}

SmartPointer<IContributionItem> ContributionManager::Remove(const QString& ID)
{
  IContributionItem::Pointer ci = Find(ID);
  if (ci.IsNull())
  {
    return ci;
  }
  return Remove(ci);
}

SmartPointer<IContributionItem> ContributionManager::Remove(const SmartPointer<IContributionItem>& item)
{
  if (contributions.removeAll(item))
  {
    ItemRemoved(item);
    return item;
  }
  return IContributionItem::Pointer(nullptr);
}

void ContributionManager::RemoveAll()
{
  QList<IContributionItem::Pointer> items = GetItems();
  contributions.clear();
  foreach (IContributionItem::Pointer item, items)
  {
    ItemRemoved(item);
  }
  dynamicItems = 0;
  MarkDirty();
}

bool ContributionManager::ReplaceItem(const QString& identifier,
                 const SmartPointer<IContributionItem>& replacementItem)
{
  if (identifier.isNull())
  {
    return false;
  }

  const int index = IndexOf(identifier);
  if (index < 0)
  {
    return false; // couldn't find the item.
  }

  // Remove the old item.
  const IContributionItem::Pointer oldItem = contributions.at(index);
  ItemRemoved(oldItem);

  // Add the new item.
  contributions.replace(index, replacementItem);
  ItemAdded(replacementItem); // throws NPE if (replacementItem == null)

  // Go through and remove duplicates.
  QMutableListIterator<IContributionItem::Pointer> i(contributions);
  i.toBack();
  while (i.hasPrevious())
  {
    IContributionItem::Pointer item = i.previous();
    if ((item.IsNotNull()) && (identifier == item->GetId()))
    {
//      if (Policy.TRACE_TOOLBAR) {
//        System.out
//            .println("Removing duplicate on replace: " + identifier);
//      }
      i.remove();
      ItemRemoved(item);
    }
  }

  return true; // success
}

void ContributionManager::SetOverrides(const SmartPointer<IContributionManagerOverrides>& newOverrides)
{
  overrides = newOverrides;
}

ContributionManager::ContributionManager()
  : isDirty(true), dynamicItems(0)
{
  // Do nothing.
}

bool ContributionManager::AllowItem(IContributionItem* /*itemToAdd*/)
{
  return true;
}

void ContributionManager::DumpStatistics()
{
  int size = contributions.size();

  BERRY_INFO << this->ToString();
  BERRY_INFO << "   Number of elements: " << size;
  int sum = 0;
  for (int i = 0; i < size; i++)
  {
    if (contributions.at(i)->IsVisible())
    {
      ++sum;
    }
  }
  BERRY_INFO << "   Number of visible elements: " << sum;
  BERRY_INFO << "   Is dirty: " << IsDirty();
}

bool ContributionManager::HasDynamicItems() const
{
  return (dynamicItems > 0);
}

int ContributionManager::IndexOf(const SmartPointer<IContributionItem>& item) const
{
  return contributions.indexOf(item);
}

void ContributionManager::ItemAdded(const SmartPointer<IContributionItem>& item)
{
  item->SetParent(this);
  MarkDirty();
  if (item->IsDynamic())
  {
    dynamicItems++;
  }
}

void ContributionManager::ItemRemoved(const SmartPointer<IContributionItem>& item)
{
  item->SetParent(nullptr);
  MarkDirty();
  if (item->IsDynamic())
  {
    dynamicItems--;
  }
}

void ContributionManager::SetDirty(bool dirty)
{
  isDirty = dirty;
}

void ContributionManager::InternalSetItems(const QList<SmartPointer<IContributionItem> >& items)
{
  contributions.clear();
  for (int i = 0; i < items.size(); ++i)
  {
    if (AllowItem(items[i].GetPointer()))
    {
      contributions.append(items[i]);
    }
  }
}

void ContributionManager::AddToGroup(const QString& groupName, const SmartPointer<IContributionItem>& item,
                                     bool append)
{
  QMutableListIterator<IContributionItem::Pointer> items(contributions);
  for (int i = 0; items.hasNext(); ++i)
  {
    IContributionItem::Pointer o = items.next();
    if (o->IsGroupMarker())
    {
      QString id = o->GetId();
      if (id.compare(groupName, Qt::CaseInsensitive) == 0)
      {
        ++i;
        if (append)
        {
          for (; items.hasNext(); ++i)
          {
            IContributionItem::Pointer ci = items.next();
            if (ci->IsGroupMarker())
            {
              break;
            }
          }
        }
        if (AllowItem(item.GetPointer()))
        {
          contributions.insert(i, item);
          ItemAdded(item);
        }
        return;
      }
    }
  }
  throw ctkInvalidArgumentException(QString("Group not found: ") + groupName);
}

}
