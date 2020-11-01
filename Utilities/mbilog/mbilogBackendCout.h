/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _mbilogBackendCout_H
#define _mbilogBackendCout_H

#include <iostream>

#include "mbilogExports.h"
#include "mbilogLogMessage.h"
#include "mbilogLoggingTypes.h"
#include "mbilogTextBackendBase.h"

namespace mbilog
{
  /**
   *  \brief Default backend of the mbi logging mechanism. This backend is used if no other backend is registered.
   *         The backend formats the logging messages to a normal string and writes them to std::cout.
   *  \ingroup mbilog
   */
  class MBILOG_EXPORT BackendCout : public TextBackendBase
  {
  public:
    BackendCout();
    ~BackendCout() override;

    /** \brief This method is called by the mbi logging mechanism if the object is registered in
     *         the mbi logging mechanism and a logging message is emitted. The method formats the
     *         logging messages to a normal string (depending on formatting mode) and writes it to std::cout.
     *
     *  \param l Logging message.
     */
    void ProcessMessage(const mbilog::LogMessage &l) override;

    /** \brief Sets the formatting mode. If true long messages will be displayed. Default is false (short/smart
     * messages).
     *         Long messages provide all informations and are also capable to be postproccessed (e.g. in a web viewer).
     */
    void SetFull(bool full);

    OutputType GetOutputType() const override;

  private:
    /** \brief The formatting mode of this backend. True is full/long message formatting mode. False is short/smart
     *         message formatting mode
     */
    bool m_useFullOutput;
  };
}

#endif
