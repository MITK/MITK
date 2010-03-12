/*=========================================================================
 
 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "berryQtObjectTableModel.h"

#include <berryLog.h>
#include <berryDebugUtil.h>
#include <berryDebugBreakpointManager.h>

#include <QTimer>
#include <QIcon>

#include <cstring>

namespace berry
{

class DebugObjectListener: public IDebugObjectListener
{
public:

  DebugObjectListener(QtObjectTableModel* model) :
    model(model)
  {

  }

  Events::Types GetEventTypes() const
  {
    return Events::ALL;
  }

  void ObjectCreated(const Object* obj)
  {
    model->ObjectCreated(obj);
  }

  void ObjectDestroyed(const Object* obj)
  {
    model->ObjectDestroyed(obj);
  }

  void ObjectTracingChanged(unsigned int traceId, bool enabled = true,
      const Object*  /*obj*/ = 0)
  {
  }

  void SmartPointerCreated(unsigned int id, const Object* obj)
  {
    model->SmartPointerCreated(id, obj);
  }

  void SmartPointerDestroyed(unsigned int id, const Object* obj)
  {
    model->SmartPointerDestroyed(id, obj);
  }

private:

  QtObjectTableModel* model;
};

QtObjectTableModel::QtObjectTableModel(QObject* parent) :
  QAbstractItemModel(parent), objectListener(new DebugObjectListener(this))
{
  std::vector<const Object*> objects;
  DebugUtil::GetRegisteredObjects(objects);
  for (std::vector<const Object*>::const_iterator i = objects.begin(); i
      != objects.end(); ++i)
  {
    const char* name = (*i)->GetClassName();
    ObjectItem* classItem = 0;
    QListIterator<ObjectItem*> iter(indexData);
    bool classFound = false;
    while (iter.hasNext())
    {
      classItem = iter.next();
      if (std::strcmp(classItem->className, name) == 0)
      {
        classFound = true;
        break;
      }
    }

    ObjectItem* instanceItem = new ObjectItem(*i, 0);
    // get smartpointer ids
    std::list<unsigned int> spIds(DebugUtil::GetSmartPointerIDs(*i));
    for (std::list<unsigned int>::const_iterator spIdIter = spIds.begin(); spIdIter
        != spIds.end(); ++spIdIter)
    {
      ObjectItem* spItem = new ObjectItem((unsigned int) (*spIdIter),
          instanceItem);
      instanceItem->children.push_back(spItem);
    }

    if (classFound)
    {
      instanceItem->parent = classItem;
      classItem->children.push_back(instanceItem);
    }
    else
    {
      classItem = new ObjectItem(name);
      indexData.push_back(classItem);

      instanceItem->parent = classItem;
      classItem->children.push_back(instanceItem);
    }

  }

  QTimer* timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(UpdatePendingData()));
  timer->start(500);

  DebugUtil::AddObjectListener(objectListener);
}

QtObjectTableModel::~QtObjectTableModel()
{
  DebugUtil::RemoveObjectListener(objectListener);
}

QModelIndex QtObjectTableModel::index(int row, int column,
    const QModelIndex& parent) const
{
  if (parent.isValid())
  {
    if (parent.parent().isValid())
    {
      ObjectItem* data = static_cast<ObjectItem*> (parent.internalPointer());
      poco_assert(data->type == ObjectItem::INSTANCE)
;      return createIndex(row, column, data->children[row]);
    }
    else
    {
      ObjectItem* data = static_cast<ObjectItem*>(parent.internalPointer());
      poco_assert(data->type == ObjectItem::CLASS);
      return createIndex(row, column, data->children[row]);
    }
  }
  else
  return createIndex(row, column, indexData[row]);
}

QModelIndex QtObjectTableModel::parent(const QModelIndex& index) const
{
  ObjectItem* data = static_cast<ObjectItem*>(index.internalPointer());
  if (data->parent)
  {
    return createIndex(data->parent->children.indexOf(data), 0, data->parent);
  }
  else
  {
    return QModelIndex();
  }
}

int QtObjectTableModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid())
  {
    if (parent.parent().isValid())
    {
      if (parent.parent().parent().isValid()) // smart pointer parent
      return 0;

      // instance parent
      ObjectItem* data = static_cast<ObjectItem*>(parent.internalPointer());
      poco_assert(data->type == ObjectItem::INSTANCE);
      return data->children.size();
    }
    else
    {
      // class parent
      ObjectItem* data = static_cast<ObjectItem*>(parent.internalPointer());
      poco_assert(data->type == ObjectItem::CLASS);
      return data->children.size();
    }
  }
  else
  {
    // root
    return indexData.size();
  }
}

int QtObjectTableModel::columnCount(const QModelIndex&) const
{
  return 2;
}

QVariant QtObjectTableModel::data(const QModelIndex& index, int role) const
{
  if (role == Qt::DisplayRole)
  {
    if (index.column() == 0)
    {
      if (index.parent().isValid())
      {
        QModelIndex parentIndex = index.parent();
        if (parentIndex.parent().isValid())
        {
          ObjectItem* data = static_cast<ObjectItem*>(index.internalPointer());
          poco_assert(data->type == ObjectItem::SMARTPOINTER);
          return QVariant(QString("SmartPointer (id: ") + QString::number(data->spId) + QString(")"));
        }
        else
        {
          ObjectItem* data = static_cast<ObjectItem*>(index.internalPointer());
          poco_assert(data->type == ObjectItem::INSTANCE);
#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
          QString text = QString::number(data->obj->GetTraceId());
#else
          QString text;
#endif
          text += QString(" (References: ") + QString::number(data->obj->GetReferenceCount()) + QString(")");
          return QVariant(text);
        }
      }
      else
      {
        ObjectItem* data = static_cast<ObjectItem*>(index.internalPointer());
        poco_assert(data->type == ObjectItem::CLASS);
        return QVariant(QString(data->className) + " (" + QString::number(data->children.size()) + ")");
      }
    }
  }
  else if (role == Qt::CheckStateRole && index.column() == 1)
  {
    ObjectItem* data = static_cast<ObjectItem*>(index.internalPointer());
    if (data->type == ObjectItem::INSTANCE)
    {
      QVariant var = (DebugUtil::IsTraced(data->obj) ? Qt::Checked : Qt::Unchecked);
      return var;
    }
    else if (data->type == ObjectItem::CLASS)
    {
      QVariant var = (DebugUtil::IsTraced(data->className) ? Qt::Checked : Qt::Unchecked);
      return var;
    }
  }
  else if (role == Qt::DecorationRole && index.column() == 0)
  {
    ObjectItem* data = static_cast<ObjectItem*>(index.internalPointer());
    if (data->type == ObjectItem::INSTANCE)
    {
      QVariant var;
#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
      if (DebugUtil::GetBreakpointManager()->BreakAtObject(data->obj->GetTraceId()))
#else
      if(false)
#endif
      {
        var = QIcon(":/objectinspector/break-enabled.png");
      }
      return var;
    }
    else if (data->type == ObjectItem::SMARTPOINTER)
    {
      QVariant var;
      if (DebugUtil::GetBreakpointManager()->BreakAtSmartpointer(data->spId))
      {
        var = QIcon(":/objectinspector/break-enabled.png");
      }
      return var;
    }
  }
  else if (role == Qt::UserRole)
  {

    return QVariant::fromValue(index.internalPointer());
  }

  return QVariant();
}

QVariant QtObjectTableModel::headerData(int section,
    Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    switch (section)
    {
      case 0:
      {
        int count = 0;
        QListIterator<ObjectItem*> iter(indexData);
        while (iter.hasNext())
        {
          count += iter.next()->children.size();
        }

        return QVariant(QString("Instances (") + QString::number(count) + ")");
      }
      case 1:
      {
        return QVariant(QString("Trace"));
      }
    }
  }

  return QVariant();
}

Qt::ItemFlags QtObjectTableModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);

  ObjectItem* item = static_cast<ObjectItem*> (index.internalPointer());
  if ((item->type == ObjectItem::INSTANCE || item->type == ObjectItem::CLASS)
      && index.column() == 1)
  flags |= Qt::ItemIsUserCheckable;

  return flags;
}

bool QtObjectTableModel::setData(const QModelIndex &index,
    const QVariant &value, int role)
{
  if (index.isValid() && index.column() == 1 && role == Qt::CheckStateRole)
  {
    ObjectItem* item = static_cast<ObjectItem*> (index.internalPointer());

    if (value.toInt() == Qt::Checked)
    {
      if (item->type == ObjectItem::INSTANCE)
        DebugUtil::TraceObject(item->obj);
      else if (item->type == ObjectItem::CLASS)
        DebugUtil::TraceClass(item->className);
    }
    else
    {
      if (item->type == ObjectItem::INSTANCE)
        DebugUtil::StopTracing(item->obj);
      else if (item->type == ObjectItem::CLASS)
        DebugUtil::StopTracing(item->className);
    }

    emit dataChanged(index, index);
    return true;
  }

  return false;
}

void QtObjectTableModel::ResetData()
{
  indexData.clear();
  DebugUtil::ResetObjectSummary();
  QAbstractItemModel::reset();
}

void QtObjectTableModel::ObjectCreated(const Object* obj)
{
  // This method is called inside the berry::Object
  // constructor, hence we cannot reliably call the virtual
  // method berry::Object::GetClassName() to put the new
  // object under the right "class" parent. So add it to
  // a list of pending objects.

  ObjectItem* item = new ObjectItem(obj, 0);
  pendingData.push_back(item);
}

void QtObjectTableModel::UpdatePendingData()
{
  if (pendingData.empty())
  return;

  QListIterator<ObjectItem*> instanceIter(pendingData);
  while (instanceIter.hasNext())
  {
    ObjectItem* instanceItem = instanceIter.next();
    ObjectItem* classItem = 0;
    ObjectItem classSearchItem(instanceItem->obj->GetClassName());
    int classIndex = 0;
    classItem = FindObjectItem(classSearchItem, classIndex);
    if (!classItem)
    {
      classItem = new ObjectItem(instanceItem->obj->GetClassName());
      classItem->children.push_back(instanceItem);
      instanceItem->parent = classItem;

      classIndex = rowCount(QModelIndex());
      beginInsertRows(QModelIndex(), classIndex, classIndex);
      indexData.push_back(classItem);
      endInsertRows();
    }
    else
    {
      instanceItem->parent = classItem;

      QModelIndex classModelIndex = createIndex(classIndex, 0, classItem);
      int rowIndex = rowCount(classModelIndex);
      beginInsertRows(classModelIndex, rowIndex, rowIndex);
      classItem->children.push_back(instanceItem);
      endInsertRows();
    }
  }

  pendingData.clear();
}

void QtObjectTableModel::ObjectDestroyed(const Object* obj)
{
  ObjectItem searchItem(obj, 0);
  int index = 0;
  ObjectItem* item = FindObjectItem(searchItem, index);
  if (!item)
  {
    QMutableListIterator<ObjectItem*> pendingIter(pendingData);
    while (pendingIter.hasNext())
    {
      ObjectItem* pendingItem = pendingIter.next();
      if (pendingItem->obj == obj)
      {
        pendingIter.remove();
        delete pendingItem;
        break;
      }
    }
    return;
  }

  int parentIndex = indexData.indexOf(item->parent);
  beginRemoveRows(createIndex(parentIndex, 0, item->parent), index, index);
  item->parent->children.removeAt(index);
  endRemoveRows();

  if (item->parent->children.empty())
  {
    beginRemoveRows(QModelIndex(), parentIndex, parentIndex);
    indexData.removeAt(parentIndex);
    endRemoveRows();

    delete item->parent;
  }
  else
  {
    delete item;
  }
}

void QtObjectTableModel::SmartPointerCreated(unsigned int id, const Object* obj)
{
  ObjectItem searchInstance(obj, 0);
  int instanceIndex = 0;
  ObjectItem* instanceItem = FindObjectItem(searchInstance, instanceIndex);
  if (!instanceItem)
  {
    QListIterator<ObjectItem*> pendingIter(pendingData);
    while (pendingIter.hasNext())
    {
      ObjectItem* pendingItem = pendingIter.next();
      if (pendingItem->obj == obj)
      {
        pendingItem->children.push_back(new ObjectItem(id, pendingItem));
        break;
      }
    }
    return;
  }

  int itemIndex = instanceItem->children.size();
  beginInsertRows(createIndex(instanceIndex, 0, instanceItem), itemIndex,
      itemIndex);
  instanceItem->children.push_back(new ObjectItem(id, instanceItem));
  endInsertRows();
}

void QtObjectTableModel::SmartPointerDestroyed(unsigned int id, const Object* obj)
{
  ObjectItem searchSP(id, 0);
  int spIndex = 0;
  ObjectItem* spItem = FindObjectItem(searchSP, spIndex);
  if (!spItem)
  {
    QListIterator<ObjectItem*> pendingIter(pendingData);
    while (pendingIter.hasNext())
    {
      ObjectItem* pendingItem = pendingIter.next();
      if (pendingItem->obj == obj)
      {
        QMutableListIterator<ObjectItem*> spIter(pendingItem->children);
        while (spIter.hasNext())
        {
          spItem = spIter.next();
          if (spItem->spId == id)
          {
            spIter.remove();
            delete spItem;
            return;
          }
        }
        break;
      }
    }
    return;
  }

  int parentIndex = 0;
  ObjectItem searchInstance(obj, 0);
  ObjectItem* instanceItem = FindObjectItem(searchInstance, parentIndex);
  beginRemoveRows(createIndex(parentIndex, 0, instanceItem), spIndex, spIndex);
  instanceItem->children.removeAt(spIndex);
  delete spItem;
  endRemoveRows();
}

ObjectItem* QtObjectTableModel::FindObjectItem(
    const ObjectItem& item, int& index)
{
  switch (item.type)
  {
    case ObjectItem::CLASS:
    {
      QListIterator<ObjectItem*> i(indexData);
      index = 0;
      while (i.hasNext())
      {
        ObjectItem* next = i.next();
        if (std::strcmp(next->className, item.className) == 0)
        return next;
        ++index;
      }
      return 0;
    }
    case ObjectItem::INSTANCE:
    {
      QListIterator<ObjectItem*> i(indexData);
      while (i.hasNext())
      {
        ObjectItem* next = i.next();
        index = 0;
        QListIterator<ObjectItem*> childIter(next->children);
        while (childIter.hasNext())
        {
          ObjectItem* child = childIter.next();
          if (child->obj == item.obj)
          return child;
          ++index;
        }
      }
      return 0;
    }
    case ObjectItem::SMARTPOINTER:
    {
      QListIterator<ObjectItem*> classIter(indexData);
      while (classIter.hasNext())
      {
        ObjectItem* nextClass = classIter.next();
        QListIterator<ObjectItem*> instanceIter(nextClass->children);
        while (instanceIter.hasNext())
        {
          ObjectItem* nextInstance = instanceIter.next();
          index = 0;
          QListIterator<ObjectItem*> spIter(nextInstance->children);
          while (spIter.hasNext())
          {
            ObjectItem* nextSp = spIter.next();
            if (nextSp->spId == item.spId)
            return nextSp;
            ++index;
          }
        }

      }
      return 0;
    }
  }
  return 0;
}

}
