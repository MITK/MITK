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
#include <mitkDataStorage.h>
#include <mitkVector.h>
#include <mitkSurface.h>

#include <QAbstractTableModel>

class QmitkUSZonesDataModel : public QAbstractTableModel
{
  Q_OBJECT

signals:

public slots:

public:
  typedef std::vector<mitk::DataNode::Pointer> DataNodeVector;
  static const char* DataNodePropertySize;


  explicit QmitkUSZonesDataModel(QObject *parent = 0);

  void SetDataStorage(mitk::DataStorage::Pointer dataStorage, mitk::DataNode::Pointer baseNode);
  void AddNode(const mitk::DataNode*);
  void RemoveNode(const mitk::DataNode*);
  void ChangeNode(const mitk::DataNode*);

  virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
  virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
  virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
  virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;

  virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

  virtual bool insertRows ( int row, int count, const QModelIndex& parent = QModelIndex() );
  virtual bool removeRows ( int row, int count, const QModelIndex& parent = QModelIndex() );
  virtual bool removeRows ( int row, int count, const QModelIndex& parent, bool removeFromDataStorage );

protected:

  mitk::Surface::Pointer MakeSphere(const mitk::DataNode::Pointer dataNode, mitk::ScalarType radius) const;

  DataNodeVector m_ZoneNodes;

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::DataNode::Pointer m_BaseNode;
};

#endif // QMITKUSZONESDATAMODEL_H
