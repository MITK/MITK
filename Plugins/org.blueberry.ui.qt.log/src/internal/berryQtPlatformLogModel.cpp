/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtPlatformLogModel.h"
#include "berryQtLogPlugin.h"

#include <mitkLogLevel.h>

#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>

#include <QTimer>
#include <QIcon>
#include <QModelIndex>
#include <QDebug>

namespace berry {

const QString QtPlatformLogModel::Error = QString("Error");
const QString QtPlatformLogModel::Warn = QString("Warning");
const QString QtPlatformLogModel::Fatal = QString("Fatal");
const QString QtPlatformLogModel::Info = QString("Info");
const QString QtPlatformLogModel::Debug = QString("Debug");

void QtPlatformLogModel::slotFlushLogEntries()
{
  m_Mutex.lock();
  QList<ExtendedLogMessage> *tmp=m_Active;
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

void QtPlatformLogModel::addLogEntry(const mitk::LogMessage &msg)
{
  m_Mutex.lock();
  //mitk::LogBackendCout::FormatSmart(msg); FormatSmart is not static any more. So commented out this statement. Todo: fix
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
QtPlatformLogModel::addLogEntry(const ctkPluginFrameworkEvent& event)
{
  auto level = mitk::LogLevel::Info;
  if (event.getType() == ctkPluginFrameworkEvent::PLUGIN_ERROR)
  {
    level = mitk::LogLevel::Error;
  }
  else if (event.getType() == ctkPluginFrameworkEvent::FRAMEWORK_WAIT_TIMEDOUT ||
           event.getType() == ctkPluginFrameworkEvent::PLUGIN_WARNING)
  {
    level = mitk::LogLevel::Warn;
  }

  mitk::LogMessage msg(level,"n/a",-1,"n/a");

  QString str;
  QDebug dbg(&str);
  dbg << event;
  msg.Message = str.toStdString();
  //msg.moduleName = event.getPlugin()->getSymbolicName().toStdString();

  addLogEntry(msg);
}

QtPlatformLogModel::QtPlatformLogModel(QObject* parent) : QAbstractTableModel(parent),
m_ShowAdvancedFiels(false),
m_ShowCategory(true)
{
  m_Active=new QList<ExtendedLogMessage>;
  m_Pending=new QList<ExtendedLogMessage>;
  connect(this, SIGNAL(signalFlushLogEntries()), this, SLOT( slotFlushLogEntries() ), Qt::QueuedConnection );
  QtLogPlugin::GetInstance()->GetContext()->connectFrameworkListener(this, SLOT(addLogEntry(ctkPluginFrameworkEvent)));
  myBackend = new QtLogBackend(this);
}

QtPlatformLogModel::~QtPlatformLogModel()
{
  disconnect(this, SIGNAL(signalFlushLogEntries()), this, SLOT( slotFlushLogEntries() ));
  QtLogPlugin::GetInstance()->GetContext()->disconnectFrameworkListener(this);

  // dont delete and unregister backend, only deactivate it to avoid thread synchronization issues cause mitk::UnregisterBackend is not threadsafe
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

    LogEntry(const mitk::LogMessage &msg)
    {
      message = msg.Message.c_str();


      filePath = msg.filePath;

      std::stringstream out;
      out << msg.LineNumber;
      lineNumber = out.str().c_str();

      moduleName = msg.ModuleName;
      category = msg.Category.c_str();
      function = msg.FunctionName;

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

      if( msg->message.Level == mitk::LogLevel::Error )
        file = ":/org_blueberry_ui_qt_log/error.png";
      else if( msg->message.Level == mitk::LogLevel::Warn )
        file = ":/org_blueberry_ui_qt_log/warning.png";
      else if( msg->message.Level == mitk::LogLevel::Debug )
        file = ":/org_blueberry_ui_qt_log/debug.png";
      else if( msg->message.Level == mitk::LogLevel::Fatal )
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
    else //!m_ShowAdvancedFiels, m_ShowCategory is not handled separately because it only activates case 2
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
