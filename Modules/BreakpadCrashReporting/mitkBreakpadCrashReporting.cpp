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


#ifdef Q_OS_WIN

#include <windows.h>
#include <tchar.h>
#include "client/windows/crash_generation/client_info.h"
#include "client/windows/crash_generation/crash_generation_server.h"
#include "client/windows/handler/exception_handler.h"
#include "client/windows/common/ipc_protocol.h"

#elif Q_OS_MAC

#include <client/mac/handler/exception_handler.h>
#include <sys/wait.h>
#include <fcntl.h>

#elif __gnu_linux__

#include <client/linux/crash_generation/crash_generation_server.h>
#include <client/linux/crash_generation/client_info.h>
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
:server_fd(-1)
,client_fd(-1)
{
  m_CrashServer         = NULL;
  m_ExceptionHandler    = NULL;

  m_NamedPipeString       = "\\\\.\\pipe\\MitkCrashServices\\MitkBasedApplication";
  m_CrashDumpPath         = QDir(QApplication::instance()->applicationDirPath()).absolutePath();
  m_CrashDumpPath.append("/CrashDumps/"); // is created if it does not exist

  m_NumberOfConnectionAttempts = 3;
  m_ReconnectDelay             = 300;

  // TODO platform specific
#ifdef WIN32
  m_CrashReportingServerExecutable = QDir(QApplication::instance()->applicationDirPath()).absolutePath().append("/CrashReportingServer.exe");
#else
  m_CrashReportingServerExecutable = QDir(QApplication::instance()->applicationDirPath()).absolutePath().append("/CrashReportingServer");
#endif
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

#ifdef WIN32
// TODO platform specific
//This function gets called in the event of a crash.
bool BreakpadCrashReportingDumpCallbackWindows(const wchar_t* dump_path,
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

#elif __gnu_linux__
bool BreakpadCrashReportingDumpCallbackLinux(const google_breakpad::MinidumpDescriptor& descriptor,
                             void* context,
                             bool succeeded)
{
  return succeeded;
}
#endif

bool mitk::BreakpadCrashReporting::DumpCallbackPlatformIndependent()
{
  return true;
}

void mitk::BreakpadCrashReporting::InitializeClientHandler(bool connectToCrashGenerationServer)
{
#ifdef WIN32  // http://stackoverflow.com/questions/5625884/conversion-of-stdwstring-to-qstring-throws-linker-error
  std::wstring dump_path = std::wstring((const wchar_t *)m_CrashDumpPath.utf16());
#else
  std::string dump_path = m_CrashDumpPath.toStdString();
#endif



#ifdef WIN32
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

  m_ExceptionHandler = new google_breakpad::ExceptionHandler(
                                 dump_path,
                                 NULL,
                                 BreakpadCrashReportingDumpCallbackWindows,
                                 NULL,
                                 google_breakpad::ExceptionHandler::HANDLER_ALL,
                                 MiniDumpNormal, //see DbgHelp.h
                                 pipe,
                                 NULL); // custom client info (unused)

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

#elif __gnu_linux__

  google_breakpad::MinidumpDescriptor dumpDescriptor( dump_path );

  if (client_fd == -1)
  {
    MITK_WARN << "In-process crash dump handling, the unsafer method";
  }

  m_ExceptionHandler = new google_breakpad::ExceptionHandler(
                                 dumpDescriptor, // descriptor (where to dump)
                                 NULL, // filter (we don't filter)
                                 BreakpadCrashReportingDumpCallbackLinux, // our callback in cases of crashes
                                 NULL, // callback_context (no idea.. custom data probably)
                                 true, // install_handler (yes, write dumps with each crash, not only on request)
                                 client_fd ); // should be initialized in StopCrashServer() by ealier call


#endif
}

static void
#ifdef WIN32
  _cdecl
#endif
ShowClientConnected(void* context,
                                       const google_breakpad::ClientInfo* client_info)
{ // callback of the crash generation server on client connect
#ifdef WIN32
  MITK_INFO << "Breakpad Client connected: " << client_info->pid();
#else
  MITK_INFO << "Breakpad Client connected: TODO proc-info";
#endif

  breakpadOnceConnected = true; // static variables indicate server shutdown after usage
  breakpadNumberOfConnections++;
}

#ifdef WIN32
static void _cdecl ShowClientCrashed(void* context, const google_breakpad::ClientInfo* client_info, const std::wstring* dump_path)
#elif __gnu_linux__
static void ShowClientCrashed(void* context, const google_breakpad::ClientInfo* client_info, const std::string* dump_path)
#endif
{ // callback of the crash generation server on client crash

#ifdef WIN32
  MITK_INFO << "Breakpad Client request dump: " << client_info->pid();
  // we may add some log info here along the dump file
  google_breakpad::CustomClientInfo custom_info = client_info->GetCustomInfo();
#else
  MITK_INFO << "Breakpad Client request dump: TODO proc-info";
#endif

}

static void
#ifdef WIN32
  _cdecl
#endif
ShowClientExited(void* context,
                                    const google_breakpad::ClientInfo* client_info)
{ // callback of the crash generation server on client exit
#ifdef WIN32
  MITK_INFO << "Breakpad Client exited :" << client_info->pid();
#else
  MITK_INFO << "Breakpad Client exited : TODO proc-info";
#endif

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

  /*
     Idea here:
      - application-under-observation starts out-of-process dump generation executable
      - dump generation executable creates google CrashGenerationServer
        - requires fd to listen to

     DONC
      - CreateReportChannel
        - fork
          - in parent: InitializeServer, exit with return value of qtapplication.exec(), i.e. run forever. TODO stop with crashed child
          - in parent: continue
  */

#ifdef __gnu_linux__
  google_breakpad::CrashGenerationServer::CreateReportChannel(&server_fd, &client_fd); // both OUT parameters

  pid_t child_pid = fork();

  if ( child_pid != 0)
  {
    // server process
    InitializeServer(server_fd);

    if (qApp)
    {
      MITK_INFO << "Wait for observed breakpad child to finish/crash...";
      int status;
      waitpid( child_pid, &status, WEXITED );
      MITK_INFO << "Breakpad child terminated, so I also terminate...";
      exit(EXIT_SUCCESS);
    }
    else
    {
      MITK_ERROR << "You MUST initialize the qApp instance before calling StartCrashServer. You did not. Exiting...";
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    // child process
    return true; // assume we are fine since we got here..
  }

#endif

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

bool mitk::BreakpadCrashReporting::InitializeServer( int listen_fd )
{
  QDir myDir;
  myDir.mkpath(m_CrashDumpPath); // Assure directory is created.

  google_breakpad::CrashGenerationServer::OnClientDumpRequestCallback dump_callback = &ShowClientCrashed;
#ifdef WIN32
  google_breakpad::CrashGenerationServer::OnClientExitedCallback exit_callback = &ShowClientExited;   // this...
#elif __gnu_linux__
  google_breakpad::CrashGenerationServer::OnClientExitingCallback exit_callback = &ShowClientExited;  // and that.. tell much about cross-platform..
#endif

  void* dump_context = NULL;
  void* exit_context = NULL;

#ifdef WIN32  // http://stackoverflow.com/questions/5625884/conversion-of-stdwstring-to-qstring-throws-linker-error
  std::wstring dump_path = std::wstring((const wchar_t *)m_CrashDumpPath.utf16());
  std::wstring pipe_name = std::wstring((const wchar_t *)m_NamedPipeString.utf16());
  m_CrashServer = new google_breakpad::CrashGenerationServer(pipe_name,
                                           NULL,
                                           ShowClientConnected, // connect callback
                                           NULL,
                                           dump_callback,
                                           dump_context,
                                           exit_callback, // exit callback
                                           exit_context,
                                           NULL,
                                           NULL,
                                           true,
                                           &dump_path);
#elif __gnu_linux__
  std::string dump_path = m_CrashDumpPath.toStdString();

  MITK_INFO << "Start Breakpad crash dump generation server with file descriptor " << listen_fd;

  m_CrashServer = new google_breakpad::CrashGenerationServer(listen_fd,
                                           dump_callback,
                                           dump_context,
                                           exit_callback,
                                           exit_context,
                                           true, // generate_dumps
                                           &dump_path);
#endif

  if (!m_CrashServer->Start())
  {
    MITK_ERROR << "Unable to start Breakpad crash dump generation server.";
    delete m_CrashServer;
    m_CrashServer = NULL;
    return false;
  }
  else
  {
    MITK_INFO << "Breakpad crash dump generation server started.";
    return true;
  }

  return false;

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
  int* x = 0;
  *x = 1;
}

int mitk::BreakpadCrashReporting::GetNumberOfConnections()
{
  return breakpadNumberOfConnections;
}

void mitk::BreakpadCrashReporting::SetNamedPipeName(QString name)
{
  m_NamedPipeString = name;
}
