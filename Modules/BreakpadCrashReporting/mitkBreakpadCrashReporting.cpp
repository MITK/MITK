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

#include "mitkLogMacros.h"


#if defined(Q_OS_WIN)
#include <windows.h>
#include <tchar.h>
#include "client/windows/crash_generation/client_info.h"
#include "client/windows/crash_generation/crash_generation_server.h"
#include "client/windows/handler/exception_handler.h"
#include "client/windows/common/ipc_protocol.h"

#elif defined(Q_OS_MAC)
#include <client/mac/handler/exception_handler.h>
#include <sys/wait.h>
#include <fcntl.h>
#elif defined(Q_OS_LINUX)
#include <client/linux/handler/exception_handler.h>
#include <sys/wait.h>
#include <fcntl.h>
#endif

#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QProcess>

#include <itksys/SystemTools.hxx>

static bool breakpadOnceConnected = false;            // indicates a server having had at least one client connection
static int  breakpadNumberOfConnections = 0;          // current number of connected clients
static int  numberOfConnectionAttemptsPerformed = 1;  // number of performed re-connect attempts of a crash client

mitk::BreakpadCrashReporting::BreakpadCrashReporting()
{
  m_CrashServer         = NULL;
  m_ExceptionHandler    = NULL;
  
  m_NamedPipeString       = "\\\\.\\pipe\\MitkCrashServices\\MitkBasedApplication";
  m_CrashDumpPath         = QDir(QApplication::instance()->applicationDirPath()).absolutePath();
  m_CrashDumpPath.append("/CrashDumps/");

  m_NumberOfConnectionAttempts = 3;
  m_ReconnectDelay             = 300;

  m_CrashReportingServerExecutable = QDir(QApplication::instance()->applicationDirPath()).absolutePath().append("/CrashReportingServer.exe");
}

mitk::BreakpadCrashReporting::~BreakpadCrashReporting()
{
  if (m_ExceptionHandler) 
  {
    delete m_ExceptionHandler;
  }
  if (m_CrashServer) 
  {
    delete m_CrashServer;
  }
}
//This function gets called in the event of a crash.
bool ShowDumpResults(const wchar_t* dump_path,
                     const wchar_t* minidump_id,
                     void* context,
                     EXCEPTION_POINTERS* exinfo,
                     MDRawAssertionInfo* assertion,
                     bool succeeded) 
{
  /*
    NO STACK USE, NO HEAP USE IN THIS FUNCTION
    Creating QString's, using qDebug, etc. - everything is crash-unfriendly.
  */
  //QMessageBox::information( NULL, "Application problem", "The application encountered an error. \n\n A detailed error report may have been written - contact support.", QMessageBox::Ok );

  return succeeded;
}


void mitk::BreakpadCrashReporting::InitializeClientHandler(bool connectToCrashGenerationServer)
{  
  google_breakpad::CustomClientInfo custom_info;

  /* This is needed for CRT to not show dialog for invalid param
   failures and instead let the code handle it.*/
  _CrtSetReportMode(_CRT_ASSERT, 0);

  const wchar_t* pipe; 
  if(connectToCrashGenerationServer)
  {
     pipe = (const wchar_t*)m_NamedPipeString.utf16();
     MITK_INFO << "Initializing Breakpad Crash Handler, connecting to named pipe: " << m_NamedPipeString.toStdString().c_str() << "\n";
  }
  else
  {
     pipe = (const wchar_t*) L"";
     MITK_INFO << "Initializing Breakpad Crash Handler, connecting to named pipe: ";
  }

#ifdef _MSC_VER  // http://stackoverflow.com/questions/5625884/conversion-of-stdwstring-to-qstring-throws-linker-error
  std::wstring dump_path = std::wstring((const wchar_t *)m_CrashDumpPath.utf16());
#else
  std::wstring dump_path = m_CrashDumpPath.toStdWString();
#endif

  

  m_ExceptionHandler = new google_breakpad::ExceptionHandler(
#if defined(Q_OS_WIN)
                                dump_path,
#else
                                m_CrashDumpPath.toStdString(),
#endif
                                 NULL,
                                 ShowDumpResults,
                                 NULL,
                                 google_breakpad::ExceptionHandler::HANDLER_ALL,
                                 MiniDumpNormal, //see DbgHelp.h
                                 pipe,
                                 &custom_info);

  if(connectToCrashGenerationServer)
  {
    if(!m_ExceptionHandler->IsOutOfProcess()) 
    { // we want to connect to a server but connection handler did not connect to OOP server.
      
      MITK_INFO << "Initializing Breakpad Crash Handler: connection attempt to crash report server failed. Server started?";
      
      if(numberOfConnectionAttemptsPerformed < this->m_NumberOfConnectionAttempts)
      {
        itksys::SystemTools::Delay(m_ReconnectDelay);         //sleep a little
        numberOfConnectionAttemptsPerformed++;
        InitializeClientHandler(connectToCrashGenerationServer);
      }
      else
      {
        MITK_INFO << "Initializing Breakpad Crash Handler: connection attempt to crash report server failed - will proceed with in process handler.";
      }
    }
  }
}

static void _cdecl ShowClientConnected(void* context,
                                       const google_breakpad::ClientInfo* client_info) 
{ // callback of the crash generation server on client connect
  MITK_INFO << "Breakpad Client connected: " << client_info->pid();

  breakpadOnceConnected = true; // static variables indicate server shutdown after usage
  breakpadNumberOfConnections++;
}

static void _cdecl ShowClientCrashed(void* context,
                                     const google_breakpad::ClientInfo* client_info,
                                     const std::wstring* dump_path) 
{ // callback of the crash generation server on client crash

  MITK_INFO << "Breakpad Client request dump: " << client_info->pid();

  // we may add some log info here along the dump file
  google_breakpad::CustomClientInfo custom_info = client_info->GetCustomInfo();
}

static void _cdecl ShowClientExited(void* context,
                                    const google_breakpad::ClientInfo* client_info) 
{ // callback of the crash generation server on client exit
  MITK_INFO << "Breakpad Client exited :" << client_info->pid();

  // we'd like to shut down server if there is no further client connected, 
  // but no access to private server members in this callback
  breakpadNumberOfConnections--;
  if(breakpadNumberOfConnections == 0 && breakpadOnceConnected)
  {
    MITK_INFO << "Breakpad Server: no more client connections. Shuting down...";
    exit(0);
  }
}

bool mitk::BreakpadCrashReporting::StartCrashServer(bool lauchOutOfProcessExecutable) 
{
  
  if (m_CrashServer)
  { // Do not create another instance of the server.
    MITK_INFO << "Crash Server object already generated.";    
    return true;
  }

  if(lauchOutOfProcessExecutable)
  { // spawn process and launch CrashReportingServer executable
    QString tmpPipeString           = m_NamedPipeString;
    QString tmpCrashDumpPathString  = m_CrashDumpPath;
    QStringList arguments;
    arguments << tmpPipeString.prepend('"').append('"');
    arguments << tmpCrashDumpPathString.prepend('"').append('"');
    bool success =  QProcess::startDetached( m_CrashReportingServerExecutable, arguments);
    
    return success;
  }
  else
  { // directly open up server instance in this thread
    return InitializeServer();
  }
}

bool mitk::BreakpadCrashReporting::InitializeServer()
{
  QDir myDir;
  myDir.mkpath(m_CrashDumpPath); // Assure directory is created.
  
#ifdef _MSC_VER  // http://stackoverflow.com/questions/5625884/conversion-of-stdwstring-to-qstring-throws-linker-error
  std::wstring dump_path = std::wstring((const wchar_t *)m_CrashDumpPath.utf16());
#else
  std::wstring dump_path = m_CrashDumpPath.toStdWString();
#endif

  m_CrashServer = new google_breakpad::CrashGenerationServer((const wchar_t*)m_NamedPipeString.utf16(),
                                           NULL,
                                           ShowClientConnected, // connect callback
                                           NULL,
                                           ShowClientCrashed, // dump callback
                                           NULL,
                                           ShowClientExited, // exit callback
                                           NULL,
                                           NULL,
                                           NULL,
                                           true,
                                           &dump_path);

  if (!m_CrashServer->Start()) 
  {
    MITK_ERROR << "Unable to start google breakpad server.";
    delete m_CrashServer;
    m_CrashServer = NULL;
    return false;
  }
  else
  {
    MITK_INFO << "Google breakpad server started.";
    return true;
  }
}

bool mitk::BreakpadCrashReporting::RequestDump() 
{
  if(this->m_ExceptionHandler != NULL)
  {
    if(m_ExceptionHandler->WriteMinidump())
    {
      MITK_INFO << "Breakpad Crash Reporting: Successfully requested a minidump.";
      return true;
    }
    else
    {
      MITK_INFO << "Breakpad Crash Reporting: Requested of minidump failed.";
      return false;
    }
  }
  return false;
}

void mitk::BreakpadCrashReporting::StopCrashServer() 
{
  delete m_CrashServer;
  m_CrashServer = NULL;
}

void mitk::BreakpadCrashReporting::CrashAppForTestPurpose() 
{
  //printf(NULL);

  //derived derived;

  int* x = 0;
  *x = 1;
}

int mitk::BreakpadCrashReporting::GetNumberOfConnections() 
{
  return breakpadNumberOfConnections;
}



