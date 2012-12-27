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

#ifndef QMITKPROPERTYTREEMODEL_H
#define QMITKPROPERTYTREEMODEL_H

#include <QAbstractItemModel>
#include <mitkPropertyList.h>

class QmitkPropertyTreeItem;

class QmitkPropertyTreeModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  QmitkPropertyTreeModel(QObject *parent = NULL);
  ~QmitkPropertyTreeModel();

  void SetProperties(mitk::PropertyList *properties);

  int columnCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QModelIndex index(int row, int column, const QModelIndex &parent) const;
  QModelIndex parent(const QModelIndex &child) const;
  int rowCount(const QModelIndex &parent) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

private:
  void CreateRootItem();

  mitk::PropertyList *m_Properties;
  QmitkPropertyTreeItem *m_RootItem;
};

#endif
