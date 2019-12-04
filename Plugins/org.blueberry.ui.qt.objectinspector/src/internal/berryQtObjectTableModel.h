/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  QtObjectTableModel(QObject* parent = nullptr);
  ~QtObjectTableModel() override;

  QModelIndex index(int row, int column, const QModelIndex& parent =
      QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;

  int rowCount(const QModelIndex&) const override;
  int columnCount(const QModelIndex&) const override;
  QVariant data(const QModelIndex& index, int) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int) const override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role) override;

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
