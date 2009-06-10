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

namespace cherry {

class QtPlatformLogModel : public QAbstractTableModel
{

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
    : message(msg.c_str()), source(src.c_str())
    { time.setTime_t(t); }

    QString message;
    QString source;
    QDateTime time;

    QString level;
    QString filePath;
    QString lineNumber;
    QString moduleName;
    QString category;
  
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
      std::string s = out.str(); 
      lineNumber = s.c_str();
      
      moduleName = msg.moduleName;
      category = msg.category.c_str();
      source = msg.functionName;
      
      time.setTime_t(std::time(NULL)); 
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
};

}

#endif /*CHERRYQTPLATFORMLOGMODEL_H_*/
