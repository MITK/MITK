

/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/



#include "QmitkTbssTableModel.h"

QmitkTbssTableModel::QmitkTbssTableModel(QObject *parent)
  : QAbstractTableModel(parent)
{
}

QmitkTbssTableModel::QmitkTbssTableModel(QList< QPair<QString, int> > pairs, QObject *parent)
  : QAbstractTableModel(parent)
{
  listOfPairs=pairs;
}

int QmitkTbssTableModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return listOfPairs.size();
}

int QmitkTbssTableModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return 2;
}

QVariant QmitkTbssTableModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (index.row() >= listOfPairs.size() || index.row() < 0)
    return QVariant();

  if (role == Qt::DisplayRole) {
    QPair<QString, int> pair = listOfPairs.at(index.row());

    if (index.column() == 0)
        return pair.first;
    else if (index.column() == 1)
        return pair.second;
  }
  return QVariant();
}

QVariant QmitkTbssTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    switch (section) {
      case 0:
          return tr("Group");

      case 1:
          return tr("Size");

      default:
          return QVariant();
    }
  }
  return QVariant();
}

bool QmitkTbssTableModel::insertRows(int position, int rows, const QModelIndex &index)
{
  Q_UNUSED(index);
  beginInsertRows(QModelIndex(), position, position+rows-1);

  for (int row=0; row < rows; row++) {
    QPair<QString, int> pair(" ", 0);
    listOfPairs.insert(position, pair);
  }

  endInsertRows();
  return true;
}

bool QmitkTbssTableModel::removeRows(int position, int rows, const QModelIndex &index)
{
  Q_UNUSED(index);
  beginRemoveRows(QModelIndex(), position, position+rows-1);

  for (int row=0; row < rows; ++row) {
    listOfPairs.removeAt(position);
  }

  endRemoveRows();
  return true;
}

bool QmitkTbssTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (index.isValid() && role == Qt::EditRole) {
    int row = index.row();

    QPair<QString, int> p = listOfPairs.value(row);

    if (index.column() == 0)
            p.first = value.toString();
    else if (index.column() == 1)
            p.second = value.toInt();
    else
      return false;

    listOfPairs.replace(row, p);
    emit(dataChanged(index, index));

    return true;
  }

  return false;
}

Qt::ItemFlags QmitkTbssTableModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::ItemIsEnabled;

  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

QList< QPair<QString, int> > QmitkTbssTableModel::getList()
{
  return listOfPairs;
}



