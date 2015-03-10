/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYQTOBJECTTABLEMODEL_H_
#define BERRYQTOBJECTTABLEMODEL_H_

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QList>

#include <berryIDebugObjectListener.h>

#include "berryObjectItem.h"

namespace berry
{

class QtObjectTableModel: public QAbstractItemModel
{
  Q_OBJECT

public:

  QtObjectTableModel(QObject* parent = 0);
  ~QtObjectTableModel();

  QModelIndex index(int row, int column, const QModelIndex& parent =
      QModelIndex()) const;
  QModelIndex parent(const QModelIndex& index) const;

  int rowCount(const QModelIndex&) const;
  int columnCount(const QModelIndex&) const;
  QVariant data(const QModelIndex& index, int) const;

  QVariant headerData(int section, Qt::Orientation orientation, int) const;

  Qt::ItemFlags flags(const QModelIndex& index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);

  void ResetData();

protected:

  void ObjectCreated(const Object* obj);
  void ObjectDestroyed(const Object* obj);

  void SmartPointerCreated(unsigned int id, const Object* obj);
  void SmartPointerDestroyed(unsigned int id, const Object* obj);

private slots:

  void UpdatePendingData();

private:

  ObjectItem* FindObjectItem(const ObjectItem& item, int& index);

  QList<ObjectItem*> indexData;
  QList<ObjectItem*> pendingData;

  QScopedPointer<IDebugObjectListener> objectListener;
  friend class DebugObjectListener;

};

}

#endif /* BERRYQTOBJECTTABLEMODEL_H_ */
