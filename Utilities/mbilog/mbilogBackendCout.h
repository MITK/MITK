/*=========================================================================

Program:   mbilog - logging for mitk / BlueBerry

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _mbilogBackendCout_H
#define _mbilogBackendCout_H

#include <iostream>

#include "mbilogExports.h"
#include "mbilogAbstractBackend.h"
#include "mbilogLogMessage.h"
#include "mbilogConfig.h"

namespace mbilog{

  class MBILOG_DLL_API BackendCout : public AbstractBackend
  {
    public:

      BackendCout();
      ~BackendCout();
      virtual void ProcessMessage(const mbilog::LogMessage &l );

      void SetFull(bool full);

      static void FormatSmart(const LogMessage &l,int threadID=0);
      static void FormatFull(const LogMessage &l,int threadID=0);

      static void FormatSmart(std::ostream &out, const LogMessage &l,int threadID=0);
      static void FormatFull(std::ostream &out, const LogMessage &l,int threadID=0);

    private:

      static void AppendTimeStamp(std::ostream& out);
      static void FormatSmartWindows(const mbilog::LogMessage &l,int /*threadID*/);

      bool useFullOutput;

  };

}


#endif