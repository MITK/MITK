/*=========================================================================

Program:   mbilog - logging for mitk / BlueBerry

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _mbilogTextBackendBase_H
#define _mbilogTextBackendBase_H

#include "mbilogExports.h"
#include "mbilogBackendBase.h"

namespace mbilog{


  /** Documentation
   *  \brief This class is an abstract superclass for logging text backends. It adds string formatting methods
   *         to the backend interface BackendBase. Accordingly backends that inherit from this class can be 
   *         registered in the mbi logging mechanism.
   *
   *  \ingroup mbilog
   */
  class MBILOG_DLL_API TextBackendBase : public BackendBase
  {
  public: 

    virtual ~TextBackendBase();

	/** \brief This method is called by the mbi logging mechanism if the object is registered in
	 *         the mbi logging mechanism and a logging message is emitted.
	 *
	 *  \param logMessage Logging message which was emitted.
	 *
	 */
    virtual void ProcessMessage(const mbilog::LogMessage& logMessage)=0;
  };

}

#endif