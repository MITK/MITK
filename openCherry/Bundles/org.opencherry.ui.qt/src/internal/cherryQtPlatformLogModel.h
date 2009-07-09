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

#include "cherryLog.h"

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
  void addLogEntry(const PlatformEvent& event);

private:

  typedef MessageDelegate1<QtPlatformLogModel, const PlatformEvent&> PlatformEventDelegate;

  struct ExtendedLogMessage {
    mbilog::LogMessage message;
    clock_t time;
    int threadid;
    
    ExtendedLogMessage(const ExtendedLogMessage &src):message(src.message),time(src.time),threadid(src.threadid)
    {
    }
    
    ExtendedLogMessage(const mbilog::LogMessage &msg):message(msg),time(std::clock()),threadid(0)
    {
    }
    
    ExtendedLogMessage operator = (const ExtendedLogMessage& src)
    {
      return ExtendedLogMessage(src);
    }
    
  };
    
  class QtLogBackend : public mbilog::AbstractBackend
  {
    public:
    
      QtLogBackend(QtPlatformLogModel *_myModel)
      {
        myModel=_myModel;
        deactivated = false;
        mbilog::RegisterBackend(this);
        CHERRY_INFO << "openCherry mbilog backend registered";
      }
      
      ~QtLogBackend()
      {
        mbilog::UnregisterBackend(this);
      }
      
      void ProcessMessage(const mbilog::LogMessage &l )
      {
        if(!deactivated)
          myModel->addLogEntry(l);
      }
      
      void Deactivate()
      {
        deactivated=true;
      }                             
      
    private:
    
      QtPlatformLogModel *myModel;
      bool deactivated;
      
  } *myBackend;

  std::vector<ExtendedLogMessage> m_Entries;
  std::list<ExtendedLogMessage> *m_Active,*m_Pending;
  
  QMutex m_Mutex;
  
  signals:
  
    void signalFlushLogEntries();

  protected slots:
  
    void slotFlushLogEntries();
};

}

#endif /*CHERRYQTPLATFORMLOGMODEL_H_*/
