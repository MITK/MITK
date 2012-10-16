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

#ifndef MITK_BREAKPAD_CRASH_REPORTING_H
#define MITK_BREAKPAD_CRASH_REPORTING_H

#include "BreakpadCrashReportingExports.h"

#include <QString>


namespace google_breakpad {

  class ExceptionHandler;
  class CrashGenerationServer;
}

namespace mitk {

  /**
  * \brief Integration of Google's Breakpad Project in MITK.
  *
  * Breakpad is a library and tool suite that allows you to distribute an application to users with compiler-provided
  * debugging information removed, record crashes in compact "minidump" files, send them back to your server, and
  * produce C and C++ stack traces from these minidumps. Breakpad can also write minidumps on request for programs that
  * have not crashed (from http://code.google.com/p/google-breakpad/wiki/GettingStartedWithBreakpad).
  *
  *
  * Usage:
  *
  * In-process usage:
  * Instantiate this class, and initialize an event handler for 'unhandled' exceptions by calling
  * InitializeClientHandler(false). In the event of a crash your application will try to generate a dump file.
  *
  * Out-of-process (OOP) usage:
  * However,your application crashed - therefore using your application's process for dealing with unhandled exceptions is
  * not safe. Heap and stack may be corrupted. Having a separated process that invoces the generation of a minidump of your process is best,
  * this is called out-of-process exception handling.
  *
  * Sample code for simple OOP usage:
  *   mitk::BreakpadCrashReporting myBreakpad;
  *   myBreakpad.StartCrashServer(true);
  *   [... some code ...]
  *   myBreakpad.InitializeClientHandler(true);
  *
  * Note 1: The start of a detached process takes some time. Either you call InitializeClientHandler(true) a while after calling
  * StartCrashServer(true), or it may take some time and a few connection attempts (configurable, see re-connect handling).
  *
  * Note 2: If there is no connection to the server possible, there is an automatic fallback to in-process usage.
  * Client and server output will indicate the operating mode.
  *
  * Note 3: The crash reporting server process will automatically shutdown, if there was a client connected and exits
  * (either due to shutdown or due to crash). Also, the sample server will shutdown automatically, if  there isalready
  * one server instance running.
  *
  */
  class MITK_BREAKPAD_EXPORT BreakpadCrashReporting
  {
    public:
      BreakpadCrashReporting();
      ~BreakpadCrashReporting();

      /** Initializes an event handler for 'unhandled exceptions' that will dump a so-called 'minidump' to a defined folder.
      * For usage as "in-process" exception handler set connectToCrashGenerationServer      = false.
      * For usage as "out-of-process" (OOP) exception handler, set connectToCrashGenerationServer = true.
      *
      * Related params:
      * Are defined by means of SetNamedPipeName() and SetCrashDumpPath().
      *
      * OOP Usage:
      * In OOP use case, the handler uses a crash generation client that connects to a crash generation server via named pipes.
      * Such a crash generation server should be started then on beforehand by means of the function StartCrashServer() below.
      *
      * If the connection attempt to a server fails, reconnects attempt may be scheduled by SetNumberOfConnectionAttempts()
      * and SetReconnectDelayInMilliSeconds(). Note that during re-connect attempts, your application will be blocked.
      *
      *
      *
      */
      void InitializeClientHandler(bool connectToCrashGenerationServer);

      /** Starts a crash generation server for "out-of-process" exception handling.
      *
      * For usage outside of your main application (i.e. already in a separate process), set launchOutOfProcessExecutable = false.
      * For usage inside of your main application, set launchOutOfProcessExecutable = true.
      *
      * In the latter case, StartCrashServer() will spawn a detached process launching the crash generation server.
      * This server process will automatically shutdown again, if a once connected client exits due to client shutdown or crash.
      *
      * By default, an instance of the sample crash reporting server, mitk::CrashReportingServer will be used. Alternatively,
      * you may define a process to be started by SetCrashReportingServerExecutable().
      *
      * Related params are defined by means of SetNamedPipeName() and SetCrashDumpPath().
      *
      */
      bool StartCrashServer(bool launchOutOfProcessExecutable);

      // Named pipe string to communicate with OutOfProcessCrashReporter.
      QString m_NamedPipeString;
      void    SetNamedPipeName(QString name){m_NamedPipeString = name;}

      // Directory path to save crash dumps.
      QString m_CrashDumpPath;
      void    SetCrashDumpPath(QString path){m_CrashDumpPath = path;}

      // External out-of-process (OOP) Crash Reporting Server file path - if OOP is used.
      QString m_CrashReportingServerExecutable;
      void    SetCrashReportingServerExecutable(QString exe){m_CrashReportingServerExecutable = exe;}

      // Re-connect handling in case a crash server cannot be reached.
      void    SetNumberOfConnectionAttempts(int no){m_NumberOfConnectionAttempts = no;}
      void    SetReconnectDelayInMilliSeconds(int ms) {m_ReconnectDelay = ms;}
      int     m_NumberOfConnectionAttempts;
      int     m_ReconnectDelay;

      // This may be a security issue.
      google_breakpad::ExceptionHandler*       m_ExceptionHandler;
      google_breakpad::CrashGenerationServer*  m_CrashServer;

      // Do not call this without purpose :-)
      void CrashAppForTestPurpose();

      // Writes a minidump immediately.  This can be used to capture the
      // execution state independently of a crash.  Returns true on success.
      bool RequestDump();

      // returns the number of currently connected clients
      int GetNumberOfConnections();


  protected:
      bool InitializeServer();
      void StopCrashServer();
  };
} // namespace mitk

#endif /* _H_HEADER_INCLUDED_ */
