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
  QmitkTbssTableModel(QObject *parent=nullptr);
  QmitkTbssTableModel(QList< QPair<QString, int> > listofPairs, QObject *parent=nullptr);

  int rowCount(const QModelIndex &parent) const override;
  int columnCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole) override;
  bool insertRows(int position, int rows, const QModelIndex &index=QModelIndex()) override;
  bool removeRows(int position, int rows, const QModelIndex &index=QModelIndex()) override;
  QList< QPair<QString, int> > getList();

private:
  QList< QPair<QString, int> > listOfPairs;
};

#endif
