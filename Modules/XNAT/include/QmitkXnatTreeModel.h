/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkXnatTreeModel_h
#define QmitkXnatTreeModel_h

// CTK includes
#include <ctkXnatTreeModel.h>

// MITK includes
#include "MitkXNATExports.h"

namespace mitk
{
  class DataNode;
}

class MITKXNAT_EXPORT QmitkXnatTreeModel : public ctkXnatTreeModel
{
  Q_OBJECT

public:
  QmitkXnatTreeModel();

  QVariant data(const QModelIndex &index, int role) const override;

  bool dropMimeData(
    const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

  using QAbstractItemModel::supportedDropActions;
  virtual Qt::DropActions supportedDropActions();

  Qt::ItemFlags flags(const QModelIndex &index) const override;

  ctkXnatObject *GetXnatObjectFromUrl(const QString &);

  void fetchMore(const QModelIndex &index) override;

  QModelIndexList match(
    const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const override;

signals:
  void Error(const QModelIndex &idx);
  void ResourceDropped(const QList<mitk::DataNode *> &, ctkXnatObject *, const QModelIndex &);

private:
  ctkXnatObject *InternalGetXnatObjectFromUrl(const QString &xnatObjectType, const QString &url, ctkXnatObject *parent);
};

#endif
