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
#include <QMutex>


namespace google_breakpad {

  class ExceptionHandler;
  class CrashGenerationServer;
}

namespace mitk {

  /**
  * \brief Prototype for using Google's Breakpad Project in MITK.
  */
  class MITK_BREAKPAD_EXPORT BreakpadCrashReporting
  {
    public:
      BreakpadCrashReporting();
      ~BreakpadCrashReporting();
      void Initialize();

      // named pipe string to communicate with OutOfProcessCrashReporter.
      QString m_NamedPipeString;

      // QString directory path to save crash dumps.
      QString m_CrashDumpPath;

      mutable QMutex m_CriticalSection;

      google_breakpad::ExceptionHandler*       m_ExceptionHandler;
      google_breakpad::CrashGenerationServer*  m_CrashServer;

      void CrashAppForTestPurpose();

      void RequestDump();
  protected:
      void StartCrashServer();
      void StopCrashServer();

    
  };
} // namespace mitk

#endif /* _H_HEADER_INCLUDED_ */
