

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



#include "QmitkTbssMetaTableModel.h"

QmitkTbssMetaTableModel::QmitkTbssMetaTableModel(QObject *parent)
  : QAbstractTableModel(parent)
{
}

QmitkTbssMetaTableModel::QmitkTbssMetaTableModel(QList< QPair<QString, QString> > pairs, QObject *parent)
  : QAbstractTableModel(parent)
{
  listOfPairs=pairs;
}

int QmitkTbssMetaTableModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return listOfPairs.size();
}

int QmitkTbssMetaTableModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return 2;
}

QVariant QmitkTbssMetaTableModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (index.row() >= listOfPairs.size() || index.row() < 0)
    return QVariant();

  if (role == Qt::DisplayRole) {
    QPair<QString, QString> pair = listOfPairs.at(index.row());

    if (index.column() == 0)
        return pair.first;
    else if (index.column() == 1)
        return pair.second;
  }
  return QVariant();
}

QVariant QmitkTbssMetaTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    switch (section) {
      case 0:
          return tr("Image");

      case 1:
          return tr("Filename");

      default:
          return QVariant();
    }
  }
  return QVariant();
}

bool QmitkTbssMetaTableModel::insertRows(int position, int rows, const QModelIndex &index)
{
  Q_UNUSED(index);
  beginInsertRows(QModelIndex(), position, position+rows-1);

  for (int row=0; row < rows; row++) {
    QPair<QString, QString> pair("Image ", "Filename");
    listOfPairs.insert(position, pair);
  }

  endInsertRows();
  return true;
}

bool QmitkTbssMetaTableModel::removeRows(int position, int rows, const QModelIndex &index)
{
  Q_UNUSED(index);
  beginRemoveRows(QModelIndex(), position, position+rows-1);

  for (int row=0; row < rows; ++row) {
    listOfPairs.removeAt(position);
  }

  endRemoveRows();
  return true;
}

bool QmitkTbssMetaTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (index.isValid() && role == Qt::EditRole) {
    int row = index.row();

    QPair<QString, QString> p = listOfPairs.value(row);

    if (index.column() == 0)
            p.first = value.toString();
    else if (index.column() == 1)
            p.second = value.toString();
    else
      return false;

    listOfPairs.replace(row, p);
    emit(dataChanged(index, index));

    return true;
  }

  return false;
}

Qt::ItemFlags QmitkTbssMetaTableModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::ItemIsEnabled;

  return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

QList< QPair<QString, QString> > QmitkTbssMetaTableModel::getList()
{
  return listOfPairs;
}



