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



#ifndef QmitkTbssTableModel_h
#define QmitkTbssTableModel_h


#include <QAbstractTableModel>

#include <QPair>
#include <QList>
#include <QObject>

class QmitkTbssTableModel : public QAbstractTableModel
{

  //Q_OBJECT

public:
  QmitkTbssTableModel(QObject *parent=0);
  QmitkTbssTableModel(QList< QPair<QString, int> > listofPairs, QObject *parent=0);

  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
  bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex());
  bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex());
  QList< QPair<QString, int> > getList();

private:
  QList< QPair<QString, int> > listOfPairs;
};

#endif
