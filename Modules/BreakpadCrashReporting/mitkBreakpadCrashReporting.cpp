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


#ifdef WIN32

#include <windows.h>
#include <tchar.h>
#include "client/windows/crash_generation/client_info.h"
#include "client/windows/crash_generation/crash_generation_server.h"
#include "client/windows/handler/exception_handler.h"
#include "client/windows/common/ipc_protocol.h"

#elif __APPLE__

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

#include <itksys/SystemTools.hxx>

static bool breakpadOnceConnected = false;            // indicates a server having had at least one client connection
static int  breakpadNumberOfConnections = 0;          // current number of connected clients

#ifdef WIN32
static int  numberOfConnectionAttemptsPerformed = 1;  // number of performed re-connect attempts of a crash client
#endif

// Get application path: there is no cross-plattform standard c++ method which can get the executable path
// (other toolkits offer it, e.g. Qt).
// Currently only for Windows and Linux implemented:
std::string mitk::BreakpadCrashReporting::GetModulePath() {
#ifdef WIN32
  TCHAR path[MAX_PATH];
  if( GetModuleFileName( NULL, path, MAX_PATH ) )
  {
    std::string pathString = path;
    pathString.erase( pathString.find_last_of("\\") );
    return pathString;
  }
#elif __gnu_linux__
  char buff[1024];
  ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff)-1);
  if (len != -1) {
    buff[len] = '\0';
    std::string path = buff;
    path.erase( path.find_last_of("/") );
    return path;
  } else {
    /* handle error condition */
  }
#endif
  return "";
}

mitk::BreakpadCrashReporting::BreakpadCrashReporting( const std::string& dumpPath )
: m_CrashServer(NULL)
, m_ExceptionHandler(NULL)
, m_CrashDumpPath( dumpPath )
  // Linux connection parameters
, server_fd(-1)
, client_fd(-1)
  // Windows connection parameters
, m_NamedPipeString("\\\\.\\pipe\\MitkCrashServices\\MitkBasedApplication")
, m_CrashReportingServerExecutable( GetModulePath() + "/CrashReportingServer.exe" )
, m_NumberOfConnectionAttempts(3)
, m_ReconnectDelay(300)
{
  if ( m_CrashDumpPath.empty() )
  {
    m_CrashDumpPath = GetModulePath() + "/CrashDumps"; // ToDo: what happens if GetModulePath returns emtpy string
  }
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
  return succeeded;
}

#elif __gnu_linux__
bool BreakpadCrashReportingDumpCallbackLinux(
                             const google_breakpad::MinidumpDescriptor& /*descriptor*/,
                             void* /*context*/,
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
  std::wstring dump_path( m_CrashDumpPath.begin(), m_CrashDumpPath.end() );
#else
  std::string dump_path = m_CrashDumpPath;
#endif



#ifdef WIN32
  /* This is needed for CRT to not show dialog for invalid param
   failures and instead let the code handle it.*/
  _CrtSetReportMode(_CRT_ASSERT, 0);

  const wchar_t* pipe;
  if(connectToCrashGenerationServer)
  {
     pipe = (const wchar_t*)m_NamedPipeString.c_str();
     MITK_INFO << "Initializing Breakpad Crash Handler, connecting to named pipe: " << m_NamedPipeString.c_str() << "\n";
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

#ifdef WIN32
static void
  _cdecl
ShowClientConnected(void* /*context*/,
                    const google_breakpad::ClientInfo* client_info)
{ // callback of the crash generation server on client connect
  MITK_INFO << "Breakpad Client connected: " << client_info->pid();

  breakpadOnceConnected = true; // static variables indicate server shutdown after usage
  breakpadNumberOfConnections++;
}
#endif

#ifdef WIN32
static void _cdecl ShowClientCrashed(void* /*context*/, const google_breakpad::ClientInfo* client_info, const std::wstring* /*dump_path*/)
#elif __gnu_linux__
static void ShowClientCrashed(void* context, const google_breakpad::ClientInfo* /*client_info*/, const std::string* /*dump_path*/)
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
ShowClientExited(void* /*context*/,
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

#ifdef __gnu_linux__
  google_breakpad::CrashGenerationServer::CreateReportChannel(&server_fd, &client_fd); // both OUT parameters

  pid_t child_pid = fork();

  if ( child_pid != 0)
  {
    // server process
    InitializeServer(server_fd);

    MITK_INFO << "Wait for observed breakpad child to finish/crash...";
    int status;
    do {
      pid_t w = waitpid(child_pid, &status, WUNTRACED | WCONTINUED);
      if (w == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
      }

      if (WIFEXITED(status)) {
        printf("exited, status=%d\n", WEXITSTATUS(status));
      } else if (WIFSIGNALED(status)) {
        printf("killed by signal %d\n", WTERMSIG(status));
      } else if (WIFSTOPPED(status)) {
        printf("stopped by signal %d\n", WSTOPSIG(status));
      } else if (WIFCONTINUED(status)) {
        printf("continued\n");
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    MITK_INFO << "Breakpad child terminated, so I also terminate...";
    exit(EXIT_SUCCESS);
  }
  else
  {
    // child process
    return true; // assume we are fine since we got here..
  }

#elif WIN32

  if(lauchOutOfProcessExecutable)
  { // spawn process and launch CrashReportingServer executable
    std::string commandline = m_CrashReportingServerExecutable + " \"" + m_NamedPipeString + "\" " + m_CrashDumpPath;
    int success =  system( commandline.c_str() );

    return ( success != -1 );
  }
  else
  { // directly open up server instance in this thread
    return InitializeServer();
  }

#endif
}

bool mitk::BreakpadCrashReporting::InitializeServer( int listen_fd )
{
  itksys::SystemTools::MakeDirectory(m_CrashDumpPath.c_str()); // Make sure directory is created.

  google_breakpad::CrashGenerationServer::OnClientDumpRequestCallback dump_callback = &ShowClientCrashed;
#ifdef WIN32
  google_breakpad::CrashGenerationServer::OnClientExitedCallback exit_callback = &ShowClientExited;   // this...
#elif __gnu_linux__
  google_breakpad::CrashGenerationServer::OnClientExitingCallback exit_callback = &ShowClientExited;  // and that.. tell much about cross-platform..
#endif

  void* dump_context = NULL;
  void* exit_context = NULL;

#ifdef WIN32  // http://stackoverflow.com/questions/5625884/conversion-of-stdwstring-to-qstring-throws-linker-error
  std::wstring dump_path(m_CrashDumpPath.begin(), m_CrashDumpPath.end() );
  std::wstring pipe_name( m_NamedPipeString.begin(), m_NamedPipeString.end() );
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
  std::string dump_path = m_CrashDumpPath;

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

void mitk::BreakpadCrashReporting::CrashAppForTestPurpose()
{
  int* x = 0;
  *x = 1;
}

int mitk::BreakpadCrashReporting::GetNumberOfConnections() const
{
  return breakpadNumberOfConnections;
}

void mitk::BreakpadCrashReporting::SetNamedPipeName(const std::string& name)
{
  m_NamedPipeString = name;
}

std::string mitk::BreakpadCrashReporting::GetNamedPipeName() const
{
  return m_NamedPipeString;
}

void mitk::BreakpadCrashReporting::SetCrashDumpPath(const std::string& path)
{
  m_CrashDumpPath = path;
}

std::string mitk::BreakpadCrashReporting::GetCrashDumpPath() const
{
  return m_CrashDumpPath;
}


void mitk::BreakpadCrashReporting::SetCrashReportingServerExecutable(const std::string& exe)
{
  m_CrashReportingServerExecutable = exe;
}

void mitk::BreakpadCrashReporting::SetNumberOfConnectionAttempts(int no)
{
  m_NumberOfConnectionAttempts = no;
}

void mitk::BreakpadCrashReporting::SetReconnectDelayInMilliSeconds(int ms)
{
  m_ReconnectDelay = ms;
}
