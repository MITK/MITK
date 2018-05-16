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

#include "QmitkAlgorithmListModel.h"

#include "mapAlgorithmProfileHelper.h"

QmitkAlgorithmListModel::QmitkAlgorithmListModel(QObject *parent) : QAbstractTableModel(parent)
{
}

void QmitkAlgorithmListModel::SetAlgorithms(::map::deployment::DLLDirectoryBrowser::DLLInfoListType algList)
{
  emit beginResetModel();

  m_AlgList = algList;

  emit endResetModel();
};

int QmitkAlgorithmListModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return m_AlgList.size();
}

int QmitkAlgorithmListModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return 0;

  return 4;
}

QVariant QmitkAlgorithmListModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  QVariant result;

  if (index.row() < static_cast<int>(m_AlgList.size()))
  {
    const ::map::deployment::DLLInfo *info = m_AlgList[index.row()].GetPointer();

    if (Qt::DisplayRole == role)
    {
      switch (index.column())
      {
        case 0:
          result = QVariant(info->getAlgorithmUID().getName().c_str());
          break;
        case 1:
          result = QVariant(info->getAlgorithmUID().getNamespace().c_str());
          break;
        case 2:
          result = QVariant(info->getAlgorithmUID().getVersion().c_str());
          break;
        case 3:
          std::stringstream descriptionString;
          ::map::algorithm::profile::ValueListType keys =
            ::map::algorithm::profile::getKeywords(info->getAlgorithmProfileStr());
          for (::map::algorithm::profile::ValueListType::const_iterator keyPos = keys.begin(); keyPos != keys.end();
               ++keyPos)
          {
            if (keyPos != keys.begin())
            {
              descriptionString << "; ";
            }
            descriptionString << *keyPos;
          }
          descriptionString << "</p>";
          result = QVariant(descriptionString.str().c_str());
          break;
      }
    }
    else if (Qt::UserRole == role)
    {
      result = QVariant(index.row());
    }
  }

  return result;
}

Qt::ItemFlags QmitkAlgorithmListModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);

  return flags;
}

QVariant QmitkAlgorithmListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((Qt::DisplayRole == role) && (Qt::Horizontal == orientation))
  {
    if (section == 0)
    {
      return QVariant("Name");
    }
    else if (section == 1)
    {
      return QVariant("Namespace");
    }
    else if (section == 2)
    {
      return QVariant("Version");
    }
    else if (section == 3)
    {
      return QVariant("Keywords");
    }
  }
  return QVariant();
}
