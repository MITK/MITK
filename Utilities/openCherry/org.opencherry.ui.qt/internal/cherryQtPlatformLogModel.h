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

#include "org.opencherry.osgi/event/cherryPlatformEvent.h"

#include <vector>

namespace cherry {

class QtPlatformLogModel : public QAbstractTableModel
{
  
public:
  
  QtPlatformLogModel(QObject* parent = 0);
  
  int rowCount(const QModelIndex&) const;
  int columnCount(const QModelIndex&) const;
  QVariant data(const QModelIndex& index, int) const;
  
  QVariant headerData(int section, Qt::Orientation orientation, int) const;
  
  
private:
  
  void addLogEntry(const PlatformEvent& event);

  struct LogEntry {
    LogEntry(const std::string& msg, const std::string& src, std::time_t t)
    : message(msg.c_str()), source(src.c_str())
    { time.setTime_t(t); }

    QString message;
    QString source;
    QDateTime time;
  };

  std::vector<LogEntry> m_Entries;
};

}

#endif /*CHERRYQTPLATFORMLOGMODEL_H_*/
