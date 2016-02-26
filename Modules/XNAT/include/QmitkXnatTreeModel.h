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

#ifndef QMITKXNATTREEMODEL_H
#define QMITKXNATTREEMODEL_H

// CTK includes
#include <ctkXnatTreeModel.h>

// MITK includes
#include "MitkXNATExports.h"

namespace mitk {
class DataNode;
}

class MITKXNAT_EXPORT QmitkXnatTreeModel : public ctkXnatTreeModel
{
  Q_OBJECT

public:
  QmitkXnatTreeModel();

  virtual QVariant data(const QModelIndex& index, int role) const;

  virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

  using QAbstractItemModel::supportedDropActions;
  virtual Qt::DropActions supportedDropActions();

  virtual Qt::ItemFlags flags(const QModelIndex &index) const;

  ctkXnatObject* GetXnatObjectFromUrl(const QString&);

  signals:
  void ResourceDropped(const QList<mitk::DataNode*>&, ctkXnatObject*, const QModelIndex&);

private:
    ctkXnatObject *InternalGetXnatObjectFromUrl(const QString &xnatObjectType, const QString &url, ctkXnatObject *parent);

};

#endif // QMITKXNATTREEMODEL_H
