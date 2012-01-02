/*=========================================================================

Program:   mbilog - logging for mitk / BlueBerry

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _mbilogLogMessage_H
#define _mbilogLogMessage_H

#include <string>

#include "mbilogExports.h"

namespace mbilog{

 class MBILOG_DLL_API LogMessage {

    public:

      const int level;
      const char* filePath;
      const int lineNumber;
      const char* functionName;

      const char* moduleName;
      std::string category;
      std::string message;

      LogMessage(
        const int _level,
        const char* _filePath,
        const int _lineNumber,
        const char* _functionName
      )
        : level(_level)
        , filePath(_filePath)
        , lineNumber(_lineNumber)
        , functionName(_functionName)
      {
      }
  };

}


#endif