/*=========================================================================

Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "cherryQtPlatformLogModel.h"

#include "cherryPlatform.h"
#include "event/cherryPlatformEvents.h"

#include <Poco/Message.h>

namespace cherry {

QtPlatformLogModel::QtPlatformLogModel(QObject* parent)
 : QAbstractTableModel(parent)
{
  Platform::GetEvents().logged += PlatformEventDelegate(this, &QtPlatformLogModel::addLogEntry);
}

void
QtPlatformLogModel::addLogEntry(const PlatformEvent& event)
{

  const Poco::Message& entry = Poco::RefAnyCast<const Poco::Message>(*event.GetData());

  this->beginInsertRows(QModelIndex(), m_Entries.size(), m_Entries.size());


  m_Entries.push_back(LogEntry(entry.getText(), entry.getSource(),
      entry.getTime().epochTime()));

  this->endInsertRows();
}

int
QtPlatformLogModel::rowCount(const QModelIndex&) const
{
  return m_Entries.size();
}

int
QtPlatformLogModel::columnCount(const QModelIndex&) const
{
  return 3;
}

QVariant
QtPlatformLogModel::data(const QModelIndex& index, int role) const
{
  if (role == Qt::DisplayRole)
  {
    switch (index.column()) {
    case 0:
      return QVariant(m_Entries[index.row()].message);
    case 1:
    {
      const QString& source = m_Entries[index.row()].source;
      if (source.size() > 0)
        return QVariant(source);
      else return QVariant("Platform");
    }
    case 2:
      return QVariant(m_Entries[index.row()].time);
    }
  }

  return QVariant();
}

QVariant
QtPlatformLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    switch (section) {
    case 0: return QVariant("Message");
    case 1: return QVariant("Source");
    case 2: return QVariant("Date");
    }
  }

  return QVariant();
}


}
