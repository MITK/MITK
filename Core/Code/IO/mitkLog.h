/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Tue, 12 May 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_LOG_H
#define _MITK_LOG_H

#include <mitkCommon.h>

namespace mitk
{

  /*!
    \brief mbilog backend implementation for mitk
   */
  class MITK_CORE_EXPORT LogBackend : public mbilog::AbstractBackend
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
     
     /** \brief Automatically extracts and removes the "--logfile <file>" parameters from the standard C main(argc,argv) parameter list and calls SetLogFile if needed
      */      
      static void CatchLogFileCommandLineParameter(int &argc,char **argv);
      
  };
  
}

#endif
