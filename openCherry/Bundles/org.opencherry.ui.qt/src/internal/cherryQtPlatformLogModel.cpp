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

#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>

#include <Poco/Message.h>
#include "mbilog.h"
#include <QTimer>

namespace cherry {

/// mbilog integration - begin


void QtPlatformLogModel::addLogEntry(const mbilog::LogMessage &msg)
{
  m_Mutex.lock();
  mbilog::BackendCout::FormatSmart(msg);
  m_Active->push_back(msg);
  m_Mutex.unlock();

  emit signalFlushLogEntries();
}

void QtPlatformLogModel::slotFlushLogEntries()
{
  m_Mutex.lock();
  std::list<mbilog::LogMessage> *tmp=m_Active;
  m_Active=m_Pending; m_Pending=tmp;
  m_Mutex.unlock();

  int num = m_Pending->size();
  
  if(num>0)
  {
    std::cout << "flushing " << num << " entries\n";
  
    int row = m_Entries.size();
    this->beginInsertRows(QModelIndex(), row, row+num-1);
    do {
      m_Entries.push_back(LogEntry(m_Pending->front()));
      m_Pending->pop_front();
    } while(--num);
    this->endInsertRows();
  }
}

/// mbilog integration - end

QtPlatformLogModel::QtPlatformLogModel(QObject* parent) : QAbstractTableModel(parent)
{
  m_Active=new std::list<mbilog::LogMessage>;
  m_Pending=new std::list<mbilog::LogMessage>;
  connect(this, SIGNAL(signalFlushLogEntries()), this, SLOT( slotFlushLogEntries() ), Qt::QueuedConnection );  
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

  int row = m_Entries.size();

  this->beginInsertRows(QModelIndex(), row, row);
  m_Entries.push_back(LogEntry(entry.getText(), entry.getSource(),entry.getTime().epochTime()));
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
    case 0: {
      std::stringstream ss;
      ss << std::setw(7) << std::setprecision(3) << std::fixed << ((double)m_Entries[index.row()].time)/CLOCKS_PER_SEC;
      return QVariant(QString(ss.str().c_str()));
    }
    case 1: return QVariant(m_Entries[index.row()].level);
    case 2: return QVariant(m_Entries[index.row()].message);
    case 3: return QVariant(m_Entries[index.row()].category);
    case 4: return QVariant(m_Entries[index.row()].moduleName);
    case 5: return QVariant(m_Entries[index.row()].function);
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
    case 0: return QVariant("Time");
    case 1: return QVariant("Level");
    case 2: return QVariant("Message");
    case 3: return QVariant("Category");
    case 4: return QVariant("Module");
    case 5: return QVariant("Function");
    case 6: return QVariant("File");
    case 7: return QVariant("Line");
    }
  }

  return QVariant();
}


}
