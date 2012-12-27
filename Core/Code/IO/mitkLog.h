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


#ifndef _MITK_LOG_H
#define _MITK_LOG_H

#include <MitkExports.h>
#include <mbilog.h>

namespace mitk
{

  /*!
    \brief mbilog backend implementation for mitk
   */
  class MITK_CORE_EXPORT LoggingBackend : public mbilog::TextBackendBase
  {
    public:

     /** \brief overloaded method for receiving log message from mbilog
      */
      void ProcessMessage(const mbilog::LogMessage& );

     /** \brief registers MITK logging backend at mbilog
      */
      static void Register();

     /** \brief Unregisters MITK logging backend at mbilog
      */
      static void Unregister();

     /** \brief Sets extra log file path (additionally to the console log)
      */
      static void SetLogFile(const char *file);

     /** @return Returns the log file if there is one. Returns an empty string
      *          if no log file is active.
      */
      static std::string GetLogFile();

     /** \brief Enables an additional logging output window by means of itk::outputwindow
     * This might be relevant for showing log output in applications with no default output console
      */
      static void EnableAdditionalConsoleWindow(bool enable);

     /** \brief Automatically extracts and removes the "--logfile <file>" parameters from the standard C main(argc,argv) parameter list and calls SetLogFile if needed
      */
      static void CatchLogFileCommandLineParameter(int &argc,char **argv);

  };

}

#endif
