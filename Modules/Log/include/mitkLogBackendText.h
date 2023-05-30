/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLogBackendText_h
#define mitkLogBackendText_h

#include <mitkLogBackendBase.h>

namespace mitk
{
  /** \brief Abstract superclass for text-based log backends.
   *
   * Adds string formatting methods to the interface LogBackendBase. Backends that inherit from this class can be
   * registered by the MITK log mechanism.
   */
  class MITKLOG_EXPORT LogBackendText : public LogBackendBase
  {
  public:
    ~LogBackendText() override;

    void ProcessMessage(const LogMessage& message) override = 0;

  protected:
    /** \brief Format the given message in the smart/short format and writes it to std::cout.
     *
     * \param message
     * \param threadID Can be set to the thread id where the log message was emitted. 0 by default.
     */
    void FormatSmart(const LogMessage& message, int threadID = 0);

    /** \brief Format the given message in the full/long format and writes it to std::cout.
     *
     * \param message
     * \param threadID Can be set to the thread id where the log message was emitted. 0 by default.
     */
    void FormatFull(const LogMessage& message, int threadID = 0);

    /** \brief Format the given message in the smart/short format and writes it to the given std::ostream.
     *
     * \param out
     * \param message
     * \param threadID Can be set to the thread id where the log message was emitted. 0 by default.
     */
    void FormatSmart(std::ostream& out, const LogMessage& message, int threadID = 0);

    /** \brief Format the given message in the full/long format and writes it to the given std::ostream.
     *
     * \param out
     * \param message
     * \param threadID Can be set to the thread id where the log message was emitted. 0 by default.
     */
    void FormatFull(std::ostream& out, const LogMessage& message, int threadID = 0);

    /** \brief Write system time to the given stream.
     */
    void AppendTimeStamp(std::ostream& out);

    /** \brief Special variant of method FormatSmart which uses colored messages (only for Windows).
     */
    void FormatSmartWindows(const LogMessage& message, int /*threadID*/);
  };
}

#endif
