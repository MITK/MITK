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

#include <windows.h>
#include <tchar.h>

#include "client/windows/crash_generation/client_info.h"
#include "client/windows/crash_generation/crash_generation_server.h"
#include "client/windows/handler/exception_handler.h"
#include "client/windows/common/ipc_protocol.h"

#include <QApplication>
#include <QDir>

mitk::BreakpadCrashReporting::BreakpadCrashReporting()
{
  m_CrashServer         = NULL;
  m_ExceptionHandler    = NULL;
  
  m_NamedPipeString       = "\\\\.\\pipe\\BreakpadCrashServices\\TestServer";
  m_CrashDumpPath = QDir(QApplication::instance()->applicationDirPath()).filePath("CrashDumps");
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

bool ShowDumpResults(const wchar_t* dump_path,
                     const wchar_t* minidump_id,
                     void* context,
                     EXCEPTION_POINTERS* exinfo,
                     MDRawAssertionInfo* assertion,
                     bool succeeded) 
{
  /*if(succeeded)
    MITK_INFO << "Dump generation request result succeeded.";
  else
    MITK_INFO << "Dump generation request result failed.";
  return succeeded;*/
  return succeeded;
}


void mitk::BreakpadCrashReporting::Initialize()
{  
  //std::wstring dumpPath = this->m_CrashDumpPath.toStdWString();

  google_breakpad::CustomClientInfo custom_info;

  StartCrashServer();

  /* This is needed for CRT to not show dialog for invalid param
   failures and instead let the code handle it.*/
  _CrtSetReportMode(_CRT_ASSERT, 0);

  const wchar_t kPipeName[] = L"\\\\.\\pipe\\BreakpadCrashServices\\TestServer";
  m_ExceptionHandler = new google_breakpad::ExceptionHandler(L"C:\\dumps\\",
                                 NULL,
                                 ShowDumpResults,
                                 NULL,
                                 google_breakpad::ExceptionHandler::HANDLER_ALL,
                                 MiniDumpNormal, //see DbgHelp.h
                                 kPipeName,
                                 &custom_info);

}

static void _cdecl ShowClientConnected(void* context,
                                       const google_breakpad::ClientInfo* client_info) 
{
  MITK_INFO << "Breakpad Client connected: " << client_info->pid();
}

static void _cdecl ShowClientCrashed(void* context,
                                     const google_breakpad::ClientInfo* client_info,
                                     const std::wstring* dump_path) 
{

  MITK_INFO << "Breakpad Client request dump: " << client_info->pid();

  google_breakpad::CustomClientInfo custom_info = client_info->GetCustomInfo();
  if (custom_info.count <= 0) 
  {
    return;
  }  
}

static void _cdecl ShowClientExited(void* context,
                                    const google_breakpad::ClientInfo* client_info) 
{
  MITK_INFO << "Breakpad Client exited :" << client_info->pid();
}

void mitk::BreakpadCrashReporting::StartCrashServer() 
{
  // Do not create another instance of the server.
  if (m_CrashServer)
  {
    return;
  }

  std::wstring dump_path = L"C:\\Dumps\\";
  const wchar_t kPipeName[] = L"\\\\.\\pipe\\BreakpadCrashServices\\TestServer";
  m_CrashServer = new google_breakpad::CrashGenerationServer(kPipeName,
                                           NULL,
                                           ShowClientConnected,
                                           NULL,
                                           ShowClientCrashed,
                                           NULL,
                                           ShowClientExited,
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
  }
  else
  {
    MITK_INFO << "Google breakpad server started.";
  }
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



