/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLogBackendBase_h
#define mitkLogBackendBase_h

#include <mitkLogMessage.h>
#include <MitkLogExports.h>

namespace mitk
{
  /** \brief Interface for log backends that can be registered in the MITK log mechanism.
   */
  class MITKLOG_EXPORT LogBackendBase
  {
  public:
    /** Type of the output of a backend.
     */
    enum class OutputType
    {
      Console,
      File,
      Other = 100
    };

    virtual ~LogBackendBase();

    /** \brief Called by the MITK log mechanism if the object is registered and a log message is emitted.
     *
     * \param message Logging message which was emitted.
     */
    virtual void ProcessMessage(const LogMessage& message) = 0;

    /**
     * \return The type of this backend.
     */
    virtual OutputType GetOutputType() const = 0;
  };
}

#endif
