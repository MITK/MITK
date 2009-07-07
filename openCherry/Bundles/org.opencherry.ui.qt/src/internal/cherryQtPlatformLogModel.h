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

#ifndef CHERRYQTPLATFORMLOGMODEL_H_
#define CHERRYQTPLATFORMLOGMODEL_H_

#include <QtCore/QAbstractTableModel>
#include <QtCore/QDateTime>

#include "event/cherryPlatformEvent.h"
#include "cherryMessage.h"

#include <vector>
#include <ctime>
#include <sstream>

#include "mbilog.h"

#include <QMutex>


namespace cherry {

class QtPlatformLogModel : public QAbstractTableModel
{
  Q_OBJECT

public:

  QtPlatformLogModel(QObject* parent = 0);
  ~QtPlatformLogModel();

  int rowCount(const QModelIndex&) const;
  int columnCount(const QModelIndex&) const;
  QVariant data(const QModelIndex& index, int) const;

  QVariant headerData(int section, Qt::Orientation orientation, int) const;

  void addLogEntry(const mbilog::LogMessage &msg);

private:

  typedef MessageDelegate1<QtPlatformLogModel, const PlatformEvent&> PlatformEventDelegate;

  void addLogEntry(const PlatformEvent& event);

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
      
      switch(msg.level)
      {
        case mbilog::Info:
          level="INFO";
          break;
      
        case mbilog::Warn:
          level="WARN";
          break;
          
        case mbilog::Error:
          level="ERROR";
          break;
          
        case mbilog::Fatal:
          level="FATAL";
          break;
          
        case mbilog::Debug:
          level="DEBUG";
          break;
      }
                                                           
      filePath = msg.filePath;
            
      std::stringstream out;
      out << msg.lineNumber;
      lineNumber = out.str().c_str();
      
      moduleName = msg.moduleName;
      category = msg.category.c_str();
      function = msg.functionName;
      
      time=std::clock(); 
    }
  };
    
  class QtLogBackend : public mbilog::AbstractBackend
  {
    public:
    
      QtLogBackend(QtPlatformLogModel *_myModel)
      {
        myModel=_myModel;
        mbilog::RegisterBackend(this);
      }
      
      ~QtLogBackend()
      {
        mbilog::UnregisterBackend(this);
      }
      
      void ProcessMessage(const mbilog::LogMessage &l )
      {
        myModel->addLogEntry(l);
      }                             
      
    private:
    
      QtPlatformLogModel *myModel;
  } *myBackend;

  std::vector<LogEntry> m_Entries;
  std::list<mbilog::LogMessage> *m_Active,*m_Pending;
  
  QMutex m_Mutex;
  
  signals:
  
    void signalFlushLogEntries();

  protected slots:
  
    void slotFlushLogEntries();
  
  
};

}

#endif /*CHERRYQTPLATFORMLOGMODEL_H_*/
