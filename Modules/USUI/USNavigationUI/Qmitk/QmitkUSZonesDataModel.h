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

#ifndef QMITKUSZONESDATAMODEL_H
#define QMITKUSZONESDATAMODEL_H

#include <mitkCommon.h>
#include <mitkDataNode.h>
#include <QAbstractTableModel>

class QmitkUSZonesDataModel : public QAbstractTableModel
{
  Q_OBJECT

signals:

public slots:

public:
  explicit QmitkUSZonesDataModel(QObject *parent = 0);

  virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
  virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
  virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
  virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;

  virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

  virtual bool insertRows ( int row, int count, const QModelIndex& parent = QModelIndex() );
  virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );

protected:
  std::vector<mitk::DataNode::Pointer> m_ZoneNodes;
};

#endif // QMITKUSZONESDATAMODEL_H
