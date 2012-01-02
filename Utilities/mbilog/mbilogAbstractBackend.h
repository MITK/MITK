/*=========================================================================

Program:   mbilog - logging for mitk / BlueBerry

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _mbilogAbstractBackend_H
#define _mbilogAbstractBackend_H

#include "mbilogExports.h"
#include "mbilogLogMessage.h"

namespace mbilog{

  struct MBILOG_DLL_API AbstractBackend
  {
    virtual ~AbstractBackend(){}
    virtual void ProcessMessage(const mbilog::LogMessage& )=0;
  };
  
}


#endif