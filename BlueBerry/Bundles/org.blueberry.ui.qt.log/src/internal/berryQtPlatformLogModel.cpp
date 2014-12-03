/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include "berryQtPlatformLogModel.h"

#include "berryPlatform.h"
#include "event/berryPlatformEvents.h"

#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>

#include <Poco/Message.h>
#include "berryLog.h"
#include <QTimer>
#include <QIcon>
#include <QModelIndex>

namespace berry {

const QString QtPlatformLogModel::Error = QString("Error");
const QString QtPlatformLogModel::Warn = QString("Warning");
const QString QtPlatformLogModel::Fatal = QString("Fatal");
const QString QtPlatformLogModel::Info = QString("Info");
const QString QtPlatformLogModel::Debug = QString("Debug");

void QtPlatformLogModel::slotFlushLogEntries()
{
  m_Mutex.lock();
  std::list<ExtendedLogMessage> *tmp=m_Active;
  m_Active=m_Pending; m_Pending=tmp;
  m_Mutex.unlock();

  int num = static_cast<int>(m_Pending->size());

  if (num > 0)
  {
    int row = static_cast<int>(m_Entries.size());
    this->beginInsertRows(QModelIndex(), row, row+num-1);
    do {
      m_Entries.push_back(m_Pending->front());
      m_Pending->pop_front();



    } while(--num);
    this->endInsertRows();
  }
}

void QtPlatformLogModel::addLogEntry(const mbilog::LogMessage &msg)
{
  m_Mutex.lock();
  //mbilog::BackendCout::FormatSmart(msg); FormatSmart is not static any more. So commented out this statement. Todo: fix
  m_Active->push_back(ExtendedLogMessage(msg));
  m_Mutex.unlock();

  emit signalFlushLogEntries();

}

void
QtPlatformLogModel::SetShowAdvancedFiels( bool showAdvancedFiels )
{
  if( m_ShowAdvancedFiels != showAdvancedFiels )
  {
    m_ShowAdvancedFiels = showAdvancedFiels;
    this->beginResetModel();
    this->endResetModel();
  }
}

void QtPlatformLogModel::SetShowCategory( bool showCategory )
{
  if( m_ShowCategory != showCategory )
  {
    m_ShowCategory = showCategory;
    this->beginResetModel();
    this->endResetModel();
  }
}

void
QtPlatformLogModel::addLogEntry(const PlatformEvent& event)
{
  const Poco::Message& entry = Poco::RefAnyCast<const Poco::Message>(*event.GetData());
  mbilog::LogMessage msg(mbilog::Info,"n/a",-1,"n/a");
  msg.message += entry.getText();
  msg.category = "BlueBerry."+entry.getSource();
  msg.moduleName = "n/a";
  addLogEntry(msg);
}

QtPlatformLogModel::QtPlatformLogModel(QObject* parent) : QAbstractTableModel(parent),
m_ShowAdvancedFiels(false),
m_ShowCategory(true)
{
  m_Active=new std::list<ExtendedLogMessage>;
  m_Pending=new std::list<ExtendedLogMessage>;
  connect(this, SIGNAL(signalFlushLogEntries()), this, SLOT( slotFlushLogEntries() ), Qt::QueuedConnection );
  Platform::GetEvents().logged += PlatformEventDelegate(this, &QtPlatformLogModel::addLogEntry);
  myBackend = new QtLogBackend(this);
}

QtPlatformLogModel::~QtPlatformLogModel()
{
  disconnect(this, SIGNAL(signalFlushLogEntries()), this, SLOT( slotFlushLogEntries() ));

  // dont delete and unregister backend, only deactivate it to avoid thread syncronization issues cause mbilog::UnregisterBackend is not threadsafe
  // will be fixed.
  //  delete myBackend;
  //  delete m_Active;
  //  delete m_Pending;
  m_Mutex.lock();
  myBackend->Deactivate();
  m_Mutex.unlock();
}

// QT Binding

int
QtPlatformLogModel::rowCount(const QModelIndex&) const
{
  return static_cast<int>(m_Entries.size());
}

int
QtPlatformLogModel::columnCount(const QModelIndex&) const
{
  int returnValue = 2;
  if( m_ShowAdvancedFiels ) returnValue += 7;
  if( m_ShowCategory ) returnValue += 1;
  return returnValue;
}
          /*
  struct LogEntry {
    LogEntry(const std::string& msg, const std::string& src, std::time_t t)
    : message(msg.c_str()), moduleName(src.c_str()),time(std::clock())
    {
    }

    QString message;
    clock_t time;

    QString level;
    QString filePath;
    QString lineNumber;
    QString moduleName;
    QString category;
    QString function;

    LogEntry(const mbilog::LogMessage &msg)
    {
      message = msg.message.c_str();


      filePath = msg.filePath;

      std::stringstream out;
      out << msg.lineNumber;
      lineNumber = out.str().c_str();

      moduleName = msg.moduleName;
      category = msg.category.c_str();
      function = msg.functionName;

      time=std::clock();
    }
  };        */


QVariant QtPlatformLogModel::data(const QModelIndex& index, int role) const
{
  const ExtendedLogMessage *msg = &m_Entries[index.row()];

  if (role == Qt::DisplayRole)
    {
    switch (index.column())
      {
      case 0:
        if (m_ShowAdvancedFiels) return msg->getTime();
        else return msg->getLevel();
      case 1:
        if (m_ShowAdvancedFiels) return msg->getLevel();
        else return msg->getMessage();
      case 2:
        if (m_ShowAdvancedFiels) return msg->getMessage();
        else return msg->getCategory();
      case 3:
        if (m_ShowAdvancedFiels && m_ShowCategory) return msg->getCategory();
        else if (m_ShowAdvancedFiels && !m_ShowCategory) return msg->getModuleName();
        else break;
      case 4:
        if (m_ShowAdvancedFiels && m_ShowCategory) return msg->getModuleName();
        else if (m_ShowAdvancedFiels && !m_ShowCategory) return msg->getFunctionName();
        else break;
      case 5:
        if (m_ShowAdvancedFiels && m_ShowCategory) return msg->getFunctionName();
        else if (m_ShowAdvancedFiels && !m_ShowCategory) return msg->getPath();
        else break;
      case 6:
        if (m_ShowAdvancedFiels && m_ShowCategory) return msg->getPath();
        else if (m_ShowAdvancedFiels && !m_ShowCategory) return msg->getLine();
        else break;
      case 7:
        if (m_ShowAdvancedFiels && m_ShowCategory) return msg->getLine();
        else break;
      }
    }

  else if(  role == Qt::DecorationRole )
  {
    if ( (m_ShowAdvancedFiels && index.column()==1)
      || (!m_ShowAdvancedFiels && index.column()==0) )
    {
      QString file ( ":/org_blueberry_ui_qt_log/information.png" );

      if( msg->message.level == mbilog::Error )
        file = ":/org_blueberry_ui_qt_log/error.png";
      else if( msg->message.level == mbilog::Warn )
        file = ":/org_blueberry_ui_qt_log/warning.png";
      else if( msg->message.level == mbilog::Debug )
        file = ":/org_blueberry_ui_qt_log/debug.png";
      else if( msg->message.level == mbilog::Fatal )
        file = ":/org_blueberry_ui_qt_log/fatal.png";

      QIcon icon(file);
      return QVariant(icon);

    }
  }


  return QVariant();
}

QVariant
QtPlatformLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    if( m_ShowAdvancedFiels && m_ShowCategory )
    {
      switch (section)
      {
        case 0: return QVariant(" Time ");
        case 1: return QVariant(" Level ");
        case 2: return QVariant(" Message ");
        case 3: return QVariant(" Category ");
        case 4: return QVariant(" Module ");
        case 5: return QVariant(" Function ");
        case 6: return QVariant(" File ");
        case 7: return QVariant(" Line ");
      }
    }
    else if (m_ShowAdvancedFiels && !m_ShowCategory)
    {
      switch (section)
      {
        case 0: return QVariant(" Time ");
        case 1: return QVariant(" Level ");
        case 2: return QVariant(" Message ");
        case 3: return QVariant(" Module ");
        case 4: return QVariant(" Function ");
        case 5: return QVariant(" File ");
        case 6: return QVariant(" Line ");
      }
    }
    else //!m_ShowAdvancedFiels, m_ShowCategory is not handled seperately because it only activates case 2
    {
      switch (section)
      {
        case 0: return QVariant(" Level ");
        case 1: return QVariant(" Message ");
        case 2: return QVariant(" Category ");
      }
    }
  }

  return QVariant();
}

QVariant QtPlatformLogModel::ExtendedLogMessage::getTime() const
    {
    std::stringstream ss;
    std::locale C("C");
    ss.imbue(C);
    ss << std::setw(7) << std::setprecision(3) << std::fixed << ((double)this->time)/CLOCKS_PER_SEC;
    return QVariant(QString(ss.str().c_str()));
    }

QString QtPlatformLogModel::GetDataAsString()
    {
    QString returnValue("");

    for (int message=0; message<this->rowCount(QModelIndex()); message++)
      {
      for (int column=0; column<this->columnCount(QModelIndex()); column++)
        {
        returnValue += " " + this->data(this->index(message,column),Qt::DisplayRole).toString();
        }
      returnValue += "\n";
      }

    return returnValue;
    }


}
