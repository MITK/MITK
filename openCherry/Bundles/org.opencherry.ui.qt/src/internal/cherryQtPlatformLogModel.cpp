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

#include "mbilog.h"

namespace cherry {

/// mbilog integration - begin


void QtPlatformLogModel::addLogEntry(const mbilog::LogMessage &msg)
{
  this->beginInsertRows(QModelIndex(), m_Entries.size(), m_Entries.size());
  m_Entries.push_back(LogEntry(msg));
  this->endInsertRows();
}

/// mbilog integration - end

QtPlatformLogModel::QtPlatformLogModel(QObject* parent) : QAbstractTableModel(parent)
{
  Platform::GetEvents().logged += PlatformEventDelegate(this, &QtPlatformLogModel::addLogEntry);
  myBackend = new QtLogBackend(this);
}

QtPlatformLogModel::~QtPlatformLogModel()
{
  delete myBackend;
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
  return 8;
}

QVariant
QtPlatformLogModel::data(const QModelIndex& index, int role) const
{
  if (role == Qt::DisplayRole)
  {
    switch (index.column()) {
    case 0: return QVariant(m_Entries[index.row()].level);
    case 1: return QVariant(m_Entries[index.row()].category);
    case 2: return QVariant(m_Entries[index.row()].time);
    case 3:
    {
      const QString& source = m_Entries[index.row()].source;
      if (source.size() > 0)
        return QVariant(source);
      else return QVariant("Platform");
    }
    case 4: return QVariant(m_Entries[index.row()].message);
    case 5: return QVariant(m_Entries[index.row()].moduleName);
    case 6: return QVariant(m_Entries[index.row()].filePath);
    case 7: return QVariant(m_Entries[index.row()].lineNumber);
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
    case 0: return QVariant("Level");
    case 1: return QVariant("Category");
    case 2: return QVariant("Date");
    case 3: return QVariant("Source");
    case 4: return QVariant("Message");
    case 5: return QVariant("Module");
    case 6: return QVariant("File");
    case 7: return QVariant("Line");
    }
  }

  return QVariant();
}


}
