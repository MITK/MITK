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

#include "mitkBreakpadCrashReporting.h"

#include <mitkLog.h>
#include "mitkLogMacros.h"

#include <QtCore>
#include <QObject>
#include <QApplication>
#include <QDateTime>
#include <QTimer>
#include <QObject>

// Simple server process for out-of-process crash reporting. By default this server will log to the executables directory, or 
// to the crash dump path if provided properly.
// Arguments: 
// NamedPipeName:       Quotated string of the named pipe to communicate with the client.
// FolderForCrashDumps: Quotated string of the folder to save minidump files in the event of a crash.

static mitk::BreakpadCrashReporting* myBreakpad;

/*class Timer : public QTimer {
  Q_OBJECT
public:
  explicit Timer(QObject *parent = 0) : QTimer(parent) {
    connect(this, SIGNAL(timeout()), this, SLOT(CheckForServerShutdown()));
  }
private slots:
  void CheckForServerShutdown() 
  {
    MITK_INFO << "mitk Crash Reporting Server check for server shutdown.";
    MITK_INFO << "number of active connections: " << QString(myBreakpad->GetNumberOfConnections()).toStdString().c_str();
  }
};

#include "main.moc"*/

int main(int argc, char* argv[])
{
  QApplication qtapplication( argc, argv );

  QString folderForCrashDumps ="";
  QString namedPipeName = "";

  if (argc != 3)
  {
    MITK_WARN << "mitk Crash Reporting Server using default arguments"; 
  }
  else
  {
    namedPipeName       = argv[1];
    folderForCrashDumps = argv[2];
    namedPipeName.remove('"');
    folderForCrashDumps.remove('"');
  }
  try
  {
    // set up logging to file
    mitk::LoggingBackend::Register();
    QString logfile;
    if(folderForCrashDumps.isEmpty())
      logfile = QCoreApplication::applicationDirPath().append("/");
    else
      logfile = folderForCrashDumps.append("/");

    QDateTime date(QDateTime::currentDateTime());
    QString dateString(date.toString("yyyy-MM-dd_hh-mm-ss"));
    logfile.append(dateString);
    logfile.append("-CrashReportingServer-");

    QString pidString( QString::number(QCoreApplication::applicationPid()) );
    logfile.append(pidString);
    QString logfile2 = logfile + QString(".log");

    MITK_INFO << "** Logging to " << logfile2.toStdString() << std::endl;
    mitk::LoggingBackend::SetLogFile( logfile2.toLocal8Bit().constData() );

    // init breakpad server    
    myBreakpad = new mitk::BreakpadCrashReporting();

    if(!namedPipeName.isEmpty())
    {  
      MITK_INFO << "Using arg[1] as named pipe name";
      myBreakpad->SetNamedPipeName(namedPipeName);
    }
    if(!folderForCrashDumps.isEmpty())
    {  
      MITK_INFO << "Using arg[2] as crash dump path";
      myBreakpad->SetCrashDumpPath(folderForCrashDumps);
    }

    MITK_INFO << "NamedPipeName: " << myBreakpad->m_NamedPipeString.toStdString().c_str() << "\n";
    MITK_INFO << "FolderForCrashDumps: " << myBreakpad->m_CrashDumpPath.toStdString().c_str() << "\n";
    
    if(myBreakpad->StartCrashServer(false)) // false = we are already in a separate process.
    {
      MITK_INFO << "mitk Crash Reporting Server successfully started."; 
    }
    else
    {
      MITK_WARN << "Error during start of mitk Crash Reporting Server. Shutting down.";
      exit(2); // a server might be already running.
    }
  
    //Timer* shutdownTimer = new Timer();
    // shutdownTimer->start(3000);

    qtapplication.exec();
  }
  catch(...)
  {
    MITK_WARN << "mitk Crash Reporting Server exception caught, shutdown.";
    exit(2);
  }



  MITK_INFO << "mitk Crash Reporting Server shuting down."; 

}
