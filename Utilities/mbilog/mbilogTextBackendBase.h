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

  protected:

    /** \brief Method formats the given LogMessage in the smart/short format and writes it to std::cout.
      * \param threadID Can be set to the threadID where the logging message was emitted. Is 0 by default.
      */
    void FormatSmart(const LogMessage &l,int threadID=0);

    /** \brief Method formats the given LogMessage in the full/long format and writes it to std::cout.
      * \param threadID Can be set to the threadID where the logging message was emitted. Is 0 by default.
      */
    void FormatFull(const LogMessage &l,int threadID=0);

    /** \brief Method formats the given LogMessage in the smart/short format and writes it to the given std::ostream.
      * \param threadID Can be set to the threadID where the logging message was emitted. Is 0 by default.
      */
    void FormatSmart(std::ostream &out, const LogMessage &l,int threadID=0);

    /** \brief Method formats the given LogMessage in the full/long format and writes it to the given std::ostream.
      * \param threadID Can be set to the threadID where the logging message was emitted. Is 0 by default.
      */
    void FormatFull(std::ostream &out, const LogMessage &l,int threadID=0);

    /** \brief Writes the system time to the given stream.*/
    void AppendTimeStamp(std::ostream& out);

    /** \brief Special variant of method FormatSmart which uses colored messages (only for windows).*/
    //TODO: implement for linux?
    void FormatSmartWindows(const mbilog::LogMessage &l,int /*threadID*/);
  };

}

#endif
