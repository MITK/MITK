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

#ifndef _mbilogBackendCout_H
#define _mbilogBackendCout_H

#include <iostream>

#include "mbilogExports.h"
#include "mbilogTextBackendBase.h"
#include "mbilogLogMessage.h"
#include "mbilogLoggingTypes.h"

namespace mbilog{

  /**
   *  \brief Default backend of the mbi logging mechanism. This backend is used if no other backend is registered.
   *         The backend formats the logging messages to a normal string and writes them to std::cout.
   *  \ingroup mbilog
   */
  class MBILOG_DLL_API BackendCout : public TextBackendBase
  {
    public:

      BackendCout();
      virtual ~BackendCout();

      /** \brief This method is called by the mbi logging mechanism if the object is registered in
       *         the mbi logging mechanism and a logging message is emitted. The method formats the
       *         logging messages to a normal string (depending on formatting mode) and writes it to std::cout.
       *
       *  \param logMessage Logging message.
       */
      virtual void ProcessMessage(const mbilog::LogMessage &l );

      /** \brief Sets the formatting mode. If true long messages will be displayed. Default is false (short/smart messages).
       *         Long messages provide all informations and are also capable to be postproccessed (e.g. in a web viewer).
       */
      void SetFull(bool full);

      virtual OutputType GetOutputType() const;

    private:

      /** \brief The formatting mode of this backend. True is full/long message formatting mode. False is short/smart
       *         message formatting mode
       */
      bool m_useFullOutput;

  };

}


#endif
